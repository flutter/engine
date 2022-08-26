// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer_state_stack.h"
#include "flutter/flow/paint_utils.h"

namespace flutter {

using AutoRestore = LayerStateStack::AutoRestore;
using RenderingAttributes = LayerStateStack::RenderingAttributes;

void LayerStateStack::setCanvasDelegate(SkCanvas* canvas) {
  if (canvas_) {
    canvas_->restoreToCount(canvas_restore_count_);
    canvas_ = nullptr;
  }
  if (canvas) {
    canvas_restore_count_ = canvas->getSaveCount();
    canvas_ = canvas;
    for (auto& state : state_stack_) {
      state->reapply(&outstanding_, canvas, nullptr);
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
      state->reapply(&outstanding_, nullptr, builder);
    }
  }
}

AutoRestore::AutoRestore(LayerStateStack* stack)
    : stack_(stack), stack_restore_count_(stack->getStackCount()) {}

AutoRestore::~AutoRestore() {
  stack_->restoreToCount(stack_restore_count_);
}

const RenderingAttributes LayerStateStack::applyOutstandingState(
    int can_apply_flags) {
  SkScalar cur_opacity = outstanding_.opacity;
  if (cur_opacity >= SK_Scalar1 || (can_apply_flags & CAN_APPLY_OPACITY) != 0) {
    return {cur_opacity};
  } else {
    resolve(nullptr, &outstanding_, canvas_, builder_);
    outstanding_.opacity = SK_Scalar1;
    return {SK_Scalar1};
  }
}

AutoRestore LayerStateStack::save() {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(std::make_unique<SaveEntry>());
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
  return ret;
}

AutoRestore LayerStateStack::saveLayer(const SkRect* bounds,
                                       bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(
      std::make_unique<SaveLayerEntry>(bounds, checkerboard));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
  return ret;
}

AutoRestore LayerStateStack::saveWithOpacity(const SkRect* bounds,
                                             SkScalar opacity,
                                             bool checkerboard) {
  auto ret = AutoRestore(this);
  if (opacity < SK_Scalar1) {
    state_stack_.emplace_back(std::make_unique<OpacityEntry>(
        bounds, outstanding_.opacity, opacity, checkerboard));
  } else {
    state_stack_.emplace_back(
        std::make_unique<SaveLayerEntry>(bounds, checkerboard));
  }
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
  return ret;
}

AutoRestore LayerStateStack::saveWithImageFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlImageFilter> filter,
    bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(
      std::make_unique<ImageFilterEntry>(bounds, filter, checkerboard));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
  return ret;
}

AutoRestore LayerStateStack::saveWithColorFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlColorFilter> filter,
    bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(
      std::make_unique<ColorFilterEntry>(bounds, filter, checkerboard));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
  return ret;
}

AutoRestore LayerStateStack::saveWithBackdropFilter(
    const SkRect* bounds,
    const std::shared_ptr<const DlImageFilter> filter,
    DlBlendMode blend_mode,
    bool checkerboard) {
  auto ret = LayerStateStack::AutoRestore(this);
  state_stack_.emplace_back(std::make_unique<BackdropFilterEntry>(
      bounds, filter, blend_mode, checkerboard));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
  return ret;
}

void LayerStateStack::translate(SkScalar tx, SkScalar ty) {
  state_stack_.emplace_back(std::make_unique<TranslateEntry>(tx, ty));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
}

void LayerStateStack::transform(const SkMatrix& matrix) {
  state_stack_.emplace_back(std::make_unique<TransformMatrixEntry>(matrix));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
}

void LayerStateStack::transform(const SkM44& matrix) {
  state_stack_.emplace_back(std::make_unique<TransformM44Entry>(matrix));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
}

void LayerStateStack::clipRect(const SkRect& rect, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRectEntry>(rect, is_aa));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
}

void LayerStateStack::clipRRect(const SkRRect& rrect, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRRectEntry>(rrect, is_aa));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
}

void LayerStateStack::clipPath(const SkPath& path, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipPathEntry>(path, is_aa));
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
}

void LayerStateStack::restoreToCount(size_t restore_count) {
  while (state_stack_.size() > restore_count) {
    state_stack_.back()->restore(&outstanding_, canvas_, builder_);
    state_stack_.pop_back();
  }
}

void LayerStateStack::SaveEntry::apply(RenderingAttributes* attributes,
                                       SkCanvas* canvas,
                                       DisplayListBuilder* builder) const {
  if (canvas) {
    canvas->save();
  }
  if (builder) {
    builder->save();
  }
}

void LayerStateStack::SaveEntry::restore(RenderingAttributes* attributes,
                                         SkCanvas* canvas,
                                         DisplayListBuilder* builder) const {
  if (canvas) {
    canvas->restore();
  }
  if (builder) {
    builder->restore();
  }
  do_checkerboard(canvas, builder);
}

void LayerStateStack::SaveLayerEntry::apply(RenderingAttributes* attributes,
                                            SkCanvas* canvas,
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

void LayerStateStack::OpacityEntry::apply(RenderingAttributes* attributes,
                                          SkCanvas* canvas,
                                          DisplayListBuilder* builder) const {
  attributes->opacity *= opacity_;
}

void LayerStateStack::OpacityEntry::restore(RenderingAttributes* attributes,
                                            SkCanvas* canvas,
                                            DisplayListBuilder* builder) const {
  attributes->opacity = outstanding_opacity_;
}

void LayerStateStack::resolve(const SkRect* bounds,
                              RenderingAttributes* attributes,
                              SkCanvas* canvas,
                              DisplayListBuilder* builder) {
  if (canvas) {
    SkPaint paint;
    paint.setAlphaf(attributes->opacity);
    canvas->saveLayer(bounds, &paint);
  }
  if (builder) {
    DlPaint paint;
    paint.setAlpha(SkScalarRoundToInt(attributes->opacity * 255));
    builder->saveLayer(bounds, &paint);
  }
}

void LayerStateStack::ImageFilterEntry::apply(
    RenderingAttributes* attributes,
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
    RenderingAttributes* attributes,
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
    RenderingAttributes* attributes,
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas) {
    sk_sp<SkImageFilter> backdrop_filter =
        filter_ ? filter_->skia_object() : nullptr;
    SkPaint paint;
    paint.setBlendMode(ToSk(blend_mode_));
    canvas->saveLayer(SkCanvas::SaveLayerRec{save_bounds(), &paint,
                                             backdrop_filter.get(), 0});
  }
  if (builder) {
    DlPaint paint;
    paint.setBlendMode(blend_mode_);
    builder->saveLayer(save_bounds(), &paint, filter_.get());
  }
}

void LayerStateStack::BackdropFilterEntry::reapply(
    RenderingAttributes* attributes,
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  // On the reapply for subsequent overlay layers, we do not
  // want to reapply the backdrop filter, but we do need to
  // do a saveLayer to encapsulate the contents and match the
  // restore that will be forthcoming. Note that this is not
  // perfect if the BlendMode is not associative as we will be
  // compositing multiple parts of the content in batches.
  // Luckily the most common SrcOver is associative.
  if (canvas) {
    SkPaint paint;
    paint.setBlendMode(ToSk(blend_mode_));
    canvas->saveLayer(save_bounds(), &paint);
  }
  if (builder) {
    DlPaint paint;
    paint.setBlendMode(blend_mode_);
    builder->saveLayer(save_bounds(), &paint);
  }
}

void LayerStateStack::TranslateEntry::apply(RenderingAttributes* attributes,
                                            SkCanvas* canvas,
                                            DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->translate(tx_, ty_);
  }
  if (builder != nullptr) {
    builder->translate(tx_, ty_);
  }
}

void LayerStateStack::TransformMatrixEntry::apply(
    RenderingAttributes* attributes,
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
    RenderingAttributes* attributes,
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->concat(m44_);
  }
  if (builder != nullptr) {
    builder->transform(m44_);
  }
}

void LayerStateStack::ClipRectEntry::apply(RenderingAttributes* attributes,
                                           SkCanvas* canvas,
                                           DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->clipRect(rect_, SkClipOp::kIntersect, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipRect(rect_, SkClipOp::kIntersect, is_aa_);
  }
}

void LayerStateStack::ClipRRectEntry::apply(RenderingAttributes* attributes,
                                            SkCanvas* canvas,
                                            DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->clipRRect(rrect_, SkClipOp::kIntersect, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipRRect(rrect_, SkClipOp::kIntersect, is_aa_);
  }
}

void LayerStateStack::ClipPathEntry::apply(RenderingAttributes* attributes,
                                           SkCanvas* canvas,
                                           DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    canvas->clipPath(path_, SkClipOp::kIntersect, is_aa_);
  }
  if (builder != nullptr) {
    builder->clipPath(path_, SkClipOp::kIntersect, is_aa_);
  }
}

}  // namespace flutter
