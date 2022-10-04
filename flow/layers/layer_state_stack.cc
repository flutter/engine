// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer_state_stack.h"
#include "flutter/flow/paint_utils.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

using AutoRestore = LayerStateStack::AutoRestore;
using MutatorContext = LayerStateStack::MutatorContext;

void LayerStateStack::clear_delegate() {
  if (canvas_) {
    canvas_->restoreToCount(restore_count_);
    canvas_ = nullptr;
  }
  if (builder_) {
    builder_->restoreToCount(restore_count_);
    builder_ = nullptr;
  }
  if (mutators_) {
    mutators_->PopTo(restore_count_);
    mutators_ = nullptr;
  }
}

void LayerStateStack::set_delegate(SkCanvas* canvas) {
  clear_delegate();
  if (canvas) {
    restore_count_ = canvas->getSaveCount();
    canvas_ = canvas;
    reapply_all();
  }
}

void LayerStateStack::set_delegate(DisplayListBuilder* builder) {
  clear_delegate();
  if (builder) {
    restore_count_ = builder->getSaveCount();
    builder_ = builder;
    reapply_all();
  }
}

void LayerStateStack::set_delegate(MutatorsStack* stack) {
  clear_delegate();
  if (stack) {
    restore_count_ = stack->stack_count();
    mutators_ = stack;
    reapply_all();
  }
}

void LayerStateStack::reapply_all() {
  // We use a local RenderingAttributes instance so that it can track the
  // necessary state changes independently as they occur in the stack.
  // Reusing |outstanding_| would wreak havoc on the current state of
  // the stack. When we are finished, though, the local attributes
  // contents should match the current outstanding_ values;
  RenderingAttributes attributes = outstanding_;
  outstanding_ = {};
  for (auto& state : state_stack_) {
    state->reapply(this);
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
                                                    DlBlendMode mode) const {
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
                                                    DlBlendMode mode) const {
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

bool LayerStateStack::needs_painting(const SkRect& bounds) const {
  if (!needs_painting()) {
    // Answer based on outstanding attributes...
    return false;
  }
  if (canvas_) {
    // Workaround for Skia bug (quickReject does not reject empty bounds).
    // https://bugs.chromium.org/p/skia/issues/detail?id=10951
    if (bounds.isEmpty()) {
      return false;
    }
    return !canvas_->quickReject(bounds);
  }
  if (builder_) {
    return !builder_->quickReject(bounds);
  }
  // We could track the attributes ourselves, but this method is
  // unlikely to be called without a canvas or builder to back it up.
  return true;
}

MutatorContext LayerStateStack::save() {
  auto ret = MutatorContext(this);
  state_stack_.emplace_back(std::make_unique<SaveEntry>());
  state_stack_.back()->apply(this);
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
    const std::shared_ptr<const DlImageFilter>& filter) {
  if (filter) {
    layer_state_stack_->push_attributes();
    layer_state_stack_->maybe_save_layer(filter);
    layer_state_stack_->push_image_filter(bounds, filter);
  }
}

void MutatorContext::applyColorFilter(
    const SkRect& bounds,
    const std::shared_ptr<const DlColorFilter>& filter) {
  if (filter) {
    layer_state_stack_->push_attributes();
    layer_state_stack_->maybe_save_layer(filter);
    layer_state_stack_->push_color_filter(bounds, filter);
  }
}

void MutatorContext::applyBackdropFilter(
    const SkRect& bounds,
    const std::shared_ptr<const DlImageFilter>& filter,
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
    state_stack_.back()->restore(this);
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
    const std::shared_ptr<const DlColorFilter>& filter) {
  state_stack_.emplace_back(std::make_unique<ColorFilterEntry>(bounds, filter));
  apply_last_entry();
}

void LayerStateStack::push_image_filter(
    const SkRect& bounds,
    const std::shared_ptr<const DlImageFilter>& filter) {
  state_stack_.emplace_back(std::make_unique<ImageFilterEntry>(bounds, filter));
  apply_last_entry();
}

void LayerStateStack::push_backdrop(
    const SkRect& bounds,
    const std::shared_ptr<const DlImageFilter>& filter,
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
    const std::shared_ptr<const DlColorFilter>& filter) {
  if (outstanding_.color_filter || outstanding_.image_filter ||
      (outstanding_.opacity < SK_Scalar1 &&
       !filter->can_commute_with_opacity())) {
    // TBD: compose the 2 color filters together.
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer(
    const std::shared_ptr<const DlImageFilter>& filter) {
  if (outstanding_.image_filter) {
    // TBD: compose the 2 image filters together.
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::AttributesEntry::restore(LayerStateStack* stack) const {
  stack->outstanding_ = attributes_;
}

void LayerStateStack::SaveEntry::apply(LayerStateStack* stack) const {
  if (stack->canvas_) {
    stack->canvas_->save();
  }
  if (stack->builder_) {
    stack->builder_->save();
  }
}

void LayerStateStack::SaveEntry::restore(LayerStateStack* stack) const {
  if (stack->canvas_) {
    stack->canvas_->restore();
  }
  if (stack->builder_) {
    stack->builder_->restore();
  }
  do_checkerboard(stack->canvas_, stack->builder_);
}

void LayerStateStack::SaveLayerEntry::apply(LayerStateStack* stack) const {
  if (stack->canvas_) {
    SkPaint paint;
    stack->canvas_->saveLayer(bounds_,
                              stack->outstanding_.fill(paint, blend_mode_));
  }
  if (stack->builder_) {
    DlPaint paint;
    stack->builder_->saveLayer(&bounds_,
                               stack->outstanding_.fill(paint, blend_mode_));
  }
  stack->outstanding_ = {};
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

void LayerStateStack::OpacityEntry::apply(LayerStateStack* stack) const {
  stack->outstanding_.save_layer_bounds = bounds_;
  stack->outstanding_.opacity *= opacity_;
  if (stack->mutators_) {
    stack->mutators_->PushOpacity(DlColor::toAlpha(opacity_));
  }
}

void LayerStateStack::ImageFilterEntry::apply(LayerStateStack* stack) const {
  stack->outstanding_.save_layer_bounds = bounds_;
  stack->outstanding_.image_filter = filter_;
  if (stack->mutators_) {
    // MutatorsStack::PushImageFilter does not exist...
  }
}

void LayerStateStack::ColorFilterEntry::apply(LayerStateStack* stack) const {
  stack->outstanding_.save_layer_bounds = bounds_;
  stack->outstanding_.color_filter = filter_;
  if (stack->mutators_) {
    // MutatorsStack::PushColorFilter does not exist...
  }
}

void LayerStateStack::BackdropFilterEntry::apply(LayerStateStack* stack) const {
  if (stack->canvas_) {
    sk_sp<SkImageFilter> backdrop_filter =
        filter_ ? filter_->skia_object() : nullptr;
    SkPaint paint;
    SkPaint* pPaint = stack->outstanding_.fill(paint, blend_mode_);
    stack->canvas_->saveLayer(
        SkCanvas::SaveLayerRec{&bounds_, pPaint, backdrop_filter.get(), 0});
  }
  if (stack->builder_) {
    DlPaint paint;
    DlPaint* pPaint = stack->outstanding_.fill(paint, blend_mode_);
    stack->builder_->saveLayer(&bounds_, pPaint, filter_.get());
  }
  if (stack->mutators_) {
    stack->mutators_->PushBackdropFilter(filter_);
  }
  stack->outstanding_ = {};
}

void LayerStateStack::BackdropFilterEntry::reapply(
    LayerStateStack* stack) const {
  // On the reapply for subsequent overlay layers, we do not
  // want to reapply the backdrop filter, but we do need to
  // do a saveLayer to encapsulate the contents and match the
  // restore that will be forthcoming. Note that this is not
  // perfect if the BlendMode is not associative as we will be
  // compositing multiple parts of the content in batches.
  // Luckily the most common SrcOver is associative.
  SaveLayerEntry::apply(stack);
}

void LayerStateStack::TranslateEntry::apply(LayerStateStack* stack) const {
  if (stack->canvas_) {
    stack->canvas_->translate(tx_, ty_);
  }
  if (stack->builder_) {
    stack->builder_->translate(tx_, ty_);
  }
  if (stack->mutators_) {
    stack->mutators_->PushTransform(SkMatrix::Translate(tx_, ty_));
  }
}

void LayerStateStack::TransformMatrixEntry::apply(
    LayerStateStack* stack) const {
  if (stack->canvas_) {
    stack->canvas_->concat(matrix_);
  }
  if (stack->builder_) {
    stack->builder_->transform(matrix_);
  }
  if (stack->mutators_) {
    stack->mutators_->PushTransform(matrix_);
  }
}

void LayerStateStack::TransformM44Entry::apply(LayerStateStack* stack) const {
  if (stack->canvas_) {
    stack->canvas_->concat(m44_);
  }
  if (stack->builder_) {
    stack->builder_->transform(m44_);
  }
  if (stack->mutators_) {
    stack->mutators_->PushTransform(m44_.asM33());
  }
}

void LayerStateStack::IntegralTransformEntry::apply(
    LayerStateStack* stack) const {
  if (stack->canvas_) {
    auto matrix = stack->canvas_->getTotalMatrix();
    matrix = RasterCacheUtil::GetIntegralTransCTM(matrix);
    stack->canvas_->setMatrix(matrix);
  }
  if (stack->builder_) {
    auto matrix = stack->builder_->getTransform();
    matrix = RasterCacheUtil::GetIntegralTransCTM(matrix);
    stack->builder_->transformReset();
    stack->builder_->transform(matrix);
  }
}

void LayerStateStack::ClipRectEntry::apply(LayerStateStack* stack) const {
  if (stack->canvas_) {
    stack->canvas_->clipRect(rect_, SkClipOp::kIntersect, is_aa_);
  }
  if (stack->builder_) {
    stack->builder_->clipRect(rect_, SkClipOp::kIntersect, is_aa_);
  }
  if (stack->mutators_) {
    stack->mutators_->PushClipRect(rect_);
  }
}

void LayerStateStack::ClipRRectEntry::apply(LayerStateStack* stack) const {
  if (stack->canvas_) {
    stack->canvas_->clipRRect(rrect_, SkClipOp::kIntersect, is_aa_);
  }
  if (stack->builder_) {
    stack->builder_->clipRRect(rrect_, SkClipOp::kIntersect, is_aa_);
  }
  if (stack->mutators_) {
    stack->mutators_->PushClipRRect(rrect_);
  }
}

void LayerStateStack::ClipPathEntry::apply(LayerStateStack* stack) const {
  if (stack->canvas_) {
    stack->canvas_->clipPath(path_, SkClipOp::kIntersect, is_aa_);
  }
  if (stack->builder_) {
    stack->builder_->clipPath(path_, SkClipOp::kIntersect, is_aa_);
  }
  if (stack->mutators_) {
    stack->mutators_->PushClipPath(path_);
  }
}

}  // namespace flutter
