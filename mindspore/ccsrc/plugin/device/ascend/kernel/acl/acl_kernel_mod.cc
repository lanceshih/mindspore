/**
 * Copyright 2022 Huawei Technologies Co., Ltd
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
#include "plugin/device/ascend/kernel/acl/acl_kernel_mod.h"

#include <vector>
#include <map>
#include "ir/tensor.h"
#include "include/common/utils/anfalgo.h"
#include "kernel/common_utils.h"
#include "backend/common/session/anf_runtime_algorithm.h"
#include "runtime/device/kernel_runtime.h"

namespace mindspore {
namespace kernel {
namespace {
const char kNAttrName[] = "N";
}
int AclKernelMod::Resize(const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
                         const std::vector<KernelTensorPtr> &outputs,
                         const std::map<uint32_t, tensor::TensorPtr> &inputsOnHost) {
  auto node = anf_node_.lock();
  MS_EXCEPTION_IF_NULL(node);
  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);

  if (!common::AnfAlgo::IsDynamicShape(cnode)) {
    MS_LOG(EXCEPTION) << "The node is not dynamic shape: " << cnode->fullname_with_scope();
  }

  std::vector<size_t> useless_input_lists;
  // Update input size list
  for (size_t i = 0; i < input_size_list_.size(); ++i) {
    auto index = AnfAlgo::GetInputGraphIdxByKernelIdx(node, i);
    if (index >= input_size_list_.size()) {
      MS_LOG(EXCEPTION) << "Error real index:" << index;
    }
    TypeId type_id = AnfAlgo::GetInputDeviceDataType(node, index);
    auto type_size = GetTypeByte(TypeIdToType(type_id));
    auto shape = AnfAlgo::GetInputDeviceShape(node, index);
    if (IsDynamic(shape)) {
      MS_LOG(ERROR) << "Please check infer op shape before resize, error input index is:" << i;
      return 1;
    }
    input_size_list_[i] = type_size * SizeOf(shape);
    if (input_size_list_[i] == 0) {
      if (type_size == 0) {
        input_size_list_[i] = SIZE_MAX;
        MS_LOG(INFO) << "Useless optional input" << i << " with node " << node->DebugString();
      } else {
        MS_LOG(INFO) << "Useless dynamic zero input" << i << " with node " << node->DebugString();
      }
      (void)useless_input_lists.emplace_back(i);
    }
  }
  common::AnfAlgo::SetNodeAttr(kAttrUselessInput, MakeValue(useless_input_lists), node);

  // Update output size list
  AscendKernelMod::UpdateOutputSizeList();

  if (!AclUtils::UpdateTensorDesc(node, &input_desc_list_, &output_desc_list_)) {
    MS_LOG(EXCEPTION) << "Fail to update op desc: " << node->fullname_with_scope();
  }
  return 0;
}

bool AclKernelMod::SkipUnRunNode(const std::vector<AddressPtr> &inputs, const std::vector<AddressPtr> &outputs,
                                 void *stream_ptr, const size_t input_size) {
  MS_EXCEPTION_IF_NULL(stream_ptr);
  for (auto &[attr_name, value] : attr_list_) {
    // Special process of dynamic input number.
    if (value == nullptr) {
      continue;
    }
    if (attr_name == kNAttrName && value->isa<Int64Imm>()) {
      auto long_input_size = SizeToLong(input_size);
      if (GetValue<int64_t>(value) != long_input_size) {
        value = MakeValue(long_input_size);
      }
      if (input_size <= 1 && op_type_ == kConcatDOpName) {
        // cppcheck-suppress unreadVariable
        auto lock = device::KernelRuntime::LockRuntime(stream_ptr);
        auto iter = std::find_if(input_size_list_.begin(), input_size_list_.end(),
                                 [](const size_t size) { return size != 0 && size != SIZE_MAX; });
        if (iter == input_size_list_.end()) {
          return true;
        }
        size_t index = iter - input_size_list_.begin();
        if (index >= inputs.size()) {
          return true;
        }
        auto status = aclrtMemcpyAsync(outputs[0]->addr, outputs[0]->size, inputs[index]->addr, inputs[index]->size,
                                       ACL_MEMCPY_DEVICE_TO_DEVICE, stream_ptr);
        if (status != ACL_SUCCESS) {
          MS_LOG(EXCEPTION) << "AclrtMemcpyAsync failed for " << anf_node_.lock()->fullname_with_scope();
        }

        MS_LOG(INFO) << "Execute node:" << anf_node_.lock()->fullname_with_scope() << " success.";
        return true;
      }
    }
  }
  return false;
}

void AclKernelMod::ProcessAttribute(const std::shared_ptr<AclOpDesc> &op_desc_ptr) {
  auto node = anf_node_.lock();
  const auto &attr_to_input_maps = GeOpConvertor::GetNeedAddInput(node, true);
  const auto &input_names = kernel::AclUtils::GetOpInputAnchorNames(node);
  for (auto &[attr_name, value] : attr_list_) {
    if (value == nullptr) {
      MS_LOG(WARNING) << "Current node's attr [" << attr_name << "] is nullptr";
      continue;
    }
    MS_LOG(INFO) << attr_name << " --- " << value->ToString();
    if (attr_to_input_maps.count(attr_name) != 0) {
      auto to_input_name = attr_to_input_maps.at(attr_name);
      MS_LOG(INFO) << to_input_name << " --- " << input_names;
      auto iter = std::find(input_names.begin(), input_names.end(), to_input_name);
      if (iter == input_names.end()) {
        MS_LOG(EXCEPTION) << "Adaptor's attr name " << to_input_name << " isn't match any input name:" << input_names;
      }
      op_desc_ptr->ProcessAclAttrs(attr_name, value, SET_ACL_INPUT);
      continue;
    }
    op_desc_ptr->ProcessAclAttrs(attr_name, value, SET_ACL_ATTR);
  }
}

bool AclKernelMod::Launch(const std::vector<AddressPtr> &inputs, const std::vector<AddressPtr> &,
                          const std::vector<AddressPtr> &outputs, void *stream_ptr) {
  if (stream_ptr == nullptr) {
    MS_LOG(ERROR) << "stream_ptr should not be nullptr.";
    return false;
  }
  auto node = anf_node_.lock();
  auto op_desc_ptr = std::make_shared<AclOpDesc>(op_type_, node);
  MS_EXCEPTION_IF_NULL(op_desc_ptr);
  op_desc_ptr->AddTensorDesc(input_desc_list_, output_desc_list_);
  op_desc_ptr->AddDataBuf(inputs, input_size_list_, outputs, output_size_list_);
  if (SkipUnRunNode(inputs, outputs, stream_ptr, op_desc_ptr->input_tensor_desc().size())) {
    return true;
  }
  ProcessAttribute(op_desc_ptr);

  // cppcheck-suppress unreadVariable
  auto lock = device::KernelRuntime::LockRuntime(stream_ptr);
  // Current enable binary->fuzz->stable mode.
  auto set_compile_flag = ACL_SUCCESS;
  if (is_dynamic_) {
    set_compile_flag = aclopSetCompileFlag(ACL_OP_COMPILE_FUZZ);
  } else {
    set_compile_flag = aclopSetCompileFlag(ACL_OP_COMPILE_DEFAULT);
  }
  if (set_compile_flag != ACL_SUCCESS) {
    MS_LOG(ERROR) << "Acl set compile mode failed! op_name is " << op_type_ << " and error flag is "
                  << set_compile_flag;
    return false;
  }

  MS_LOG(INFO) << "Start aclopCompileAndExecute of node: " << op_type_;
  bool ret = aclopCompileAndExecute(const_cast<char *>(op_type_.c_str()), op_desc_ptr->input_tensor_desc().size(),
                                    op_desc_ptr->input_tensor_desc().data(), op_desc_ptr->input_tensor_data().data(),
                                    op_desc_ptr->output_tensor_desc().size(), op_desc_ptr->output_tensor_desc().data(),
                                    op_desc_ptr->output_tensor_data().data(), op_desc_ptr->acl_attr(), ACL_ENGINE_SYS,
                                    ACL_COMPILE_SYS, nullptr, stream_ptr);
  if (ret != ACL_SUCCESS) {
    MS_LOG(ERROR) << "Acl compile and execute failed! op_name is " << op_type_ << " and op info is "
                  << anf_node_.lock()->DebugString();
    return false;
  }

  MS_LOG(INFO) << "Success launch of node: " << op_type_;
  return true;
}

std::vector<TaskInfoPtr> AclKernelMod::GenTask(const std::vector<AddressPtr> &, const std::vector<AddressPtr> &,
                                               const std::vector<AddressPtr> &, uint32_t) {
  return {};
}

void AclKernelMod::SyncData() {}
}  // namespace kernel
}  // namespace mindspore
