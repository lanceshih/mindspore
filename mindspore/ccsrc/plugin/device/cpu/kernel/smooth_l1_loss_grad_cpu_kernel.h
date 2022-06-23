/**
 * Copyright 2020-2022 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_CCSRC_PLUGIN_DEVICES_CPU_KERNEL_SMOOTH_L1_LOSS_GRAD_CPU_KERNEL_H_
#define MINDSPORE_CCSRC_PLUGIN_DEVICES_CPU_KERNEL_SMOOTH_L1_LOSS_GRAD_CPU_KERNEL_H_

#include <memory>
#include <unordered_map>
#include <vector>
#include <utility>
#include <map>
#include "plugin/device/cpu/kernel/cpu_kernel.h"
#include "plugin/factory/ms_factory.h"

namespace mindspore {
namespace kernel {
class SmoothL1LossGradCpuKernelMod : public NativeCpuKernelMod, public MatchKernelHelper<SmoothL1LossGradCpuKernelMod> {
 public:
  SmoothL1LossGradCpuKernelMod() = default;
  ~SmoothL1LossGradCpuKernelMod() override = default;

  bool Init(const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
            const std::vector<KernelTensorPtr> &outputs) override;

  int Resize(
    const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
    const std::vector<KernelTensorPtr> &outputs,
    const std::map<uint32_t, tensor::TensorPtr> &inputsOnHost = std::map<uint32_t, tensor::TensorPtr>()) override;

  bool Launch(const std::vector<AddressPtr> &inputs, const std::vector<AddressPtr> &workspace,
              const std::vector<AddressPtr> &outputs) override {
    return kernel_func_(this, inputs, workspace, outputs);
  }

  const std::vector<std::pair<KernelAttr, KernelRunFunc>> &GetFuncList() const override;

 protected:
  std::vector<KernelAttr> GetOpSupport() override { return OpSupport(); }

 private:
  template <typename T>
  bool LaunchKernel(const std::vector<kernel::AddressPtr> &inputs, const std::vector<kernel::AddressPtr> &workspace,
                    const std::vector<kernel::AddressPtr> &outputs);
  enum ReductionType { NONE = 0, MEAN = 1, SUM = 2, INVALID_MODE = 255 };

  template <typename T>
  bool CalNoReduce(const T *predict_addr, const T *target_addr, const T *dloss_addr, T *result_addr);

  template <typename T>
  bool CalMean(const T *predict_addr, const T *target_addr, const T *dloss_addr, T *result_addr);

  template <typename T>
  bool CalSum(const T *predict_addr, const T *target_addr, const T *dloss_addr, T *result_addr);

  float beta_{1.0};
  TypeId dtype_{kTypeUnknown};
  int64_t tensor_size_{1};
  ReductionType reduction_{INVALID_MODE};
};
}  // namespace kernel
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_PLUGIN_DEVICES_CPU_KERNEL_SMOOTH_L1_LOSS_GRAD_CPU_KERNEL_H_
