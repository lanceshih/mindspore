/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vector>
#include "include/errorcode.h"
#include "src/lite_session.h"
#include "utils/log_adapter.h"
#include "src/scheduler.h"
#include "src/runtime/runtime_api.h"
#include "src/runtime/allocator.h"
#include "src/executor.h"
#include "src/common/utils.h"
#include "src/common/graph_util.h"
#include "src/kernel_registry.h"
#if SUPPORT_GPU
#include "src/runtime/opencl/opencl_runtime.h"
#endif

namespace mindspore {
namespace lite {
int LiteSession::ConvertTensors(const lite::Model *model) {
  MS_EXCEPTION_IF_NULL(model);
  auto meta_graph = model->GetMetaGraph();
  MS_EXCEPTION_IF_NULL(meta_graph);
  uint32_t tensorCount = meta_graph->allTensors()->size();
  for (uint32_t i = 0; i < tensorCount; i++) {
    auto *srcTensor = meta_graph->allTensors()->GetAs<schema::Tensor>(i);
    if (srcTensor == nullptr) {
      MS_LOG(ERROR) << i << "th tensor in meta_graph is nullptr";
      return RET_NULL_PTR;
    }
    std::vector<int> shape;
    if (srcTensor->dims() == nullptr) {
      MS_LOG(DEBUG) << "Dims of " << i << "th tensor is nullptr";
    } else {
      if (srcTensor->nodeType() == schema::NodeType_ValueNode) {
        for (size_t j = 0; j < srcTensor->dims()->size(); j++) {
          shape.push_back(srcTensor->dims()->data()[j]);
        }
      }
    }
    int dataType = srcTensor->dataType();
    auto *dstTensor = new tensor::Tensor(TypeId(dataType), shape, srcTensor->format(), srcTensor->nodeType());
    if (srcTensor->nodeType() == schema::NodeType_ValueNode && srcTensor->data() != nullptr &&
        srcTensor->data()->size() > 0) {
      if (shape.empty()) {
        shape.push_back(1);
      }
      MS_ASSERT(dstTensor != nullptr);
      MS_ASSERT(dstTensor->Size() == srcTensor->data()->size());
      // no copy data, do copy when call LiteKernel::Init
      dstTensor->SetData(const_cast<unsigned char *>(srcTensor->data()->data()));
    }
    this->tensors.emplace_back(dstTensor);
  }
  return RET_OK;
}

void LiteSession::InitGraphInOutTensor(const lite::Model *model) {
  auto meta_graph = model->GetMetaGraph();
  MS_ASSERT(this->input_map.empty());
  MS_ASSERT(meta_graph != nullptr);
  auto graph_input_node_indexes = GetGraphInputNodes(meta_graph);
  for (auto in_node_index : graph_input_node_indexes) {
    auto *in_node = meta_graph->nodes()->GetAs<schema::CNode>(in_node_index);
    MS_ASSERT(nullptr != in_node);
    MS_ASSERT(this->input_map.find(in_node->name()->str()) == this->input_map.end());
    for (size_t i = 0; i < in_node->inputIndex()->size(); i++) {
      auto in_tensor_index = size_t(in_node->inputIndex()->GetAs<uint32_t>(i));
      bool is_graph_input = false;
      for (size_t j = 0; j < meta_graph->inputIndex()->size(); j++) {
        if (in_tensor_index == size_t(meta_graph->inputIndex()->GetAs<uint32_t>(j))) {
          is_graph_input = true;
          break;
        }
      }
      if (!is_graph_input) {
        continue;
      }
      MS_ASSERT(in_tensor_index < this->tensors.size());
      auto *in_tensor = this->tensors.at(in_tensor_index);
      MS_ASSERT(in_tensor != nullptr);
      auto *ms_tensor = new tensor::LiteTensor(in_tensor);
      MS_ASSERT(nullptr != ms_tensor);
      this->input_map[in_node->name()->str()].emplace_back(ms_tensor);
    }
  }

  auto graph_output_node_indexes = GetGraphOutputNodes(meta_graph);
  for (auto out_node_index : graph_output_node_indexes) {
    auto *out_node = meta_graph->nodes()->GetAs<schema::CNode>(out_node_index);
    MS_ASSERT(nullptr != out_node);
    MS_ASSERT(this->output_map.find(out_node->name()->str()) == this->output_map.end());
    for (size_t i = 0; i < out_node->outputIndex()->size(); i++) {
      auto out_tensor_index = size_t(out_node->outputIndex()->GetAs<uint32_t>(i));
      bool is_graph_output = false;
      for (size_t j = 0; j < meta_graph->outputIndex()->size(); j++) {
        if (out_tensor_index == size_t(meta_graph->outputIndex()->GetAs<uint32_t>(j))) {
          is_graph_output = true;
          break;
        }
      }
      if (!is_graph_output) {
        continue;
      }
      MS_ASSERT(out_tensor_index < this->tensors.size());
      auto *out_tensor = this->tensors.at(out_tensor_index);
      MS_ASSERT(out_tensor != nullptr);
      auto *ms_tensor = new tensor::LiteTensor(out_tensor);
      MS_ASSERT(nullptr != ms_tensor);
      this->output_map[out_node->name()->str()].emplace_back(ms_tensor);
    }
  }
}

int LiteSession::CompileGraph(Model *model) {
  // model.MetaGraph ==> kernels
  if (model == nullptr) {
    MS_LOG(ERROR) << "The input model is nullptr.";
    return RET_PARAM_INVALID;
  }

  auto ret = ConvertTensors(model);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "ConvertTensors failed: " << ret;
    return ret;
  }

  InitGraphInOutTensor(model);

  // scheduler kernels
  Scheduler scheduler(context_);
  ret = scheduler.Schedule(model, &tensors, &kernels);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "Schedule kernels failed: " << ret;
    return ret;
  }

  return RET_OK;
}

std::vector<mindspore::tensor::MSTensor *> LiteSession::GetInputs() const {
  std::vector<mindspore::tensor::MSTensor *> ret;
  for (auto &iter : this->input_map) {
    auto &node_input_tensors = iter.second;
    for (auto tensor : node_input_tensors) {
      if (!IsContain(ret, tensor)) {
        ret.emplace_back(tensor);
      }
    }
  }
  return ret;
}

int LiteSession::RunGraph(const session::KernelCallBack &before, const session::KernelCallBack &after) {
  MS_EXCEPTION_IF_NULL(this->context_);
  SetMaxWokerNum(context_->thread_num_);
  context_->running_ = true;
  Executor executor;
  if (before == nullptr && after == nullptr) {
    return executor.Run(this->inputs, this->outputs, this->kernels, this->context_->allocator.get());
  } else {
    return executor.Run(this->inputs, this->outputs, this->kernels, this->context_->allocator.get(), before, after);
  }
}

std::vector<mindspore::tensor::MSTensor *> LiteSession::GetOutputs() const {
  std::vector<mindspore::tensor::MSTensor *> ret;
  for (auto &iter : this->output_map) {
    auto &node_output_tensors = iter.second;
    for (auto tensor : node_output_tensors) {
      if (!IsContain(ret, tensor)) {
        ret.emplace_back(tensor);
      }
    }
  }
  return ret;
}

int LiteSession::Init(Context *context) {
  MS_EXCEPTION_IF_NULL(context);
  this->context_ = new (std::nothrow) Context(context->thread_num_, context->allocator, context->device_ctx_);
  if (this->context_ == nullptr) {
    MS_LOG(ERROR) << "new context failed";
    return RET_MEMORY_FAILED;
  }
  this->context_->cpu_bind_mode_ = context->cpu_bind_mode_;
  ConfigThreadPool(context->cpu_bind_mode_, context->thread_num_);
  auto ret = KernelRegistry::GetInstance()->Init();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "KernelRegistry Init Failed.";
    return ret;
  }
#if SUPPORT_GPU
  if (context_->device_ctx_.type == DT_GPU) {
    auto opencl_runtime = lite::opencl::OpenCLRuntime::GetInstance();
    opencl_runtime->Init();
  }
#endif
  return RET_OK;
}

void LiteSession::BindThread(bool ifBind) {
  if (this->context_->cpu_bind_mode_ != NO_BIND) {
    DoAllThreadBind(ifBind, static_cast<int>(this->context_->cpu_bind_mode_));
  }
}

LiteSession::~LiteSession() {
  for (auto *tensor : tensors) {
    // weight data can not be to free, we will free weight data when freeing meta_graph
    if (tensor->TensorType() == schema::NodeType_ValueNode && !IsContain(this->inputs, tensor)) {
      tensor->SetData(nullptr);
    }
    delete tensor;
  }
  // inputs outputs input_map output_map are freed in tensors
  for (auto *input : inputs) {
    ((tensor::LiteTensor *)input)->SetTensorImpl(nullptr);
    delete input;
  }
  for (auto *output : outputs) {
    ((tensor::LiteTensor *)output)->SetTensorImpl(nullptr);
    delete output;
  }
  for (auto iter : this->input_map) {
    for (auto *ms_tensor : iter.second) {
      ((tensor::LiteTensor *)ms_tensor)->SetTensorImpl(nullptr);
      delete ms_tensor;
    }
    iter.second.clear();
  }
  input_map.clear();
  for (auto iter : this->output_map) {
    for (auto *ms_tensor : iter.second) {
      ((tensor::LiteTensor *)ms_tensor)->SetTensorImpl(nullptr);
      delete ms_tensor;
    }
    iter.second.clear();
  }
  output_map.clear();
  for (auto *kernel : kernels) {
    delete kernel;
  }
  delete this->context_;
}

std::vector<mindspore::tensor::MSTensor *> LiteSession::GetInputsByName(const std::string &name) const {
  auto ret = input_map.find(name);
  if (ret == input_map.end()) {
    MS_LOG(WARNING) << "Node  " << name << " is not an input node";
    std::vector<mindspore::tensor::MSTensor *> empty_ret;
    return empty_ret;
  }
  return ret->second;
}

std::vector<mindspore::tensor::MSTensor *> LiteSession::GetOutputsByName(const std::string &name) const {
  auto ret = output_map.find(name);
  if (ret == output_map.end()) {
    MS_LOG(WARNING) << "Node  " << name << " is not an output node";
    std::vector<mindspore::tensor::MSTensor *> empty_ret;
    return empty_ret;
  }
  return ret->second;
}
}  // namespace lite

session::LiteSession *session::LiteSession::CreateSession(lite::Context *context) {
  auto session = new lite::LiteSession();
  auto ret = session->Init(context);
  if (ret != mindspore::lite::RET_OK) {
    MS_LOG(ERROR) << "init sesssion failed";
    delete session;
    return nullptr;
  }
  return session;
}
}  // namespace mindspore
