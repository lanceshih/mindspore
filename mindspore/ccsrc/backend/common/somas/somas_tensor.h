/**
 * Copyright 2020-2021 Huawei Technologies Co., Ltd

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef MINDSPORE_CCSRC_BACKEND_COMMON_SOMAS_SOMAS_TENSOR_H
#define MINDSPORE_CCSRC_BACKEND_COMMON_SOMAS_SOMAS_TENSOR_H

#include <memory>
#include <set>
#include <vector>

#include "utils/hash_map.h"
#include "backend/common/somas/somas_solver_pre.h"

namespace mindspore {
namespace somas {
// Lifetime type
struct Lifetime {
  size_t start_;
  size_t end_;

  explicit Lifetime(size_t start = 0, size_t end = 0) : start_(start), end_(end) {}
};
using lifetime_t = struct Lifetime;

// Tensor type
enum TensorType {
  kCommon,
  kOutputOnly,
  kWorkspace,
  kGetNextOutput,
  kSummaryInput,
  kRefNodeInput,
  kRefNodeOutput,
  kEventVirtualOutput,
  kUnknown
};

enum LifeLongType {
  kLifeLongNone,        // life time is from tensor start to tensor end
  kLifeLongGraphAll,    // life time is  from graph start to graph end
  kLifeLongGraphStart,  // life time is  from graph start to tensor end
  kLifeLongGraphEnd     // life time is  from tensor start to graph end
};

class SomasTensor {
 public:
  size_t aligned_size_{0};
  LifeLongType lifelong_value_;

  bool between_streams_;
  bool contiguous_;

  lifetime_t lifetime_;
  TensorType type_;

  size_t offset_{0};

  std::set<size_t> destination_nodes_;
  vector<size_t> consumer_list_;

  // Constructors/Destructors
  explicit SomasTensor(size_t id, size_t source_node_id, size_t source_stream_id, size_t real_size,
                       LifeLongType lifelong_value = kLifeLongNone);
  SomasTensor(const SomasTensor &) = delete;
  SomasTensor &operator=(const SomasTensor &) = delete;
  ~SomasTensor() = default;

  // Accessors
  const size_t &GetId() const { return id_; }
  size_t GetSourceNodeId() const { return source_node_id_; }
  size_t GetSourceStreamId() const { return source_stream_id_; }
  const size_t &GetOriginalSize() const { return original_size_; }
  const size_t &GetAlignedSize() const { return aligned_size_; }
  const size_t &GetNumConstraints() const { return num_constraints_; }
  bool IsLifelong() const { return lifelong_value_ == kLifeLongGraphAll; }
  bool IsWorkspace() const { return type_ == kWorkspace; }
  bool IsOutputOnly() const { return type_ == kOutputOnly; }
  size_t GetOffset() const { return offset_; }
  bool IsBetweenStreams() const { return between_streams_; }
  bool IsSemiLifelongStart() const { return lifelong_value_ == kLifeLongGraphStart; }
  bool IsSemiLifelongEnd() const { return lifelong_value_ == kLifeLongGraphEnd; }
  bool IsRefOverlap() const { return ref_overlap_; }

  // Computing functions
  void SetOffset() {
    if (aligned_size_ != 0) {
      offset_ = solver_tensor_desc_->offset_;
    }
  }
  SomasSolverTensorDescPtr GetSolverTensorDesc();
  size_t num_constraints_{0};

 private:
  bool ref_overlap_;
  const size_t id_{0};
  const size_t source_node_id_;
  const size_t source_stream_id_;
  const size_t original_size_{0};

  SomasSolverTensorDescPtr solver_tensor_desc_;
};
using SomasTensorPtr = std::shared_ptr<SomasTensor>;
}  // namespace somas
}  // namespace mindspore

#endif  // MINDSPORE_CCSRC_BACKEND_COMMON_SOMAS_SOMAS_TENSOR_H
