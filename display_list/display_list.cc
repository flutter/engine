// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>
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

const SkSamplingOptions DisplayList::NearestSampling =
    SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
const SkSamplingOptions DisplayList::LinearSampling =
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
const SkSamplingOptions DisplayList::MipmapSampling =
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
const SkSamplingOptions DisplayList::CubicSampling =
    SkSamplingOptions(SkCubicResampler{1 / 3.0f, 1 / 3.0f});

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
                         bool can_apply_group_opacity,
                         std::vector<DisplayVirtualLayerInfo> indexes)
    : storage_(ptr),
      byte_count_(byte_count),
      op_count_(op_count),
      nested_byte_count_(nested_byte_count),
      nested_op_count_(nested_op_count),
      bounds_({0, 0, -1, -1}),
      bounds_cull_(cull_rect),
      can_apply_group_opacity_(can_apply_group_opacity),
      virtual_layer_indexes_(indexes) {
  virtual_bounds_valid_ = false;
  static std::atomic<uint32_t> nextID{1};
  do {
    unique_id_ = nextID.fetch_add(+1, std::memory_order_relaxed);
  } while (unique_id_ == 0);
}

DisplayList::~DisplayList() {
  uint8_t* ptr = storage_.get();
  DisposeOps(ptr, ptr + byte_count_);
}

static bool CompareOps(uint8_t* ptrA,
                       uint8_t* endA,
                       uint8_t* ptrB,
                       uint8_t* endB) {
  // These conditions are checked by the caller...
  FML_DCHECK((endA - ptrA) == (endB - ptrB));
  FML_DCHECK(ptrA != ptrB);
  uint8_t* bulkStartA = ptrA;
  uint8_t* bulkStartB = ptrB;
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
        auto bulkBytes = reinterpret_cast<const uint8_t*>(opA) - bulkStartA;
        if (bulkBytes > 0) {
          if (memcmp(bulkStartA, bulkStartB, bulkBytes) != 0) {
            return false;
          }
        }
        bulkStartA = ptrA;
        bulkStartB = ptrB;
        break;
    }
  }
  if (ptrA != endA || ptrB != endB) {
    return false;
  }
  if (bulkStartA < ptrA) {
    // Perform a final bulk compare if we have remaining bytes waiting
    if (memcmp(bulkStartA, bulkStartB, ptrA - bulkStartA) != 0) {
      return false;
    }
  }
  return true;
}

void DisplayList::DispatchPart(Dispatcher& ctx, int start, int end) const {
  uint8_t* ptr = storage_.get();

  uint8_t* opEnd = ptr + byte_count_;

  uint8_t* targetStartPtr = nullptr;
  uint8_t* targetEndPtr = nullptr;

  int count = 0;
  while (ptr < opEnd) {
    if (count == start && targetStartPtr == nullptr) {
      targetStartPtr = ptr;
    }
    if (count == end) {
      targetEndPtr = ptr;
      break;
    }
    auto op = reinterpret_cast<const DLOp*>(ptr);
    ptr += op->size;
    count += 1;
  }

  if (targetEndPtr == nullptr) {
    targetEndPtr = opEnd;
  }

  Dispatch(ctx, targetStartPtr, targetEndPtr);
}

const SkRect& DisplayList::bounds() {
  if (!virtual_layer_indexes_.empty() && virtual_bounds_valid_) {
    virtual_bounds_valid_ = false;
    return virtual_bounds_;
  }

  if (bounds_.width() < 0.0) {
    // ComputeBounds() will leave the variable with a
    // non-negative width and height
    ComputeBounds();
  }
  return bounds_;
}

uint8_t* getNPtr(uint8_t* ptr, int n) {
  for (int i = 0; i < n; i++) {
    auto op = reinterpret_cast<const DLOp*>(ptr);
    ptr += op->size;
  }
  return ptr;
}

void DisplayList::Compare(DisplayList* dl) {
  if (storage_ == nullptr) {
    return;
  }

  // 0. Diff alg.
  SkRect damage;

  const auto current = virtual_layer_indexes();
  const auto old = dl->virtual_layer_indexes();

  uint8_t* currentOpHead = storage_.get();
  uint8_t* oldOpHead = dl->storage_.get();

  // 1. 算法实现1 简易的深搜.
  std::set<int> oldUsage;
  for (unsigned long i = 0; i < current.size(); i += 2) {
    for (unsigned long j = 0; j < old.size(); j += 2) {
      if (oldUsage.find(i) == oldUsage.end() &&
          current[i].type == old[j].type) {
        auto curH = getNPtr(currentOpHead, current[i].index);
        auto curE = getNPtr(currentOpHead, current[i + 1].index);
        auto oldH = getNPtr(oldOpHead, old[j].index);
        auto oldE = getNPtr(oldOpHead, old[j + 1].index);
        if (curE - curH == oldE - oldH && CompareOps(curH, curE, oldH, oldE)) {
          oldUsage.insert(j);
          break;
        } else {
          auto rect = partBounds(current[i].index, current[i + 1].index);
          damage.join(rect);
          break;
        }
      }
    }
  }

  for(unsigned long i = 0; i < old.size(); i += 2) {
    if(oldUsage.find(i) == oldUsage.end()) {
      auto rect = partBounds(old[i].index, current[i+1].index);
      damage.join(rect);
    }
  }

  // 3. Fake
//  damage = partBounds(current[0].index, current[current.size()-1].index);


  // 2. 算法实现2 树状结构下的深层diff.
//  // 1. preroll 找到具有op的有效indexes.
////  vector<int> trash;
////  std::set<std::string> trashNames;
//
//  std::function<void()> diffNode = [&] (int currentStart, int currentEnd, int oldStart, int oldEnd) -> void {
//    // currS - currE 之间是一个子树，// oldStart - oldEnd 之间是一个子树
//
//    auto currT = current[currentStart].type;
//
//    for(int i = oldStart; i < oldEnd; i++) {
//      bool res;
//      if(res) {
//      // diff 成功则删除 old
//      // oldgarbage.push_back
//        return;
//      }
//    }
//
//    // 否则把自己删掉
//    damage.add
//  }
//
//  // 遍历每个新的indexes，old里面有相同就删掉，最后把old中剩下的都标为dirty.
//  std::function<void()> diffTree = [&] (int currentStart, int currentEnd, int oldStart, int oldEnd) -> void {
//
//    // end > start
//
//    // 0. 找到最初有效的 start
//    int i = currentStart;
//    while (i < currentEnd-1) {
//      if(current[i].index == current[i+1].index) {
//        i++;
//      }else{
//        break
//      }
//    }
//
//    int j = oldStart;
//    while(j<oldEnd-1) {
//      if(old[i].index == old[i+1].index) {
//        j++;
//      }else{
//        break;
//      }
//    }
//
//    // 1. 父节点比较
//    if(current[i].type == old[j].type) {
//
////      // 看看是不是叶子节点
////    if(current[i].index = current[currentEnd].index-1) {
////      // 把这个叶子节点拿去跟 old里面的所有同级节点比较.
////    }
//
//      // 1.1 看看op，不相同也是都寄了, old里面留着不动.
//      // compare(op)
//      // damage add current(currentStart, currentEnd);
//
//      // 把所有子节点找出去比较old的所有子节点.
//      // stack find end -> diff -> bool
//
//
//    }else{
//      // 1.2 父节点类型不相同直接寄
//      // damage add current(currentStart, currentEnd);
//    }
//  }

  // 2. Add damage.
  setVirtualBounds(damage);
  virtual_bounds_valid_ = true;
}

void DisplayList::ComputeBounds() {
  DisplayListBoundsCalculator calculator(&bounds_cull_);
  Dispatch(calculator);
  bounds_ = calculator.bounds();
}

const SkRect DisplayList::partBounds(int start, int end) {
  DisplayListBoundsCalculator calculator(&bounds_cull_);
  DispatchPart(calculator, start, end);
  return calculator.bounds();
}

void DisplayList::Dispatch(Dispatcher& dispatcher,
                           uint8_t* ptr,
                           uint8_t* end) const {
  TRACE_EVENT0("flutter", "DisplayList::Dispatch");
  while (ptr < end) {
    auto op = reinterpret_cast<const DLOp*>(ptr);
    ptr += op->size;
    FML_DCHECK(ptr <= end);
    switch (op->type) {
#define DL_OP_DISPATCH(name)                                \
  case DisplayListOpType::k##name:                          \
    static_cast<const name##Op*>(op)->dispatch(dispatcher); \
    break;

      FOR_EACH_DISPLAY_LIST_OP(DL_OP_DISPATCH)

#undef DL_OP_DISPATCH

      default:
        FML_DCHECK(false);
        return;
    }
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

void DisplayList::RenderTo(DisplayListBuilder* builder,
                           SkScalar opacity) const {
  // TODO(100983): Opacity is not respected and attributes are not reset.
  if (!builder) {
    return;
  }
  Dispatch(*builder);
}

void DisplayList::RenderTo(SkCanvas* canvas, SkScalar opacity) const {
  DisplayListCanvasDispatcher dispatcher(canvas, opacity);
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
