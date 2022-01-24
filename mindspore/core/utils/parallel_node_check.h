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

#ifndef MINDSPORE_CORE_UTILS_PARALLEL_NODE_CHECK_H_
#define MINDSPORE_CORE_UTILS_PARALLEL_NODE_CHECK_H_

#include "ir/primitive.h"

namespace mindspore {
MS_CORE_API bool IsInParallelBlackList(const PrimitivePtr &);
MS_CORE_API bool IsInAllGatherNodeList(const CNodePtr &);
MS_CORE_API bool IsInTrivialNodeList(const CNodePtr &);
MS_CORE_API bool IsParallelConsiderCNode(const CNodePtr &);
}  // namespace mindspore
#endif  // MINDSPORE_CORE_UTILS_PARALLEL_NODE_CHECK_H_
