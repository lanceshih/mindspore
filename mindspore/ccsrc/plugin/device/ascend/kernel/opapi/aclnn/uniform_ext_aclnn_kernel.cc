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
#include "plugin/device/ascend/kernel/opapi/aclnn/uniform_ext_aclnn_kernel.h"
#include <utility>
#include <algorithm>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "ir/tensor.h"
#include "runtime/device/kernel_runtime.h"
#include "transform/acl_ir/acl_helper.h"
#include "transform/acl_ir/op_api_convert.h"
#include "abstract/ops/primitive_infer_map.h"
#include "transform/symbol/acl_rt_symbol.h"
#include "transform/symbol/symbol_utils.h"

namespace mindspore {
namespace kernel {
void UniformExtAscend::GetWorkSpaceInfo(const std::vector<KernelTensor *> &inputs,
                                        const std::vector<KernelTensor *> &outputs) {
  a_ = static_cast<double>(transform::ConvertKernelTensor<float>(inputs[kIndex1]));
  b_ = static_cast<double>(transform::ConvertKernelTensor<float>(inputs[kIndex2]));
  seed_ = static_cast<uint64_t>(transform::ConvertKernelTensor<int64_t>(inputs[kIndex3]));
  offset_ = static_cast<uint64_t>(transform::ConvertKernelTensor<int64_t>(inputs[kIndex4]));

  GetWorkspaceForResize(inputs[kIndex0], a_, b_, seed_, offset_);
}

bool UniformExtAscend::Launch(const std::vector<KernelTensor *> &inputs, const std::vector<KernelTensor *> &workspace,
                              const std::vector<KernelTensor *> &outputs, void *stream_ptr) {
  MS_EXCEPTION_IF_NULL(stream_ptr);
  auto status = CALL_ASCEND_API(aclrtMemcpyAsync, outputs[0]->device_ptr(), outputs[0]->size(), inputs[0]->device_ptr(),
                                inputs[0]->size(), ACL_MEMCPY_DEVICE_TO_DEVICE, stream_ptr);
  if (status != ACL_ERROR_NONE) {
    MS_LOG(EXCEPTION) << "MaskedFillAscend Launch and call rtMemcpyAsync failed, ret = 0x" << status;
  }
  ParseGenExecutor(GEN_EXECUTOR_BOOST(op_type_, hash_id_, outputs[kIndex0], a_, b_, seed_, offset_));
  RunOp(stream_ptr, workspace);
  return true;
}

MS_ACLNN_KERNEL_FACTORY_REG(UniformExt, UniformExtAscend);
}  // namespace kernel
}  // namespace mindspore
