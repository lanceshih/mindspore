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

#ifndef MINDSPORE_CCSRC_FRONTEND_PARALLEL_SILENT_CHECK_SILENT_CHECK_H_
#define MINDSPORE_CCSRC_FRONTEND_PARALLEL_SILENT_CHECK_SILENT_CHECK_H_

#include "base/base.h"
#include "ir/manager.h"

namespace mindspore {
namespace parallel {
constexpr char GLOBAL_STEP[] = "global_step";
constexpr char NPU_DETECT[] = "NPU_DETECT";
constexpr char LOSS_SCALE[] = "loss_scale";
constexpr size_t GLOBAL_STEP_INDEX = 5;
constexpr size_t LOSS_SCALE_INDEX = 7;
class SilentCheck {
 public:
  SilentCheck(const FuncGraphPtr &root, const FuncGraphManagerPtr &mng) : root_(root), mng_(mng) {}
  virtual ~SilentCheck() = default;
  void GetLossScaleAndGlobalStep();
  void ModifySilentCheckOps();

 private:
  FuncGraphPtr root_ = nullptr;
  FuncGraphManagerPtr mng_ = nullptr;
  AnfNodePtr loss_scale_ = nullptr;
  AnfNodePtr global_step_ = nullptr;
};
}  // namespace parallel
}  // namespace mindspore

#endif  // MINDSPORE_CCSRC_FRONTEND_PARALLEL_SILENT_CHECK_SILENT_CHECK_H_
