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
#ifndef MINDSPORE_CORE_OPS_MFCC_H_
#define MINDSPORE_CORE_OPS_MFCC_H_
#include <vector>
#include <memory>

#include "ops/primitive_c.h"
#include "abstract/abstract_value.h"
#include "utils/check_convert_utils.h"

namespace mindspore {
namespace ops {
constexpr auto kNameMfcc = "Mfcc";
class MS_CORE_API Mfcc : public PrimitiveC {
 public:
  Mfcc() : PrimitiveC(kNameMfcc) {}
  ~Mfcc() = default;
  MS_DECLARE_PARENT(Mfcc, PrimitiveC);
  void Init(const float freq_upper_limit, const float freq_lower_limit, const int64_t filter_bank_channel_num,
            const int64_t dct_coeff_num);
  void set_freq_upper_limit(const float freq_upper_limit);
  void set_freq_lower_limit(const float freq_lower_limit);
  void set_filter_bank_channel_num(const int64_t filter_bank_channel_num);
  void set_dct_coeff_num(const int64_t dct_coeff_num);
  float get_freq_upper_limit() const;
  float get_freq_lower_limit() const;
  int64_t get_filter_bank_channel_num() const;
  int64_t get_dct_coeff_num() const;
};
AbstractBasePtr MfccInfer(const abstract::AnalysisEnginePtr &, const PrimitivePtr &primitive,
                          const std::vector<AbstractBasePtr> &input_args);
}  // namespace ops
}  // namespace mindspore

#endif  // MINDSPORE_CORE_OPS_MFCC_H_
