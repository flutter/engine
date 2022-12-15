// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <type_traits>

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/display_list_canvas_dispatcher.h"
#include "flutter/display_list/display_list_ops.h"
#include "flutter/display_list/display_list_utils.h"
#include "flutter/fml/trace_event.h"

namespace flutter {

const SaveLayerOptions SaveLayerOptions::kNoAttributes = SaveLayerOptions();
const SaveLayerOptions SaveLayerOptions::kWithAttributes =
    kNoAttributes.with_renders_with_attributes();

DisplayList::DisplayList()
    : byte_count_(0),
      op_count_(0),
      nested_byte_count_(0),
      nested_op_count_(0),
      unique_id_(0),
      bounds_({0, 0, 0, 0}),
      bounds_cull_({0, 0, 0, 0}),
      can_apply_group_opacity_(true) {}

DisplayList::DisplayList(uint8_t* ptr,
                         size_t byte_count,
                         unsigned int op_count,
                         size_t nested_byte_count,
                         unsigned int nested_op_count,
                         const SkRect& cull_rect,
                         bool can_apply_group_opacity)
    : storage_(ptr),
      byte_count_(byte_count),
      op_count_(op_count),
      nested_byte_count_(nested_byte_count),
      nested_op_count_(nested_op_count),
      bounds_({0, 0, -1, -1}),
      bounds_cull_(cull_rect),
      can_apply_group_opacity_(can_apply_group_opacity) {
  static std::atomic<uint32_t> next_id{1};
  do {
    unique_id_ = next_id.fetch_add(+1, std::memory_order_relaxed);
  } while (unique_id_ == 0);
}

DisplayList::~DisplayList() {
  uint8_t* ptr = storage_.get();
  DisposeOps(ptr, ptr + byte_count_);
}

void DisplayList::ComputeBounds() {
  RectBoundsAccumulator accumulator;
  DisplayListBoundsCalculator calculator(accumulator, &bounds_cull_);
  Dispatch(calculator);
  if (calculator.is_unbounded()) {
    FML_LOG(INFO) << "returning partial bounds for unbounded DisplayList";
  }
  bounds_ = accumulator.bounds();
}

void DisplayList::ComputeRTree() {
  RTreeBoundsAccumulator accumulator(&rtree_op_indices_);
  DisplayListBoundsCalculator calculator(accumulator, &bounds_cull_);
  Dispatch(calculator);
  if (calculator.is_unbounded()) {
    FML_LOG(INFO) << "returning partial rtree for unbounded DisplayList";
  }
  rtree_ = accumulator.rtree();
}

class Culler {
 public:
  virtual bool init(DispatchContext& context) = 0;
  virtual void update(DispatchContext& context) = 0;
};
class NopCuller : public Culler {
 public:
  static NopCuller instance;

  bool init(DispatchContext& context) override {
    context.next_render_index = 0;
    return true;
  }
  void update(DispatchContext& context) override {}
};
NopCuller NopCuller::instance = NopCuller();
class VectorCuller : public Culler {
 public:
  VectorCuller(const std::vector<uint32_t>& op_indices,
               const std::vector<int>& rect_indices)
      : op_indices_(op_indices),
        cur_(rect_indices.begin()),
        end_(rect_indices.end()) {}

  bool init(DispatchContext& context) override {
    if (cur_ < end_) {
      context.next_render_index = op_indices_[*cur_++];
      return true;
    } else {
      context.next_render_index = std::numeric_limits<uint32_t>::max();
      return false;
    }
  }
  void update(DispatchContext& context) override {
    if (++context.cur_index > context.next_render_index) {
      while (cur_ < end_) {
        context.next_render_index = op_indices_[*cur_++];
        if (context.next_render_index >= context.cur_index) {
          // It should be rare that we have duplicate indices
          // but if we do, then having a while loop is a cheap
          // insurance for those cases.
          return;
        }
      }
      context.next_render_index = std::numeric_limits<uint32_t>::max();
    }
  }

 private:
  const std::vector<uint32_t>& op_indices_;
  std::vector<int>::const_iterator cur_;
  std::vector<int>::const_iterator end_;
};

void DisplayList::Dispatch(Dispatcher& ctx) const {
  uint8_t* ptr = storage_.get();
  Dispatch(ctx, ptr, ptr + byte_count_, NopCuller::instance);
}
void DisplayList::Dispatch(Dispatcher& ctx, const SkRect& cull_rect) {
  if (cull_rect.isEmpty()) {
    return;
  }
  if (cull_rect.contains(bounds())) {
    Dispatch(ctx);
    return;
  }
  uint8_t* ptr = storage_.get();
  std::vector<int> rect_indices;
  rtree()->search(cull_rect, &rect_indices);
  VectorCuller culler(rtree_op_indices_, rect_indices);
  Dispatch(ctx, ptr, ptr + byte_count_, culler);
}

void DisplayList::Dispatch(Dispatcher& dispatcher,
                           uint8_t* ptr,
                           uint8_t* end,
                           Culler& culler) const {
  DispatchContext context = {
      .dispatcher = dispatcher,

      .cur_index = 0,

      .next_restore_index = std::numeric_limits<uint32_t>::max(),
  };
  if (!culler.init(context)) {
    return;
  }

  while (ptr < end) {
    auto op = reinterpret_cast<const DLOp*>(ptr);
    ptr += op->size;
    FML_DCHECK(ptr <= end);
    switch (op->type) {
#define DL_OP_DISPATCH(name)                             \
  case DisplayListOpType::k##name:                       \
    static_cast<const name##Op*>(op)->dispatch(context); \
    break;

      FOR_EACH_DISPLAY_LIST_OP(DL_OP_DISPATCH)

#undef DL_OP_DISPATCH

      default:
        FML_DCHECK(false);
        return;
    }
    culler.update(context);
  }
}

void DisplayList::DisposeOps(uint8_t* ptr, uint8_t* end) {
  while (ptr < end) {
    auto op = reinterpret_cast<const DLOp*>(ptr);
    ptr += op->size;
    FML_DCHECK(ptr <= end);
    switch (op->type) {
#define DL_OP_DISPOSE(name)                            \
  case DisplayListOpType::k##name:                     \
    if (!std::is_trivially_destructible_v<name##Op>) { \
      static_cast<const name##Op*>(op)->~name##Op();   \
    }                                                  \
    break;

      FOR_EACH_DISPLAY_LIST_OP(DL_OP_DISPOSE)

#undef DL_OP_DISPOSE

      default:
        FML_DCHECK(false);
        return;
    }
  }
}

static bool CompareOps(uint8_t* ptrA,
                       uint8_t* endA,
                       uint8_t* ptrB,
                       uint8_t* endB) {
  // These conditions are checked by the caller...
  FML_DCHECK((endA - ptrA) == (endB - ptrB));
  FML_DCHECK(ptrA != ptrB);
  uint8_t* bulk_start_a = ptrA;
  uint8_t* bulk_start_b = ptrB;
  while (ptrA < endA && ptrB < endB) {
    auto opA = reinterpret_cast<const DLOp*>(ptrA);
    auto opB = reinterpret_cast<const DLOp*>(ptrB);
    if (opA->type != opB->type || opA->size != opB->size) {
      return false;
    }
    ptrA += opA->size;
    ptrB += opB->size;
    FML_DCHECK(ptrA <= endA);
    FML_DCHECK(ptrB <= endB);
    DisplayListCompare result;
    switch (opA->type) {
#define DL_OP_EQUALS(name)                              \
  case DisplayListOpType::k##name:                      \
    result = static_cast<const name##Op*>(opA)->equals( \
        static_cast<const name##Op*>(opB));             \
    break;

      FOR_EACH_DISPLAY_LIST_OP(DL_OP_EQUALS)

#undef DL_OP_EQUALS

      default:
        FML_DCHECK(false);
        return false;
    }
    switch (result) {
      case DisplayListCompare::kNotEqual:
        return false;
      case DisplayListCompare::kUseBulkCompare:
        break;
      case DisplayListCompare::kEqual:
        // Check if we have a backlog of bytes to bulk compare and then
        // reset the bulk compare pointers to the address following this op
        auto bulk_bytes = reinterpret_cast<const uint8_t*>(opA) - bulk_start_a;
        if (bulk_bytes > 0) {
          if (memcmp(bulk_start_a, bulk_start_b, bulk_bytes) != 0) {
            return false;
          }
        }
        bulk_start_a = ptrA;
        bulk_start_b = ptrB;
        break;
    }
  }
  if (ptrA != endA || ptrB != endB) {
    return false;
  }
  if (bulk_start_a < ptrA) {
    // Perform a final bulk compare if we have remaining bytes waiting
    if (memcmp(bulk_start_a, bulk_start_b, ptrA - bulk_start_a) != 0) {
      return false;
    }
  }
  return true;
}

void DisplayList::RenderTo(DisplayListBuilder* builder,
                           SkScalar opacity, bool cull) {
  // TODO(100983): Opacity is not respected and attributes are not reset.
  if (!builder) {
    return;
  }
  if (cull) {
    SkRect clip_bounds = builder->getLocalClipBounds();
    if (!clip_bounds.contains(bounds())) {
      Dispatch(*builder, clip_bounds);
      return;
    }
  }
  Dispatch(*builder);
}

void DisplayList::RenderTo(SkCanvas* canvas, SkScalar opacity, bool cull) {
  DisplayListCanvasDispatcher dispatcher(canvas, opacity);
  if (cull) {
    SkRect clip_bounds = canvas->getLocalClipBounds();
    if (!clip_bounds.contains(bounds())) {
      Dispatch(dispatcher, clip_bounds);
      return;
    }
  }
  Dispatch(dispatcher);
}

bool DisplayList::Equals(const DisplayList* other) const {
  if (this == other) {
    return true;
  }
  if (byte_count_ != other->byte_count_ || op_count_ != other->op_count_) {
    return false;
  }
  uint8_t* ptr = storage_.get();
  uint8_t* o_ptr = other->storage_.get();
  if (ptr == o_ptr) {
    return true;
  }
  return CompareOps(ptr, ptr + byte_count_, o_ptr, o_ptr + other->byte_count_);
}

}  // namespace flutter
