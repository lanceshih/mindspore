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
#include "backend/optimizer/ascend/ir_fission/space_to_depth_split.h"
#include <vector>
#include <memory>
#include <string>
#include "backend/session/anf_runtime_algorithm.h"
#include "frontend/optimizer/opt.h"
#include "backend/optimizer/common/helper.h"

namespace mindspore {
namespace opt {
namespace {
constexpr size_t kFloat16Len = 2;
constexpr size_t kSpaceToDepthInputNum = 1;
constexpr size_t kInputIndex1 = 1;
constexpr size_t DIM1 = 1;
constexpr size_t DIM2 = 2;
constexpr size_t DIM3 = 3;

tensor::TensorPtr CreateTensor(const AnfNodePtr &node) {
  // 1 create tensor
  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);
  auto input_x = cnode->input(kSpaceToDepthInputNum);
  int64_t block_size = AnfAlgo::GetNodeAttr<int64_t>(cnode, "block_size");
  std::vector<size_t> x_shape = AnfAlgo::GetOutputInferShape(input_x, 0);
  int64_t input_channel = SizeToLong(x_shape[DIM1]);
  int64_t assist_input_channel = SizeToLong(x_shape[DIM1]) * block_size * block_size;
  std::vector<int64_t> assist_input_shape = {assist_input_channel, input_channel, block_size, block_size};
  int64_t dest_size = assist_input_channel * input_channel * block_size * block_size;
  MS_LOG(DEBUG) << "For SpaceToDepth op, assist input shape is: (" << assist_input_channel << ", " << input_channel
                << ", " << block_size << ", " << block_size << ")";
  TensorTypePtr tensor_type = std::make_shared<TensorType>(kFloat16);
  tensor::DeviceInfo device_info{kOpFormat_NC1HWC0, tensor_type};
  tensor::TensorPtr assist_tensor = std::make_shared<tensor::Tensor>(kFloat16->type_id(), assist_input_shape);
  assist_tensor->set_device_info(device_info);

  // 2 set value of tensor
  int64_t window_size = assist_input_shape[DIM2] * assist_input_shape[DIM3];
  int64_t channel_size = input_channel;
  auto data_ptr = assist_tensor->data_c();
  MS_EXCEPTION_IF_NULL(data_ptr);
  std::vector<float16> half_data(dest_size);
  float16 assist_value = float16(static_cast<float>(1));
  for (int64_t window_idx = 0; window_idx < window_size; window_idx++) {
    for (int64_t channel_idx = 0; channel_idx < channel_size; channel_idx++) {
      int64_t stride_idx = channel_idx * (channel_size + 1) * window_size;
      int64_t fill_assist_idx = stride_idx + (window_idx * (channel_size * channel_size * window_size + 1));
      half_data[fill_assist_idx] = assist_value;
    }
  }
  auto elem_num = LongToSize(dest_size) * kFloat16Len;
  auto ret_code = memcpy_s(data_ptr, static_cast<size_t>(assist_tensor->data().nbytes()), half_data.data(), elem_num);
  if (ret_code != 0) {
    MS_LOG(ERROR) << "Failed to copy data into Tensor while creating assist input for SpaceToDepth op.";
    return nullptr;
  }
  return assist_tensor;
}

ValueNodePtr CreateValueNode(const AnfNodePtr &node) {
  tensor::TensorPtr filter_tensor = CreateTensor(node);
  MS_EXCEPTION_IF_NULL(filter_tensor);
  auto assist_const = std::make_shared<ValueNode>(filter_tensor);
  auto assist_abstract = filter_tensor->ToAbstract();
  assist_const->set_abstract(assist_abstract);
  auto assist_kernel_info = std::make_shared<device::KernelInfo>();
  assist_const->set_kernel_info(assist_kernel_info);
  kernel::KernelBuildInfo::KernelBuildInfoBuilder op_builder;
  op_builder.SetOutputsFormat({kOpFormat_NC1HWC0});
  op_builder.SetOutputsDeviceType({kNumberTypeFloat16});
  AnfAlgo::SetSelectKernelBuildInfo(op_builder.Build(), assist_const.get());
  return assist_const;
}
}  // namespace

const BaseRef SpaceToDepthSplit::DefinePattern() const {
  VarPtr Xs = std::make_shared<SeqVar>();
  auto space_to_depth_prim = std::make_shared<Primitive>(kSpaceToDepthOpName);
  return VectorRef({space_to_depth_prim, Xs});
}

const AnfNodePtr SpaceToDepthSplit::Process(const FuncGraphPtr &graph, const AnfNodePtr &node, const EquivPtr &) const {
  MS_EXCEPTION_IF_NULL(graph);
  MS_EXCEPTION_IF_NULL(node);
  auto kernel_graph = graph->cast<KernelGraphPtr>();
  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);
  if (cnode->size() != kSpaceToDepthInputNum + 1) {
    MS_LOG(INFO) << "The node " << cnode->DebugString() << " is not equal to " << kSpaceToDepthInputNum << " inputs";
    return nullptr;
  }
  const auto &ori_inputs = cnode->inputs();
  TypeId x_dtype = AnfAlgo::GetOutputInferDataType(ori_inputs[kInputIndex1], 0);
  if (x_dtype != kNumberTypeFloat16) {
    MS_LOG(INFO) << "Node " << cnode->DebugString() << ": The data type of node's first input is: " << x_dtype
                 << ", not fp16, cannot do fusion.";
    return nullptr;
  }

  std::vector<AnfNodePtr> new_inputs{NewValueNode(std::make_shared<Primitive>(kSpaceToDepthOpName))};
  auto assist_const = CreateValueNode(cnode);
  new_inputs.insert(new_inputs.end(), cnode->inputs().begin() + 1, cnode->inputs().end());
  new_inputs.push_back(assist_const);
  CNodePtr new_cnode = graph->NewCNode(new_inputs);
  MS_EXCEPTION_IF_NULL(new_cnode);
  new_cnode->set_abstract(cnode->abstract());
  new_cnode->set_scope(cnode->scope());
  AnfAlgo::CopyNodeAttrs(cnode, new_cnode);
  if (kernel_graph != nullptr) {
    kernel_graph->AddValueNodeToGraph(assist_const);
    MS_LOG(INFO) << "Split SpaceToDepth op success.";
  }
  return new_cnode;
}
}  // namespace opt
}  // namespace mindspore
