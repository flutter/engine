// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/shader_mask_layer.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

ShaderMaskLayer::ShaderMaskLayer(std::shared_ptr<DlColorSource> color_source,
                                 const SkRect& mask_rect,
                                 DlBlendMode blend_mode)
    : CacheableContainerLayer(
          RasterCacheUtil::kMinimumRendersBeforeCachingFilterLayer),
      color_source_(std::move(color_source)),
      mask_rect_(mask_rect),
      blend_mode_(blend_mode) {}

void ShaderMaskLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const ShaderMaskLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (color_source_ != prev->color_source_ ||
        mask_rect_ != prev->mask_rect_ || blend_mode_ != prev->blend_mode_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }
  DiffChildren(context, prev);

  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void ShaderMaskLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context);

  AutoCache cache = AutoCache(layer_raster_cache_item_.get(), context, matrix);

  ContainerLayer::Preroll(context, matrix);
  // We always paint with a saveLayer (or a cached rendering),
  // so we can always apply opacity in any of those cases.
  context->rendering_state_flags = LayerStateStack::CALLER_CAN_APPLY_OPACITY;
}

void ShaderMaskLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "ShaderMaskLayer::Paint");
  FML_DCHECK(needs_painting(context));

  if (context.raster_cache) {
    auto restore = context.state_stack.applyState(
        paint_bounds(), LayerStateStack::CALLER_CAN_APPLY_OPACITY);

    if (layer_raster_cache_item_->Draw(context,
                                       context.state_stack.sk_paint())) {
      return;
    }
  }
  auto shader_rect = SkRect::MakeWH(mask_rect_.width(), mask_rect_.height());

  auto restore = context.state_stack.saveLayer(&paint_bounds());
  PaintChildren(context);

  if (context.builder) {
    DlPaint dl_paint;
    dl_paint.setBlendMode(blend_mode_);
    if (color_source_) {
      dl_paint.setColorSource(color_source_.get());
    }
    context.builder->translate(mask_rect_.left(), mask_rect_.top());
    context.builder->drawRect(shader_rect, dl_paint);
  } else {
    SkPaint paint;
    paint.setBlendMode(ToSk(blend_mode_));
    if (color_source_) {
      paint.setShader(color_source_->skia_object());
    }
    context.canvas->translate(mask_rect_.left(), mask_rect_.top());
    context.canvas->drawRect(shader_rect, paint);
  }
}

}  // namespace flutter
