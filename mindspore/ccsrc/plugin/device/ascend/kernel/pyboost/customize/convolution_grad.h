/**
 * Copyright 2024 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_MINDSPORE_CCSRC_PLUGIN_DEVICE_ASCEND_KERNEL_PYBOOST_CUSTOMIZE_CONVOLUTION_GRAD_H_
#define MINDSPORE_MINDSPORE_CCSRC_PLUGIN_DEVICE_ASCEND_KERNEL_PYBOOST_CUSTOMIZE_CONVOLUTION_GRAD_H_

#include <vector>
#include <memory>
#include <tuple>
#include "ir/tensor.h"
#include "ir/value.h"
#include "runtime/hardware/device_context_manager.h"
#include "kernel/pyboost/op_runner.h"

namespace mindspore {
namespace kernel {
namespace pyboost {
std::tuple<tensor::BaseTensorPtr, tensor::BaseTensorPtr, tensor::BaseTensorPtr> ConvolutionGradAscendCustomize(
  const std::shared_ptr<OpRunner> &op, const BaseTensorPtr &dout_tensor, const BaseTensorPtr &input_tensor,
  const BaseTensorPtr &weight_tensor, const std::optional<BaseTensorPtr> &bias_tensor, const ValueTuplePtr &stride,
  const ValueTuplePtr &pad, const ValueTuplePtr &dilation, const BoolImmPtr &transposed,
  const ValueTuplePtr &output_padding, const Int64ImmPtr &group, const ValueTuplePtr &output_mask);
}  // namespace pyboost
}  // namespace kernel
}  // namespace mindspore
#endif  // MINDSPORE_MINDSPORE_CCSRC_PLUGIN_DEVICE_ASCEND_KERNEL_PYBOOST_CUSTOMIZE_CONVOLUTION_GRAD_H_
