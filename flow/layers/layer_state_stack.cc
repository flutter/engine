// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer_state_stack.h"

namespace flutter {

LayerStateStack::LayerStateStack() {
  state_stack_.emplace_back(SkM44());
}

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
  if (canvas_) {
    canvas_->save();
  }
  if (builder_) {
    builder_->save();
  }
  return ret;
}

LayerStateStack::AutoRestore LayerStateStack::saveWithImageFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlImageFilter> filter) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(std::make_unique<ImageFilterEntry>(bounds, filter));
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

LayerStateStack::AutoRestore LayerStateStack::saveWithColorFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlColorFilter> filter) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(std::make_unique<ColorFilterEntry>(bounds, filter));
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

LayerStateStack::AutoRestore LayerStateStack::saveWithBackdropFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlImageFilter> filter) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(
      std::make_unique<BackdropFilterEntry>(bounds, filter));
  state_stack_.back()->apply(canvas_, builder_);
  return ret;
}

void LayerStateStack::translate(SkScalar tx, SkScalar ty) {
  state_stack_.emplace_back(std::make_unique<TranslateEntry>(tx, ty));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::transform(SkMatrix& matrix) {
  state_stack_.emplace_back(std::make_unique<TransformMatrixEntry>(matrix));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::transform(SkM44& matrix) {
  state_stack_.emplace_back(std::make_unique<TransformM44Entry>(matrix));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::clipRect(const SkRect& rect, SkClipOp op, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRectEntry>(rect, op, is_aa));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::clipRRect(const SkRRect& rrect, SkClipOp op, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRRectEntry>(rrect, op, is_aa));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::clipPath(const SkPath& path, SkClipOp op, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipPathEntry>(path, op, is_aa));
  state_stack_.back()->apply(canvas_, builder_);
}

void LayerStateStack::ImageFilterEntry::apply(
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas) {
    SkPaint paint;
    paint.setImageFilter(filter_->skia_object());
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
    paint.setColorFilter(filter_->skia_object());
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
    sk_sp<SkImageFilter> backdrop_filter = filter_->skia_object();
    canvas->saveLayer(SkCanvas::SaveLayerRec{save_bounds(), nullptr,
                                             backdrop_filter.get(), 0});
  }
  if (builder) {
    builder->saveLayer(save_bounds(), nullptr, filter_.get());
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
    canvas->clipRect(rect_, clip_op_, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipRect(rect_, clip_op_, is_aa_);
  }
}

void LayerStateStack::ClipRRectEntry::apply(SkCanvas* canvas,
                                            DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->clipRRect(rrect_, clip_op_, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipRRect(rrect_, clip_op_, is_aa_);
  }
}

void LayerStateStack::ClipPathEntry::apply(SkCanvas* canvas,
                                           DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->clipPath(path_, clip_op_, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipPath(path_, clip_op_, is_aa_);
  }
}

}  // namespace flutter
