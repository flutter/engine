// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_MATRIX_CLIP_TRACKER_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_MATRIX_CLIP_TRACKER_H_

#include <vector>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/dl_canvas.h"
#include "flutter/display_list/geometry/dl_rect.h"
#include "flutter/display_list/geometry/dl_round_rect.h"
#include "flutter/display_list/geometry/dl_transform.h"
#include "flutter/fml/logging.h"

namespace flutter {

class DisplayListMatrixClipTracker {
 private:
  using ClipOp = DlCanvas::ClipOp;

 public:
  DisplayListMatrixClipTracker(const DlFRect& cull_rect,
                               const DlTransform& matrix);

  // This method should almost never be used as it breaks the encapsulation
  // of the enclosing clips. However it is needed for practical purposes in
  // some rare cases - such as when a saveLayer is collecting rendering
  // operations prior to applying a filter on the entire layer bounds and
  // some of those operations fall outside the enclosing clip, but their
  // filtered content will spread out from where they were rendered on the
  // layer into the enclosing clipped area.
  // Omitting the |cull_rect| argument, or passing nullptr, will restore the
  // cull rect to the initial value it had when the tracker was constructed.
  void resetCullRect(const DlFRect* cull_rect = nullptr) {
    current_->resetBounds(cull_rect ? *cull_rect : original_cull_rect_);
  }

  DlFRect base_device_cull_rect() const {
    return saved_[0]->device_cull_rect();
  }

  DlTransform matrix() const { return current_->matrix(); }
  DlFRect local_cull_rect() const { return current_->local_cull_rect(); }
  DlFRect device_cull_rect() const { return current_->device_cull_rect(); }
  bool content_culled(const DlFRect& content_bounds) const {
    return current_->content_culled(content_bounds);
  }
  bool is_cull_rect_empty() const { return current_->is_cull_rect_empty(); }

  void save();
  void restore();
  void reset();
  int getSaveCount() {
    // saved_[0] is always the untouched initial conditions
    // saved_[1] is the first editable stack entry
    return saved_.size() - 1;
  }
  void restoreToCount(int restore_count);

  void translate(DlScalar tx, DlScalar ty) { current_->translate(tx, ty); }
  void scale(DlScalar sx, DlScalar sy) { current_->scale(sx, sy); }
  void skew(DlScalar skx, DlScalar sky) { current_->skew(skx, sky); }
  void rotate(DlAngle angle) { current_->rotate(angle); }
  void transform(const DlTransform& matrix) { current_->transform(matrix); }
  // clang-format off
  void transform2DAffine(
      DlScalar mxx, DlScalar mxy, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myt);
  void transformFullPerspective(
      DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
      DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
      DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt);
  // clang-format on
  void setTransform(const DlTransform& matrix) {
    current_->setTransform(matrix);
  }
  void setIdentity() { current_->setIdentity(); }
  void setIntegerTranslation() {
    current_->setTransform(current_->matrix().WithIntegerTranslation());
  }

  bool mapRect(DlFRect* rect) const { return current_->mapRect(*rect, rect); }

  void clipRect(const DlFRect& rect, ClipOp op, bool is_aa) {
    current_->clipBounds(rect, op, is_aa);
  }
  void clipRRect(const DlFRRect& rrect, ClipOp op, bool is_aa);
  void clipPath(const DlPath& path, ClipOp op, bool is_aa);

 private:
  class Data {
   public:
    Data(const DlTransform& matrix, const DlFRect& rect)
        : cull_rect_(rect), matrix_(matrix) {}
    explicit Data(const Data* copy)
        : cull_rect_(copy->device_cull_rect()), matrix_(copy->matrix()) {}

    ~Data() = default;

    DlTransform matrix() const { return matrix_; }

    DlFRect device_cull_rect() const { return cull_rect_; }
    DlFRect local_cull_rect() const;
    bool content_culled(const DlFRect& content_bounds) const;
    bool is_cull_rect_empty() const { return cull_rect_.IsEmpty(); }

    void translate(DlScalar tx, DlScalar ty) { matrix_.TranslateInner(tx, ty); }
    void scale(DlScalar sx, DlScalar sy) { matrix_.ScaleInner(sx, sy); }
    void skew(DlScalar skx, DlScalar sky) { matrix_.SkewInner(skx, sky); }
    void rotate(DlAngle angle) { matrix_.RotateInner(angle); }
    void transform(const DlTransform& matrix) { matrix_.ConcatInner(matrix); }
    void setTransform(const DlTransform& matrix) { matrix_ = matrix; }
    void setIdentity() { matrix_.SetIdentity(); }
    bool mapRect(const DlFRect& rect, DlFRect* mapped) const {
      *mapped = matrix_.TransformRect(rect);
      return matrix_.RectStaysRect();
    }
    bool canBeInverted() const { return matrix_.IsInvertible(); }

    void clipBounds(const DlFRect& clip, ClipOp op, bool is_aa);

    void resetBounds(const DlFRect& cull_rect);

   protected:
    Data(const DlFRect& rect, const DlTransform& matrix)
        : cull_rect_(rect), matrix_(matrix) {}

    DlFRect cull_rect_;
    DlTransform matrix_;
  };

  DlFRect original_cull_rect_;
  Data* current_;
  std::vector<std::unique_ptr<Data>> saved_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_MATRIX_CLIP_TRACKER_H_
