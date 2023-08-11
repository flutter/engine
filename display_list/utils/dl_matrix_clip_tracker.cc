// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/utils/dl_matrix_clip_tracker.h"

#include "flutter/display_list/dl_builder.h"
#include "flutter/fml/logging.h"

namespace flutter {

DisplayListMatrixClipTracker::DisplayListMatrixClipTracker(
    const DlFRect& cull_rect,
    const DlTransform& matrix)
    : original_cull_rect_(cull_rect) {
  // isEmpty protects us against NaN as we normalize any empty cull rects
  DlFRect cull = cull_rect.is_empty() ? DlFRect() : cull_rect;
  saved_.emplace_back(std::make_unique<Data>(matrix, cull));
  current_ = saved_.back().get();
  save();  // saved_[0] will always be the initial settings
}

// clang-format off
void DisplayListMatrixClipTracker::transform2DAffine(
    DlScalar mxx, DlScalar mxy, DlScalar mxt,
    DlScalar myx, DlScalar myy, DlScalar myt) {
  current_->transform(DlTransform::MakeAffine2D(mxx, mxy, mxt,
                                                myx, myy, myt));
}
void DisplayListMatrixClipTracker::transformFullPerspective(
    DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
    DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
    DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
    DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) {
  transform(DlTransform::MakeRowMajor(mxx, mxy, mxz, mxt,  //
                                      myx, myy, myz, myt,  //
                                      mzx, mzy, mzz, mzt,  //
                                      mwx, mwy, mwz, mwt));
}
// clang-format on

void DisplayListMatrixClipTracker::save() {
  saved_.emplace_back(std::make_unique<Data>(current_));
  current_ = saved_.back().get();
}

void DisplayListMatrixClipTracker::restore() {
  if (saved_.size() > 2) {
    saved_.pop_back();
    current_ = saved_.back().get();
  }
}

void DisplayListMatrixClipTracker::reset() {
  while (saved_.size() > 1) {
    saved_.pop_back();
    current_ = saved_.back().get();
  }
  save();  // saved_[0] will always be the initial settings
}

void DisplayListMatrixClipTracker::restoreToCount(int restore_count) {
  FML_DCHECK(restore_count <= getSaveCount());
  if (restore_count < 1) {
    restore_count = 1;
  }
  while (restore_count < getSaveCount()) {
    restore();
  }
}

void DisplayListMatrixClipTracker::clipRRect(const DlFRRect& rrect,
                                             ClipOp op,
                                             bool is_aa) {
  switch (op) {
    case ClipOp::kIntersect:
      break;
    case ClipOp::kDifference:
      if (!rrect.is_rect()) {
        return;
      }
      break;
  }
  current_->clipBounds(rrect.Bounds(), op, is_aa);
}
void DisplayListMatrixClipTracker::clipPath(const DlPath& path,
                                            ClipOp op,
                                            bool is_aa) {
  // Map "kDifference of inverse path" to "kIntersect of the original path" and
  // map "kIntersect of inverse path" to "kDifference of the original path"
  if (path.is_inverse_fill_type()) {
    switch (op) {
      case ClipOp::kIntersect:
        op = ClipOp::kDifference;
        break;
      case ClipOp::kDifference:
        op = ClipOp::kIntersect;
        break;
    }
  }

  DlFRect bounds;
  switch (op) {
    case ClipOp::kIntersect:
      bounds = path.Bounds();
      break;
    case ClipOp::kDifference:
      if (!path.is_rect(&bounds)) {
        return;
      }
      break;
  }
  current_->clipBounds(bounds, op, is_aa);
}

bool DisplayListMatrixClipTracker::Data::content_culled(
    const DlFRect& content_bounds) const {
  if (cull_rect_.is_empty() || content_bounds.is_empty()) {
    return true;
  }
  if (!canBeInverted()) {
    return true;
  }
  if (matrix_.has_perspective()) {
    return false;
  }
  DlFRect mapped;
  mapRect(content_bounds, &mapped);
  return !mapped.Intersects(cull_rect_);
}

void DisplayListMatrixClipTracker::Data::resetBounds(const DlFRect& cull_rect) {
  if (!cull_rect.is_empty()) {
    DlFRect rect;
    mapRect(cull_rect, &rect);
    if (!rect.is_empty()) {
      cull_rect_ = rect;
      return;
    }
  }
  cull_rect_.SetEmpty();
}

void DisplayListMatrixClipTracker::Data::clipBounds(const DlFRect& clip,
                                                    ClipOp op,
                                                    bool is_aa) {
  if (cull_rect_.is_empty()) {
    // No point in intersecting further.
    return;
  }
  if (matrix_.has_perspective()) {
    // We can conservatively ignore this clip.
    return;
  }
  switch (op) {
    case ClipOp::kIntersect: {
      if (clip.is_empty()) {
        cull_rect_.SetEmpty();
        break;
      }
      DlFRect rect;
      mapRect(clip, &rect);
      if (is_aa) {
        rect.RoundOut();
      }
      if (!cull_rect_.Intersect(rect)) {
        cull_rect_.SetEmpty();
      }
      break;
    }
    case ClipOp::kDifference: {
      if (clip.is_empty()) {
        break;
      }
      DlFRect rect;
      if (mapRect(clip, &rect)) {
        // This technique only works if the transform is rect -> rect
        if (is_aa) {
          rect.RoundIn();
          if (rect.is_empty()) {
            break;
          }
        }
        if (!rect.Intersects(cull_rect_)) {
          break;
        }
        if (rect.left() <= cull_rect_.left() &&
            rect.right() >= cull_rect_.right()) {
          // bounds spans entire width of cull_rect_
          // therefore we can slice off a top or bottom
          // edge of the cull_rect_.
          DlScalar top = cull_rect_.top();
          DlScalar btm = cull_rect_.bottom();
          if (rect.top() <= top) {
            top = rect.bottom();
          }
          if (rect.bottom() >= btm) {
            btm = rect.top();
          }
          if (top < btm) {
            cull_rect_.SetTop(top);
            cull_rect_.SetBottom(btm);
          } else {
            cull_rect_.SetEmpty();
          }
        } else if (rect.top() <= cull_rect_.top() &&
                   rect.bottom() >= cull_rect_.bottom()) {
          // bounds spans entire height of cull_rect_
          // therefore we can slice off a left or right
          // edge of the cull_rect_.
          DlScalar lft = cull_rect_.left();
          DlScalar rgt = cull_rect_.right();
          if (rect.left() <= lft) {
            lft = rect.right();
          }
          if (rect.right() >= rgt) {
            rgt = rect.left();
          }
          if (lft < rgt) {
            cull_rect_.SetLeft(lft);
            cull_rect_.SetRight(rgt);
          } else {
            cull_rect_.SetEmpty();
          }
        }
      }
      break;
    }
  }
}

DlFRect DisplayListMatrixClipTracker::Data::local_cull_rect() const {
  if (cull_rect_.is_empty()) {
    return cull_rect_;
  }
  DlTransform inverse;
  if (!matrix_.Invert(&inverse)) {
    return DlFRect();
  }
  if (matrix_.has_perspective()) {
    // We could do a 4-point long-form conversion, but since this is
    // only used for culling, let's just return a non-constricting
    // cull rect.
    return kMaxCullRect;
  }
  return inverse.TransformRect(cull_rect_);
}

}  // namespace flutter
