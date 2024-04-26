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

#include "plugin/device/cpu/kernel/pyboost/customize/mean_ext.h"
#include "plugin/device/cpu/kernel/pyboost/auto_generate/cast.h"
#include "plugin/device/cpu/kernel/pyboost/auto_generate/mean_ext.h"
#include "kernel/pyboost/pyboost_utils.h"
#include "ops/auto_generate/gen_ops_primitive.h"

namespace mindspore {
namespace kernel {
namespace pyboost {
namespace {
void MeanExtCPUCall(const std::shared_ptr<OpRunner> &op, const TensorPtr &input_tensor, const ValuePtr &axis,
                    const BoolImmPtr &keep_dims, const std::vector<AbstractBasePtr> &input_abs) {
  PyBoostUtils::PrepareOpInputs(op->device_context(), op->stream_id(), input_tensor);
  PyBoostUtils::PrepareOpOutputs(op->device_context(), op->stream_id(), op->outputs());

  PyBoostUtils::DispatchRun(
    std::make_shared<runtime::PyBoostDeviceTask>([op, input_tensor, axis, keep_dims, input_abs]() {
      MS_LOG(DEBUG) << "For 'MeanExt', the cpu task 'ReduceMean' start";
      auto device_context = op->device_context();
      const auto &outputs = op->outputs();
      const auto primitive = std::make_shared<Primitive>(prim::kPrimReduceMean->name());
      MS_EXCEPTION_IF_NULL(primitive);

      PyBoostUtils::MallocOpInputs(device_context, input_tensor);
      PyBoostUtils::MallocOpOutputs(device_context, outputs);

      const auto &input_address_info =
        PyBoostUtils::GetAddressInfo(device_context, op->stream_id(), input_abs, input_tensor, axis, keep_dims);
      const auto &output_address_info =
        PyBoostUtils::GetAddressInfo(device_context, op->stream_id(), {op->output_abs()}, outputs);

      PyBoostUtils::LaunchKernel(primitive, device_context, input_address_info, output_address_info);
      MS_LOG(DEBUG) << "For 'MeanExt', the cpu task 'ReduceMean' end";
    }));
}
}  // namespace

void MeanExtCPUCustomize(const std::shared_ptr<OpRunner> &op, const TensorPtr &input_tensor,
                         const std::optional<ValueTuplePtr> &axis, const BoolImmPtr &keep_dims,
                         const std::optional<Int64ImmPtr> &dtype) {
  OpRunner::InferOpOutput(op, input_tensor, axis, keep_dims, dtype);

  ValuePtr act_axis;
  if (axis.has_value()) {
    act_axis = axis.value();
  } else {
    act_axis = MakeValue<std::vector<int64_t>>({});
  }

  // Infer function has confirmed the actual dtype of output
  TypeId out_dtype = op->output_abs()->GetType()->cast<TensorTypePtr>()->element()->type_id();

  TensorPtr act_tensor = input_tensor;
  // Call Cast before Launch ReduceMean
  if (input_tensor->data_type() != out_dtype) {
    MS_LOG(DEBUG) << "Call Cast cpu kernel, src dtype: " << TypeIdToString(input_tensor->data_type())
                  << ", dst dtype: " << TypeIdToString(out_dtype);
    act_tensor =
      PyBoostUtils::CastTensor(input_tensor, out_dtype, op->device_context()->device_context_key_.device_name_);
  }

  // Set new input abstract for ReduceMean
  std::vector<AbstractBasePtr> new_input_abs{act_tensor->ToAbstract(), act_axis->ToAbstract(), keep_dims->ToAbstract()};

  // Check if dtype is matched on ReduceMean kernel
  auto kernel_attr_pair =
    PyBoostUtils::SelectKernel(new_input_abs, op->output_abs(), op->device_context(), prim::kPrimReduceMean->name());
  if (kernel_attr_pair.first) {
    MeanExtCPUCall(op, act_tensor, act_axis, keep_dims, new_input_abs);
  } else {
    auto &select_kernel = kernel_attr_pair.second;
    auto &device_name = op->device_context()->device_context_key_.device_name_;
    const auto &real_input_tensor =
      PyBoostUtils::CastTensor(act_tensor, select_kernel.input_type()[0].dtype, device_name);

    const auto &mean_ext_op = CREATE_PYBOOST_OP(MeanExt, device_name);
    mean_ext_op->set_primitive(prim::kPrimMeanExt);
    const auto out_tensor = mean_ext_op->Call(real_input_tensor, axis, keep_dims, std::nullopt);

    const auto &real_output_tensor = PyBoostUtils::CastTensor(out_tensor, out_dtype, device_name);
    op->set_outputs({real_output_tensor});
  }
}
}  // namespace pyboost
}  // namespace kernel
}  // namespace mindspore
