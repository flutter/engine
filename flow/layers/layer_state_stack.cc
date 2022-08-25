// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer_state_stack.h"
#include "flutter/flow/paint_utils.h"

namespace flutter {

void LayerStateStack::setCanvasDelegate(SkCanvas* canvas) {
  if (canvas_) {
    canvas_->restoreToCount(canvas_restore_count_);
    canvas_ = nullptr;
  }
  if (canvas) {
    canvas_restore_count_ = canvas->getSaveCount();
    canvas_ = canvas;
    for (auto& state : state_stack_) {
      if (!state->is_backdrop_filter()) {
        // BackdropFilter is only applied on the canvas that
        // was present at the time that the backdrop layer was
        // first encountered to avoid applying it on every
        // platform view embedder overlay.
        state->apply(canvas, nullptr);
      }
    }
  }
}

void LayerStateStack::setBuilderDelegate(DisplayListBuilder* builder) {
  if (builder_) {
    builder_->restoreToCount(builder_restore_count_);
    builder_ = nullptr;
  }
  if (builder) {
    builder_restore_count_ = builder->getSaveCount();
    builder_ = builder;
    for (auto& state : state_stack_) {
      if (!state->is_backdrop_filter()) {
        // BackdropFilter is only applied on the builder that
        // was present at the time that the backdrop layer was
        // first encountered to avoid applying it on every
        // platform view embedder overlay.
        state->apply(nullptr, builder);
      }
    }
  }
}

LayerStateStack::AutoRestore::AutoRestore(LayerStateStack* stack)
    : stack_(stack), stack_restore_count_(stack->getStackCount()) {}

LayerStateStack::AutoRestore::~AutoRestore() {
  stack_->restoreToCount(stack_restore_count_);
}

LayerStateStack::AutoRestore LayerStateStack::save() {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(std::make_unique<SaveEntry>());
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

LayerStateStack::AutoRestore LayerStateStack::saveLayer(const SkRect* bounds,
                                                        bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(
      std::make_unique<SaveLayerEntry>(bounds, checkerboard));
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

LayerStateStack::AutoRestore LayerStateStack::saveWithOpacity(
    const SkRect* bounds,
    SkScalar opacity,
    bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  if (opacity < SK_Scalar1) {
    state_stack_.emplace_back(
        std::make_unique<OpacityEntry>(bounds, opacity, checkerboard));
  } else {
    state_stack_.emplace_back(
        std::make_unique<SaveLayerEntry>(bounds, checkerboard));
  }
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

LayerStateStack::AutoRestore LayerStateStack::saveWithImageFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlImageFilter> filter,
    bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(
      std::make_unique<ImageFilterEntry>(bounds, filter, checkerboard));
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

LayerStateStack::AutoRestore LayerStateStack::saveWithColorFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlColorFilter> filter,
    bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(
      std::make_unique<ColorFilterEntry>(bounds, filter, checkerboard));
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

LayerStateStack::AutoRestore LayerStateStack::saveWithBackdropFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlImageFilter> filter,
    DlBlendMode blend_mode,
    bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(std::make_unique<BackdropFilterEntry>(
      bounds, filter, blend_mode, checkerboard));
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

void LayerStateStack::translate(SkScalar tx, SkScalar ty) {
  state_stack_.emplace_back(std::make_unique<TranslateEntry>(tx, ty));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::transform(const SkMatrix& matrix) {
  state_stack_.emplace_back(std::make_unique<TransformMatrixEntry>(matrix));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::transform(const SkM44& matrix) {
  state_stack_.emplace_back(std::make_unique<TransformM44Entry>(matrix));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::clipRect(const SkRect& rect, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRectEntry>(rect, is_aa));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::clipRRect(const SkRRect& rrect, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRRectEntry>(rrect, is_aa));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::clipPath(const SkPath& path, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipPathEntry>(path, is_aa));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::restoreToCount(size_t restore_count) {
  while (state_stack_.size() > restore_count) {
    state_stack_.back()->restore(canvas_, builder_);
    state_stack_.pop_back();
  }
}

void LayerStateStack::SaveEntry::apply(SkCanvas* canvas,
                                       DisplayListBuilder* builder) const {
  if (canvas) {
    canvas->save();
  }
  if (builder) {
    builder->save();
  }
}

void LayerStateStack::SaveEntry::restore(SkCanvas* canvas,
                                         DisplayListBuilder* builder) const {
  if (canvas) {
    canvas->restore();
  }
  if (builder) {
    builder->restore();
  }
  do_checkerboard(canvas, builder);
}

void LayerStateStack::SaveLayerEntry::apply(SkCanvas* canvas,
                                            DisplayListBuilder* builder) const {
  if (canvas) {
    canvas->saveLayer(save_bounds(), nullptr);
  }
  if (builder) {
    builder->saveLayer(save_bounds(), nullptr);
  }
}

void LayerStateStack::SaveLayerEntry::do_checkerboard(
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (checkerboard_ && has_bounds_) {
    if (canvas) {
      SkDrawCheckerboard(canvas, bounds_);
    }
    if (builder) {
      DlDrawCheckerboard(builder, bounds_);
    }
  }
}

void LayerStateStack::OpacityEntry::apply(SkCanvas* canvas,
                                          DisplayListBuilder* builder) const {
  if (canvas) {
    SkPaint paint;
    paint.setAlphaf(opacity_);
    canvas->saveLayer(save_bounds(), &paint);
  }
  if (builder) {
    DlPaint paint;
    paint.setAlpha(SkScalarRoundToInt(opacity_ * 255));
    builder->saveLayer(save_bounds(), &paint);
  }
}

void LayerStateStack::ImageFilterEntry::apply(
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas) {
    SkPaint paint;
    paint.setImageFilter(filter_ ? filter_->skia_object() : nullptr);
    canvas->saveLayer(save_bounds(), &paint);
  }
  if (builder) {
    DlPaint paint;
    paint.setImageFilter(filter_);
    builder->saveLayer(save_bounds(), &paint);
  }
}

void LayerStateStack::ColorFilterEntry::apply(
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas) {
    SkPaint paint;
    paint.setColorFilter(filter_ ? filter_->skia_object() : nullptr);
    canvas->saveLayer(save_bounds(), &paint);
  }
  if (builder) {
    DlPaint paint;
    paint.setColorFilter(filter_);
    builder->saveLayer(save_bounds(), &paint);
  }
}

void LayerStateStack::BackdropFilterEntry::apply(
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas) {
    sk_sp<SkImageFilter> backdrop_filter =
        filter_ ? filter_->skia_object() : nullptr;
    if (blend_mode_ != DlBlendMode::kSrcOver) {
      SkPaint paint;
      paint.setBlendMode(ToSk(blend_mode_));
      canvas->saveLayer(SkCanvas::SaveLayerRec{save_bounds(), &paint,
                                               backdrop_filter.get(), 0});
    } else {
      canvas->saveLayer(SkCanvas::SaveLayerRec{save_bounds(), nullptr,
                                               backdrop_filter.get(), 0});
    }
  }
  if (builder) {
    if (blend_mode_ != DlBlendMode::kSrcOver) {
      DlPaint paint;
      paint.setBlendMode(blend_mode_);
      builder->saveLayer(save_bounds(), &paint, filter_.get());
    } else {
      builder->saveLayer(save_bounds(), nullptr, filter_.get());
    }
  }
}

void LayerStateStack::TranslateEntry::apply(SkCanvas* canvas,
                                            DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->translate(tx_, ty_);
  }
  if (builder != nullptr) {
    builder->translate(tx_, ty_);
  }
}

void LayerStateStack::TransformMatrixEntry::apply(
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->concat(matrix_);
  }
  if (builder != nullptr) {
    builder->transform(matrix_);
  }
}

void LayerStateStack::TransformM44Entry::apply(
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->concat(m44_);
  }
  if (builder != nullptr) {
    builder->transform(m44_);
  }
}

void LayerStateStack::ClipRectEntry::apply(SkCanvas* canvas,
                                           DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->clipRect(rect_, SkClipOp::kIntersect, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipRect(rect_, SkClipOp::kIntersect, is_aa_);
  }
}

void LayerStateStack::ClipRRectEntry::apply(SkCanvas* canvas,
                                            DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->clipRRect(rrect_, SkClipOp::kIntersect, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipRRect(rrect_, SkClipOp::kIntersect, is_aa_);
  }
}

void LayerStateStack::ClipPathEntry::apply(SkCanvas* canvas,
                                           DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->clipPath(path_, SkClipOp::kIntersect, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipPath(path_, SkClipOp::kIntersect, is_aa_);
  }
}

}  // namespace flutter
