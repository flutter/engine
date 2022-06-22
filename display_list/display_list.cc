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

void DisplayList::ComputeBounds() {
  DisplayListBoundsCalculator calculator(&bounds_cull_);
  Dispatch(calculator);
  bounds_ = calculator.bounds();
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

static uint8_t* getNPtr(uint8_t* ptr, uint8_t* end, int n) {
  int i = 0;
  while (ptr < end) {
    if (n == i) {
      return ptr;
    }
    auto op = reinterpret_cast<const DLOp*>(ptr);
    ptr += op->size;
    i += 1;
  }
  return end;
}

void DisplayList::DispatchPart(Dispatcher& ctx, int start, int end) const {
  uint8_t* ptr = storage_.get();
  uint8_t* opEnd = ptr + byte_count_;
  Dispatch(ctx, getNPtr(ptr, opEnd, start), getNPtr(ptr, opEnd, end));
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

const SkRect DisplayList::partBounds(int start, int end) {
  DisplayListBoundsCalculator calculator(&bounds_cull_);
  DispatchPart(calculator, start, end);
  return calculator.bounds();
}

void DisplayList::Compare(DisplayList* dl) {
  if (storage_ == nullptr) {
    return;
  }

  // 0. Diff alg.

  SkRect damage;
  auto newTree = virtual_layer_indexes();
  auto oldTree = dl->virtual_layer_indexes();

  if (newTree.empty() || oldTree.empty() ||
      newTree[0].type != oldTree[0].type) {
    damage = bounds();
    setVirtualBounds(damage);
    virtual_bounds_valid_ = true;
    return;
  }

  // debug check virtual_indexes_.

  std::function<bool(const std::vector<DisplayVirtualLayerInfo>& tree)>
      checkTree =
          [&](const std::vector<DisplayVirtualLayerInfo>& tree) -> bool {
    std::vector<std::string> stack;
    //    bool isLegal = true;
    for (unsigned long i = 0; i < tree.size(); i++) {
      if (tree[i].isStart) {
        stack.push_back(tree[i].type);
      } else {
        if (stack.empty()) {
          return false;
        }
        std::string t = stack[stack.size() - 1];
        stack.pop_back();
        if (t != tree[i].type) {
          return false;
        }
      }
    }
    if (!stack.empty()) {
      return false;
    }
    return true;
  };

  if (!checkTree(newTree) || !checkTree(oldTree)) {
    damage = bounds();
    setVirtualBounds(damage);
    virtual_bounds_valid_ = true;
    return;
  }

  uint8_t* newOpHead = storage_.get();
  uint8_t* newOpTail = newOpHead + byte_count_;
  uint8_t* oldOpHead = dl->storage_.get();
  uint8_t* oldOpTail = dl->storage_.get() + dl->byte_count_;

  // 1.2 算法实现2 从左右分别进行深搜

  // 给定tail返回head
  std::function<int(int,const std::vector<DisplayVirtualLayerInfo>&)> findHeadWithTail = [&] (int tailIndex, const std::vector<DisplayVirtualLayerInfo>& vec) -> int {
    for (int i = tailIndex - 1; i >= 0; i--) {
      if(vec[i].isStart && vec[i].type == vec[tailIndex].type && vec[i].depth == vec[tailIndex].depth) {
        return i;
      }
    }
    return 0;
  };

  // 给定head找到对应tail
  std::function<int(int,const std::vector<DisplayVirtualLayerInfo>&)> findTailWithHead = [&] (int headIndex, const std::vector<DisplayVirtualLayerInfo>& vec) -> int {
    for(unsigned long i = headIndex+1; i < vec.size();i++) {
      if(!vec[i].isStart && vec[i].type == vec[headIndex].type && vec[i].depth == vec[headIndex].depth) {
        return i;
      }
    }
    return 0;
  };

  // 给定Head或者Tail 进行jump.
  std::function<int(int, bool,const std::vector<DisplayVirtualLayerInfo>&)> treeJump = [&] (int index, bool isHead,const std::vector<DisplayVirtualLayerInfo>& vec) -> int {
    if(isHead) {
      for (unsigned long i = index+1; i < vec.size(); i++) {
        if(vec[i].depth == vec[index].depth && vec[i].isStart) {
          return i;
        }
      }
    }else{
      for (int i = index-1; i > 0; i--) {
        if(vec[i].depth == vec[index].depth && !vec[i].isStart) {
          return i;
        }
      }
    }
    return 0;
  };

  // 递归的主函数，将new old 中的子树进行diff，获得差异化部分. 已经保证父节点类型相同.
  std::function<void(int, int, int, int)> diffTree = [&] (int newH, int newT, int oldH, int oldT) -> void {

    // 比较父节点.
    auto h1 = getNPtr(newOpHead, newOpTail, newTree[newH].index);
    auto t1 = getNPtr(newOpHead, newOpTail, newTree[newH+1].index);
    auto h2 = getNPtr(oldOpHead, oldOpTail, oldTree[oldH].index);
    auto t2 = getNPtr(oldOpHead, oldOpTail, oldTree[oldH+1].index);

    if (!(t1 - h1 == t2 - h2 && CompareOps(h1, t1, h2, t2))) {
      // 父节点不一致直接寄.
      auto r1 = partBounds(newTree[newH].index, newTree[newT].index);
      auto r2 = dl->partBounds(oldTree[oldH].index, oldTree[oldT].index);
      damage.join(r1);
      damage.join(r2);
      return;
    }

    // 新树没有子树
    if (newT - newH == 1 && oldT - oldH != 1) {
      auto r1 = dl->partBounds(oldTree[oldH+1].index, oldTree[oldT-1].index);
      damage.join(r1);
      return;
    }

    // 旧树没有子树
    if (oldT - oldH == 1 && oldT - oldH != 1) {
      auto r1 = partBounds(newTree[newH+1].index, newTree[newT-1].index);
      damage.join(r1);
      return;
    }

    if (oldT - oldH == 1 && newT - oldH == 1) {
      // 子节点在这里 return
      return;
    }

    // op一致后比较子树
    int newTreeHead = 0;
    int newTreeTail = 0;
    int oldTreeHead = 0;
    int oldTreeTail = 0;

    // 1. 从左往右递归子树

    // 相同type的最后一个树
    int lEdgeNewTreeHead = 0;
    int lEdgeOldTreeHead = 0;

    while(newTreeTail != newT-1 && oldTreeTail != oldT-1) {
      if(newTreeHead == 0) {
        newTreeHead = newH + 1;
        oldTreeHead = oldH + 1;
      }else{
        newTreeHead = treeJump(newTreeHead, true, newTree);
        oldTreeHead = treeJump(oldTreeHead, true, oldTree);
      }
      newTreeTail = findTailWithHead(newTreeHead, newTree);
      oldTreeTail = findTailWithHead(oldTreeHead, oldTree);
      if(newTree[newTreeHead].type == oldTree[oldTreeHead].type) {
        lEdgeNewTreeHead = newTreeHead;
        lEdgeOldTreeHead = oldTreeHead;
        diffTree(newTreeHead, newTreeTail, oldTreeHead, oldTreeTail);
      }else{
        break;
      }
    }


    // 2. 从右往左递归子树

    int rEdgeNewTreeHead = 0;
    int rEdgeOldTreeHead = 0;

    oldTreeTail = oldT;
    newTreeTail = newT;
    newTreeHead = 0;
    oldTreeHead = 0;

    while(newTreeHead < lEdgeNewTreeHead && oldTreeHead < lEdgeOldTreeHead) {
      if(oldTreeTail == oldT) {
        oldTreeTail = oldT - 1;
        newTreeTail = newT - 1;
      }else{
        oldTreeTail = oldTreeHead - 1;
        newTreeTail = newTreeHead - 1;
      }
      newTreeHead = findHeadWithTail(newTreeTail, newTree);
      oldTreeHead = findHeadWithTail(oldTreeTail, oldTree);
      if(newTreeHead < lEdgeNewTreeHead && oldTreeHead < lEdgeOldTreeHead && newTree[newTreeHead].type == oldTree[oldTreeHead].type) {
        rEdgeNewTreeHead = newTreeHead;
        rEdgeOldTreeHead = oldTreeHead;
        diffTree(newTreeHead, newTreeTail, oldTreeHead, oldTreeTail);
      }else{
        break;
      }
    }

    //

    // 3. 中间的都寄了

    if(rEdgeNewTreeHead - lEdgeNewTreeHead > 1) {
      int garbageNewTreeHead = 0;
      int garbageNewTreeTail = newTreeHead - 1;
      if(lEdgeNewTreeHead == 0) {
        garbageNewTreeHead = 1;
      }else{
        garbageNewTreeHead = findTailWithHead(lEdgeNewTreeHead, newTree) - 1;
      }
      if(rEdgeNewTreeHead == 0) {
        garbageNewTreeTail = newT;
      }else{
        garbageNewTreeTail = rEdgeNewTreeHead - 1;
      }
      SkRect rect = partBounds(newTree[garbageNewTreeHead].index, newTree[garbageNewTreeTail].index);
      damage.join(rect);
    }

    if(rEdgeOldTreeHead - lEdgeNewTreeHead > 1) {
      int garbageOldTreeHead = 0;
      int garbageOldTreeTail = oldTreeHead - 1;
      if(lEdgeOldTreeHead == 0) {
        garbageOldTreeHead = 1;
      }else{
        garbageOldTreeHead = findTailWithHead(lEdgeOldTreeHead, oldTree) - 1;
      }
      if(rEdgeOldTreeHead == 0) {
        garbageOldTreeTail = oldT;
      }else{
        garbageOldTreeTail = rEdgeOldTreeHead - 1;
      }
      SkRect rect = dl->partBounds(oldTree[garbageOldTreeHead].index, oldTree[garbageOldTreeTail].index);
      damage.join(rect);
    }
  };

  if(!newTree.empty() && !oldTree.empty() && newTree[0].type == oldTree[0].type) {
    diffTree(0, newTree.size()-1, 0, oldTree.size()-1);
    // debug.
    auto r1 = bounds();
    auto r2 = dl->bounds();
    r1.join(r2);
    auto percent = (damage.width() * damage.height()) / ( r1.width() * r1.height() );
    printf("DONG DIFF damage: %g %g | %g %g | %g \n", SkScalarToFloat(r1.width()), SkScalarToFloat(r1.height()),
           SkScalarToFloat(damage.width()), SkScalarToFloat(damage.height()), percent);
  }else{
    damage = bounds();
  }

  // 2. Add damage.
  setVirtualBounds(damage);
  virtual_bounds_valid_ = true;
}
}  // namespace flutter
