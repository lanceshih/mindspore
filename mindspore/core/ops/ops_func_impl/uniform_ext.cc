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
#include "ops/ops_func_impl/uniform_ext.h"
#include <utility>
#include <memory>
#include "ops/op_utils.h"
#include "ir/dtype.h"
#include "ops/op_name.h"
#include "utils/check_convert_utils.h"

namespace mindspore {
namespace ops {
BaseShapePtr UniformExtFuncImpl::InferShape(const PrimitivePtr &primitive,
                                            const std::vector<AbstractBasePtr> &input_args) const {
  MS_EXCEPTION_IF_NULL(primitive);
  return input_args[0]->GetShape();
}

TypePtr UniformExtFuncImpl::InferType(const PrimitivePtr &primitive,
                                      const std::vector<AbstractBasePtr> &input_args) const {
  MS_EXCEPTION_IF_NULL(primitive);
  auto prim_name = primitive->name();
  MS_EXCEPTION_IF_NULL(input_args[0]);
  auto x_type = input_args[0]->GetType();
  (void)CheckAndConvertUtils::CheckTensorTypeValid("x", x_type, common_valid_types_with_complex_and_bool, prim_name);
  return x_type;
}
}  // namespace ops
}  // namespace mindspore
