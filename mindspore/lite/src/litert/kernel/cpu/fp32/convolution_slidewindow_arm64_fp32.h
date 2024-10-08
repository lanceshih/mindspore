/**
 * Copyright 2023 Huawei Technologies Co., Ltd
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
#ifndef MINDSPORE_LITE_SRC_LITERT_KERNEL_CPU_FP32_CONVOLUTION_SLIDEWINDOW_ARM64_FP32_H_
#define MINDSPORE_LITE_SRC_LITERT_KERNEL_CPU_FP32_CONVOLUTION_SLIDEWINDOW_ARM64_FP32_H_
#include <vector>
#include "src/litert/kernel/cpu/fp32/convolution_slidewindow_fp32.h"

namespace mindspore::kernel {
class ConvolutionSWARM64CPUKernel : public ConvolutionSWCPUKernel {
 public:
  ConvolutionSWARM64CPUKernel(OpParameter *parameter, const std::vector<lite::Tensor *> &inputs,
                              const std::vector<lite::Tensor *> &outputs, const lite::InnerContext *ctx,
                              float *origin_weight, float *origin_bias)
      : ConvolutionSWCPUKernel(parameter, inputs, outputs, ctx, origin_weight, origin_bias) {}

  void InitGlobalVariable() override;
  int RunImpl(int task_id) override;
};
}  // namespace mindspore::kernel
#endif  // MINDSPORE_LITE_SRC_LITERT_KERNEL_CPU_FP32_CONVOLUTION_SLIDEWINDOW_ARM64_FP32_H_
