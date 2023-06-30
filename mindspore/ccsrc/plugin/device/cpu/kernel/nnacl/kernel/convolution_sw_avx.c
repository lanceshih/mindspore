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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either convolutionress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef ENABLE_AVX
#include "nnacl/kernel/convolution_sw_avx.h"
#include "nnacl/kernel/convolution_slidewindow.h"
#include "nnacl/fp32/conv_1x1_avx_fp32.h"
#include "nnacl/fp32/conv_sw_avx_fp32.h"

void ConvSWAVXInitGlobalVariable(ConvolutionBaseStruct *conv) {
  ConvolutionSWStruct *conv_sw = (ConvolutionSWStruct *)conv;
  NNACL_CHECK_NULL_RETURN_VOID(conv_sw);
  ConvParameter *conv_param = (ConvParameter *)conv->base_.param_;
  NNACL_CHECK_NULL_RETURN_VOID(conv_param);

  conv_sw->oc_tile_ = C8NUM;
  conv_sw->oc_res_ = conv_param->output_channel_ % conv_sw->oc_tile_;
  if (conv_param->kernel_h_ == 1 && conv_param->kernel_w_ == 1) {
    // 1x1 conv is aligned to C8NUM
    conv_sw->in_tile_ = C8NUM;
    conv_sw->ic_res_ = conv_param->input_channel_ % conv_sw->in_tile_;
  }
}

int ConvSWAVXRunImpl(ConvolutionBaseStruct *conv, int task_id) {
  ConvolutionSWStruct *conv_sw = (ConvolutionSWStruct *)conv;
  NNACL_CHECK_NULL_RETURN_ERR(conv_sw);
  ConvParameter *conv_param = (ConvParameter *)conv->base_.param_;
  NNACL_CHECK_NULL_RETURN_ERR(conv_param);

  if (conv_param->kernel_w_ == 1 && conv_param->kernel_h_ == 1) {
    Conv1x1SWAVXFp32(conv_sw->input_data_, (float *)conv->packed_weight_, (float *)conv->bias_data_,
                     conv_sw->output_data_, task_id, conv_param, &conv_sw->sw_param_);
  } else {
    ConvSWAVXFp32(conv_sw->input_data_, (float *)conv->packed_weight_, (float *)conv->bias_data_, conv_sw->output_data_,
                  task_id, conv_param, &conv_sw->sw_param_);
  }
  return NNACL_OK;
}

ConvolutionBaseStruct *CreateConvolutionSWAVX(ConvParameter *conv_param) {
  ConvolutionSWStruct *sw = (ConvolutionSWStruct *)malloc(sizeof(ConvolutionSWStruct));
  NNACL_MALLOC_CHECK_NULL_RETURN_NULL(sw);
  memset(sw, 0, sizeof(ConvolutionSWStruct));

  sw->conv_.run_impl_ = ConvSWAVXRunImpl;
  sw->conv_.init_global_variable_ = ConvSWAVXInitGlobalVariable;
  sw->conv_.pack_weight_ = ConvSWPackWeight;
  sw->conv_.malloc_weight_bias_ = ConvSWMallocWeightBiasData;

  sw->conv_.base_.Compute = ConvolutionSWCompute;
  sw->conv_.base_.Prepare = ConvolutionSWPrepare;
  sw->conv_.base_.Release = ConvolutionSWRelease;
  sw->conv_.base_.Resize = ConvolutionSWResize;

  return (ConvolutionBaseStruct *)sw;
}
#endif
