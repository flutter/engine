// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer_state_stack.h"
#include "flutter/flow/paint_utils.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

using AutoRestore = LayerStateStack::AutoRestore;
using MutatorContext = LayerStateStack::MutatorContext;

void LayerStateStack::set_canvas_delegate(SkCanvas* canvas) {
  if (canvas_) {
    canvas_->restoreToCount(canvas_restore_count_);
    canvas_ = nullptr;
  }
  if (canvas) {
    canvas_restore_count_ = canvas->getSaveCount();
    canvas_ = canvas;
    reapply_all(canvas, nullptr);
  }
}

void LayerStateStack::set_builder_delegate(DisplayListBuilder* builder) {
  if (builder_) {
    builder_->restoreToCount(builder_restore_count_);
    builder_ = nullptr;
  }
  if (builder) {
    builder_restore_count_ = builder->getSaveCount();
    builder_ = builder;
    reapply_all(nullptr, builder);
  }
}

void LayerStateStack::reapply_all(SkCanvas* canvas,
                                  DisplayListBuilder* builder) {
  // We use a local RenderingAttributes instance so that it can track the
  // necessary state changes independently as they occur in the stack.
  // Reusing |outstanding_| would wreak havoc on the current state of
  // the stack. When we are finished, though, the local attributes
  // contents should match the current outstanding_ values;
  RenderingAttributes attributes;
  for (auto& state : state_stack_) {
    state->reapply(&attributes, canvas, builder);
  }
  FML_DCHECK(attributes == outstanding_);
}

AutoRestore::AutoRestore(LayerStateStack* stack)
    : layer_state_stack_(stack), stack_restore_count_(stack->stack_count()) {}

AutoRestore::~AutoRestore() {
  layer_state_stack_->restore_to_count(stack_restore_count_);
}

AutoRestore LayerStateStack::applyState(const SkRect& bounds,
                                        int can_apply_flags) {
  auto ret = AutoRestore(this);
  if (needs_save_layer(can_apply_flags)) {
    save_layer(bounds);
  }
  return ret;
}

SkPaint* LayerStateStack::RenderingAttributes::fill(SkPaint& paint,
                                                    DlBlendMode mode) {
  SkPaint* ret = nullptr;
  if (opacity < SK_Scalar1) {
    paint.setAlphaf(std::max(opacity, 0.0f));
    ret = &paint;
  } else {
    paint.setAlphaf(SK_Scalar1);
  }
  if (color_filter) {
    paint.setColorFilter(color_filter->skia_object());
    ret = &paint;
  } else {
    paint.setColorFilter(nullptr);
  }
  if (image_filter) {
    paint.setImageFilter(image_filter->skia_object());
    ret = &paint;
  } else {
    paint.setImageFilter(nullptr);
  }
  paint.setBlendMode(ToSk(mode));
  if (mode != DlBlendMode::kSrcOver) {
    ret = &paint;
  }
  return ret;
}

DlPaint* LayerStateStack::RenderingAttributes::fill(DlPaint& paint,
                                                    DlBlendMode mode) {
  DlPaint* ret = nullptr;
  if (opacity < SK_Scalar1) {
    paint.setOpacity(std::max(opacity, 0.0f));
    ret = &paint;
  } else {
    paint.setOpacity(SK_Scalar1);
  }
  paint.setColorFilter(color_filter);
  if (color_filter) {
    ret = &paint;
  }
  paint.setImageFilter(image_filter);
  if (image_filter) {
    ret = &paint;
  }
  paint.setBlendMode(mode);
  if (mode != DlBlendMode::kSrcOver) {
    ret = &paint;
  }
  return ret;
}

MutatorContext LayerStateStack::save() {
  auto ret = MutatorContext(this);
  state_stack_.emplace_back(std::make_unique<SaveEntry>());
  state_stack_.back()->apply(&outstanding_, canvas_, builder_);
  return ret;
}

void MutatorContext::saveLayer(const SkRect& bounds) {
  layer_state_stack_->save_layer(bounds);
}

void MutatorContext::applyOpacity(const SkRect& bounds, SkScalar opacity) {
  if (opacity < SK_Scalar1) {
    layer_state_stack_->push_attributes();
    layer_state_stack_->maybe_save_layer(opacity);
    layer_state_stack_->push_opacity(bounds, opacity);
  }
}

void MutatorContext::applyImageFilter(
    const SkRect& bounds,
    const std::shared_ptr<const DlImageFilter> filter) {
  if (filter) {
    layer_state_stack_->push_attributes();
    layer_state_stack_->maybe_save_layer(filter);
    layer_state_stack_->push_image_filter(bounds, filter);
  }
}

void MutatorContext::applyColorFilter(
    const SkRect& bounds,
    const std::shared_ptr<const DlColorFilter> filter) {
  if (filter) {
    layer_state_stack_->push_attributes();
    layer_state_stack_->maybe_save_layer(filter);
    layer_state_stack_->push_color_filter(bounds, filter);
  }
}

void MutatorContext::applyBackdropFilter(
    const SkRect& bounds,
    const std::shared_ptr<const DlImageFilter> filter,
    DlBlendMode blend_mode) {
  layer_state_stack_->push_backdrop(bounds, filter, blend_mode);
}

void MutatorContext::translate(SkScalar tx, SkScalar ty) {
  if (!(tx == 0 && ty == 0)) {
    layer_state_stack_->maybe_save_layer_for_transform();
    layer_state_stack_->push_translate(tx, ty);
  }
}

void MutatorContext::transform(const SkMatrix& matrix) {
  if (matrix.isTranslate()) {
    translate(matrix.getTranslateX(), matrix.getTranslateY());
  } else if (!matrix.isIdentity()) {
    layer_state_stack_->maybe_save_layer_for_transform();
    layer_state_stack_->push_transform(matrix);
  }
}

void MutatorContext::transform(const SkM44& m44) {
  layer_state_stack_->maybe_save_layer_for_transform();
  layer_state_stack_->push_transform(m44);
}

void MutatorContext::integralTransform() {
  layer_state_stack_->maybe_save_layer_for_transform();
  layer_state_stack_->push_integral_transform();
}

void MutatorContext::clipRect(const SkRect& rect, bool is_aa) {
  layer_state_stack_->maybe_save_layer_for_clip();
  layer_state_stack_->push_clip_rect(rect, is_aa);
}

void MutatorContext::clipRRect(const SkRRect& rrect, bool is_aa) {
  layer_state_stack_->maybe_save_layer_for_clip();
  layer_state_stack_->push_clip_rrect(rrect, is_aa);
}

void MutatorContext::clipPath(const SkPath& path, bool is_aa) {
  layer_state_stack_->maybe_save_layer_for_clip();
  layer_state_stack_->push_clip_path(path, is_aa);
}

void LayerStateStack::restore_to_count(size_t restore_count) {
  while (state_stack_.size() > restore_count) {
    state_stack_.back()->restore(&outstanding_, canvas_, builder_);
    state_stack_.pop_back();
  }
}

void LayerStateStack::push_attributes() {
  state_stack_.emplace_back(std::make_unique<AttributesEntry>(outstanding_));
}

void LayerStateStack::push_opacity(const SkRect& bounds, SkScalar opacity) {
  state_stack_.emplace_back(std::make_unique<OpacityEntry>(bounds, opacity));
  apply_last_entry();
}

void LayerStateStack::push_color_filter(
    const SkRect& bounds,
    const std::shared_ptr<const DlColorFilter> filter) {
  state_stack_.emplace_back(std::make_unique<ColorFilterEntry>(bounds, filter));
  apply_last_entry();
}

void LayerStateStack::push_image_filter(
    const SkRect& bounds,
    const std::shared_ptr<const DlImageFilter> filter) {
  state_stack_.emplace_back(std::make_unique<ImageFilterEntry>(bounds, filter));
  apply_last_entry();
}

void LayerStateStack::push_backdrop(
    const SkRect& bounds,
    const std::shared_ptr<const DlImageFilter> filter,
    DlBlendMode blend_mode) {
  state_stack_.emplace_back(std::make_unique<BackdropFilterEntry>(
      bounds, filter, blend_mode, do_checkerboard_));
  apply_last_entry();
}

void LayerStateStack::push_translate(SkScalar tx, SkScalar ty) {
  state_stack_.emplace_back(std::make_unique<TranslateEntry>(tx, ty));
  apply_last_entry();
}

void LayerStateStack::push_transform(const SkM44& m44) {
  state_stack_.emplace_back(std::make_unique<TransformM44Entry>(m44));
  apply_last_entry();
}

void LayerStateStack::push_transform(const SkMatrix& matrix) {
  state_stack_.emplace_back(std::make_unique<TransformMatrixEntry>(matrix));
  apply_last_entry();
}

void LayerStateStack::push_integral_transform() {
  state_stack_.emplace_back(std::make_unique<IntegralTransformEntry>());
  apply_last_entry();
}

void LayerStateStack::push_clip_rect(const SkRect& rect, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRectEntry>(rect, is_aa));
  apply_last_entry();
}

void LayerStateStack::push_clip_rrect(const SkRRect& rrect, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRRectEntry>(rrect, is_aa));
  apply_last_entry();
}

void LayerStateStack::push_clip_path(const SkPath& path, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipPathEntry>(path, is_aa));
  apply_last_entry();
}

bool LayerStateStack::needs_save_layer(int flags) const {
  if (outstanding_.opacity < SK_Scalar1 &&
      (flags & LayerStateStack::CALLER_CAN_APPLY_OPACITY) == 0) {
    return true;
  }
  if (outstanding_.image_filter &&
      (flags & LayerStateStack::CALLER_CAN_APPLY_IMAGE_FILTER) == 0) {
    return true;
  }
  if (outstanding_.color_filter &&
      (flags & LayerStateStack::CALLER_CAN_APPLY_COLOR_FILTER) == 0) {
    return true;
  }
  return false;
}

void LayerStateStack::save_layer(const SkRect& bounds) {
  push_attributes();
  state_stack_.emplace_back(std::make_unique<SaveLayerEntry>(
      bounds, DlBlendMode::kSrcOver, do_checkerboard_));
  apply_last_entry();
}

void LayerStateStack::maybe_save_layer_for_transform() {
  // Alpha and ColorFilter don't care about transform
  if (outstanding_.image_filter) {
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer_for_clip() {
  // Alpha of clipped content == clip of alpha content
  // Color-filtering of clipped content == clip of color-filtered content
  if (outstanding_.image_filter) {
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer(int apply_flags) {
  if (needs_save_layer(apply_flags)) {
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer(SkScalar opacity) {
  if (outstanding_.image_filter) {
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer(
    const std::shared_ptr<const DlColorFilter> filter) {
  if (outstanding_.color_filter || outstanding_.image_filter ||
      (outstanding_.opacity < SK_Scalar1 &&
       !filter->can_commute_with_alpha())) {
    // TBD: compose the 2 color filters together.
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer(
    const std::shared_ptr<const DlImageFilter> filter) {
  if (outstanding_.image_filter) {
    // TBD: compose the 2 image filters together.
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::AttributesEntry::restore(
    RenderingAttributes* attributes,
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  *attributes = attributes_;
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
    SkPaint paint;
    canvas->saveLayer(bounds_, attributes->fill(paint, blend_mode_));
  }
  if (builder) {
    DlPaint paint;
    builder->saveLayer(&bounds_, attributes->fill(paint, blend_mode_));
  }
  *attributes = {};
}

void LayerStateStack::SaveLayerEntry::do_checkerboard(
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (do_checkerboard_) {
    if (canvas) {
      DrawCheckerboard(canvas, bounds_);
    }
    if (builder) {
      DrawCheckerboard(builder, bounds_);
    }
  }
}

void LayerStateStack::OpacityEntry::apply(RenderingAttributes* attributes,
                                          SkCanvas* canvas,
                                          DisplayListBuilder* builder) const {
  attributes->save_layer_bounds = bounds_;
  attributes->opacity *= opacity_;
}

void LayerStateStack::ImageFilterEntry::apply(
    RenderingAttributes* attributes,
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  attributes->save_layer_bounds = bounds_;
  attributes->image_filter = filter_;
}

void LayerStateStack::ColorFilterEntry::apply(
    RenderingAttributes* attributes,
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  attributes->save_layer_bounds = bounds_;
  attributes->color_filter = filter_;
}

void LayerStateStack::BackdropFilterEntry::apply(
    RenderingAttributes* attributes,
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas) {
    sk_sp<SkImageFilter> backdrop_filter =
        filter_ ? filter_->skia_object() : nullptr;
    SkPaint paint;
    SkPaint* pPaint = attributes->fill(paint, blend_mode_);
    canvas->saveLayer(
        SkCanvas::SaveLayerRec{&bounds_, pPaint, backdrop_filter.get(), 0});
  }
  if (builder) {
    DlPaint paint;
    DlPaint* pPaint = attributes->fill(paint, blend_mode_);
    builder->saveLayer(&bounds_, pPaint, filter_.get());
  }
  *attributes = {};
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
  SaveLayerEntry::apply(attributes, canvas, builder);
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

void LayerStateStack::IntegralTransformEntry::apply(
    RenderingAttributes* attributes,
    SkCanvas* canvas,
    DisplayListBuilder* builder) const {
  if (canvas != nullptr) {
    auto matrix = canvas->getTotalMatrix();
    matrix = RasterCacheUtil::GetIntegralTransCTM(matrix);
    canvas->setMatrix(matrix);
  }
  if (builder != nullptr) {
    auto matrix = canvas->getTotalMatrix();
    matrix = RasterCacheUtil::GetIntegralTransCTM(matrix);
    builder->transformReset();
    builder->transform(matrix);
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
