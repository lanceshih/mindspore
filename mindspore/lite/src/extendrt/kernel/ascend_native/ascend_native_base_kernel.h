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

#ifndef MINDSPORE_LITE_SRC_EXTENDRT_KERNEL_ASCEND_NATIVE_BASE_KERNEL_H_
#define MINDSPORE_LITE_SRC_EXTENDRT_KERNEL_ASCEND_NATIVE_BASE_KERNEL_H_

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "extendrt/delegate/type.h"
#include "extendrt/delegate/ascend_native/ascend_native_impl/utils.h"
#include "extendrt/kernel/base_kernel.h"
#include "ops/base_operator.h"
#include "ops/op_name.h"

namespace mindspore {
namespace common {
using KernelWithIndex = std::pair<AnfNodePtr, size_t>;
}  // namespace common
struct KernelWithIndexAndTensor {
  KernelWithIndexAndTensor() = default;
  KernelWithIndexAndTensor(common::KernelWithIndex kernel_index, kernel::InferTensor *tensor_info)
      : kernel_index(kernel_index), tensor_info(tensor_info) {}

  common::KernelWithIndex kernel_index;
  kernel::InferTensor *tensor_info;
};
namespace kernel {
class AscendNativeBaseKernel : public BaseKernel {
 public:
  AscendNativeBaseKernel() = delete;

  AscendNativeBaseKernel(const AscendNativeBaseKernel &kernel) = delete;

  AscendNativeBaseKernel(AscendNativeBaseKernel &&other) = delete;

  AscendNativeBaseKernel &operator=(const AscendNativeBaseKernel &kernel) = delete;

  AscendNativeBaseKernel &operator=(AscendNativeBaseKernel &&src) = delete;

  AscendNativeBaseKernel(InferPrimitive prim, std::shared_ptr<kernel::InferContext> *ctx, const void *stream,
                         std::string name)
      : BaseKernel(prim, ctx), stream_(stream), name_(name) {}

  AscendNativeBaseKernel(const std::vector<InferTensor *> &inputs, const std::vector<InferTensor *> &outputs,
                         InferPrimitive prim, std::shared_ptr<kernel::InferContext> *ctx, const void *stream,
                         std::string name)
      : BaseKernel(prim, ctx), stream_(stream), name_(name) {
    BaseKernel::set_in_tensors(inputs);
    BaseKernel::set_out_tensors(outputs);
  }

  void set_stream(const void *stream) { stream_ = stream; }
  const void *get_stream() { return stream_; }
  const std::string get_name() const { return name_; }
  void set_name(std::string name) { name_ = name; }

  virtual bool IsWeightInputHanledInner() const { return false; }
  virtual bool isFormatAndTypeSupport(int index, TypeId type, Format fmt) { return true; }
  virtual int get_workspace_size() const { return 0; }

 protected:
  const void *stream_ = nullptr;
  std::string name_;
  FuncGraphPtr func_graph_;
};
}  // namespace kernel
}  // namespace mindspore
#endif  // MINDSPORE_LITE_SRC_EXTENDRT_KERNEL_ASCEND_NATIVE_BASE_KERNEL_H_
