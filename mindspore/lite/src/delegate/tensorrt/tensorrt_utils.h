/**
 * Copyright 2021 Huawei Technologies Co., Ltd
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
#ifndef MINDSPORE_LITE_SRC_RUNTIME_DELEGATE_TENSORRT_UTILS_H_
#define MINDSPORE_LITE_SRC_RUNTIME_DELEGATE_TENSORRT_UTILS_H_
#include <vector>
#include <NvInfer.h>
#include <memory>
#include "src/delegate/tensorrt/op/tensorrt_op.h"
#include "mindspore/core/ir/dtype/type_id.h"
#include "schema/ops_generated.h"
#include "nnacl/pack.h"

#define kNCHW_N 0
#define kNCHW_C 1
#define kNCHW_H 2
#define kNCHW_W 3
namespace mindspore::lite {
struct ActivationParams {
  nvinfer1::ActivationType activation_type;
  bool has_alpha;
  float alpha;
  bool has_beta;
  float beta;
};
// Convert shape to Cuda Dims.
nvinfer1::Dims ConvertCudaDims(const std::vector<int64_t> &shape);

// Convert Tensor data to Cuda dims.
nvinfer1::Dims ConvertCudaDims(const void *data, int64_t size);

nvinfer1::Dims ConvertCudaDims(int data, size_t size);

std::vector<int64_t> ConvertMSShape(const nvinfer1::Dims dims);

nvinfer1::DataType ConvertDataType(DataType type_id);

nvinfer1::IShuffleLayer *NHWC2NCHW(nvinfer1::INetworkDefinition *network, const nvinfer1::ITensor &input);

nvinfer1::IShuffleLayer *NCHW2NHWC(nvinfer1::INetworkDefinition *network, const nvinfer1::ITensor &input);

ActivationParams ConvertActivationType(schema::ActivationType activation_type);

nvinfer1::ITensor *ConvertConstantTensor(nvinfer1::INetworkDefinition *network, const mindspore::MSTensor &ms_tensor);

nvinfer1::ITensor *ConvertTensorWithExpandDims(nvinfer1::INetworkDefinition *network,
                                               const mindspore::MSTensor &ms_tensor, size_t expand_shape_size);

nvinfer1::ITensor *ConvertScalarToITensor(nvinfer1::INetworkDefinition *network, size_t shape_size, const void *value,
                                          const DataType data_type);

nvinfer1::Weights TransposeWeight(const mindspore::MSTensor &ms_tensor, void **pack_weight);

nvinfer1::Weights TransposeWeightFP32(const mindspore::MSTensor &ms_tensor, void **pack_weight);

nvinfer1::Weights ConvertWeight(const mindspore::MSTensor &ms_tensor);

void SetCudaDevice(std::shared_ptr<GPUDeviceInfo> device_info_);

Format GetOutputFormat(Format input_format, nvinfer1::Permutation perm);

int ConvertAxisFromNHWC2NCHW(int nhwc_axis);

void PackNHWCToNCHWFp16(const void *src, void *dst, size_t batch, size_t plane, size_t channel, size_t task_id,
                        size_t thread_count);
}  // namespace mindspore::lite
#endif  // MINDSPORE_LITE_SRC_RUNTIME_DELEGATE_TENSORRT_UTILS_H_
