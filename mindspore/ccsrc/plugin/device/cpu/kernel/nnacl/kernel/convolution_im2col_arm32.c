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
#ifdef ENABLE_ARM32
#include "nnacl/kernel/convolution_im2col_arm32.h"
#include "nnacl/fp32/pack_fp32.h"

void ConvIm2ColARM32InitGlobalVariable(ConvolutionBaseStruct *conv) {
  ConvolutionIm2ColBaseStruct *conv_im2col = (ConvolutionIm2ColBaseStruct *)conv;
  conv_im2col->oc_tile_ = C4NUM;
  conv_im2col->row_tile_ = C12NUM;
  conv_im2col->row_major_to_col_nmajor_ = RowMajor2Col4Major;
}

ConvolutionBaseStruct *CreateConvIm2ColARM32(ConvParameter *conv_param) {
  ConvolutionIm2ColBaseStruct *conv_im2col = (ConvolutionIm2ColBaseStruct *)malloc(sizeof(ConvolutionIm2ColBaseStruct));
  NNACL_MALLOC_CHECK_NULL_RETURN_NULL(conv_im2col);
  memset(conv_im2col, 0, sizeof(ConvolutionIm2ColBaseStruct));

  conv_im2col->init_tmp_buffer_ = ConvIm2ColBaseInitTmpBuffer;

  conv_im2col->conv_.init_global_variable_ = ConvIm2ColARM32InitGlobalVariable;
  conv_im2col->conv_.run_impl_ = ConvIm2ColBaseRunImpl;
  conv_im2col->conv_.pack_weight_ = ConvIm2ColBasePackWeight;

  conv_im2col->conv_.base_.compute = convolution_im2col_base_compute;
  conv_im2col->conv_.base_.prepare = convolution_im2col_base_prepare;
  conv_im2col->conv_.base_.resize = convolution_im2col_base_resize;
  conv_im2col->conv_.base_.release = convolution_im2col_base_release;

  return (ConvolutionBaseStruct *)conv_im2col;
}
#endif
