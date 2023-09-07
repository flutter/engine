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
  DlFRect cull = cull_rect.IsEmpty() ? DlFRect() : cull_rect;
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
      if (!rrect.IsRect()) {
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
  if (cull_rect_.IsEmpty() || content_bounds.IsEmpty()) {
    return true;
  }
  if (!canBeInverted()) {
    return true;
  }
  if (matrix_.HasPerspective()) {
    return false;
  }
  DlFRect mapped;
  mapRect(content_bounds, &mapped);
  return !mapped.Intersects(cull_rect_);
}

void DisplayListMatrixClipTracker::Data::resetBounds(const DlFRect& cull_rect) {
  if (!cull_rect.IsEmpty()) {
    DlFRect rect;
    mapRect(cull_rect, &rect);
    if (!rect.IsEmpty()) {
      cull_rect_ = rect;
      return;
    }
  }
  cull_rect_ = kEmptyRect;
}

void DisplayListMatrixClipTracker::Data::clipBounds(const DlFRect& clip,
                                                    ClipOp op,
                                                    bool is_aa) {
  if (cull_rect_.IsEmpty()) {
    // No point in intersecting further.
    return;
  }
  if (matrix_.HasPerspective()) {
    // We can conservatively ignore this clip.
    return;
  }
  switch (op) {
    case ClipOp::kIntersect: {
      if (clip.IsEmpty()) {
        cull_rect_ = kEmptyRect;
        break;
      }
      DlFRect rect;
      mapRect(clip, &rect);
      if (is_aa) {
        rect = rect.RoundedOut();
      }
      cull_rect_ = cull_rect_.IntersectionOrEmpty(rect);
      break;
    }
    case ClipOp::kDifference: {
      if (clip.IsEmpty()) {
        break;
      }
      DlFRect rect;
      if (mapRect(clip, &rect)) {
        // This technique only works if the transform is rect -> rect
        if (is_aa) {
          rect = rect.RoundedIn();
          if (rect.IsEmpty()) {
            break;
          }
        }
        cull_rect_ = cull_rect_.CutOutOrEmpty(rect);
      }
      break;
    }
  }
}

DlFRect DisplayListMatrixClipTracker::Data::local_cull_rect() const {
  if (cull_rect_.IsEmpty()) {
    return cull_rect_;
  }
  auto inverse = matrix_.Inverse();
  if (!inverse.has_value()) {
    return DlFRect();
  }
  if (matrix_.HasPerspective()) {
    // We could do a 4-point long-form conversion, but since this is
    // only used for culling, let's just return a non-constricting
    // cull rect.
    return kMaxCullRect;
  }
  return inverse->TransformRect(cull_rect_);
}

}  // namespace flutter
