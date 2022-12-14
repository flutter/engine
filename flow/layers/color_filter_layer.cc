// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/color_filter_layer.h"

#include "flutter/display_list/display_list_comparable.h"
#include "flutter/display_list/display_list_paint.h"
#include "flutter/flow/raster_cache_item.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

ColorFilterLayer::ColorFilterLayer(std::shared_ptr<const DlColorFilter> filter,
                                   const SkPoint& offset)
    : CacheableContainerLayer(
          RasterCacheUtil::kMinimumRendersBeforeCachingFilterLayer,
          true),
      filter_(std::move(filter)),
      offset_(offset) {}

void ColorFilterLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const ColorFilterLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (NotEquals(filter_, prev->filter_) || offset_ != prev->offset_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }

  context->PushTransform(SkMatrix::Translate(offset_.fX, offset_.fY));
  if (context->has_raster_cache()) {
    context->SetTransform(
        RasterCacheUtil::GetIntegralTransCTM(context->GetTransform()));
  }

  DiffChildren(context, prev);

  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void ColorFilterLayer::Preroll(PrerollContext* context) {
  auto mutator = context->state_stack.save();
  mutator.translate(offset_);

  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context);
  AutoCache cache = AutoCache(layer_raster_cache_item_.get(), context,
                              context->state_stack.transform_3x3());

  SkRect child_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, &child_bounds);

  // Our saveLayer would apply any outstanding opacity or any outstanding
  // image filter before it applies our color filter, but that is in the
  // wrong order compared to how these attributes were applied to the tree
  // (they would have come from one of our ancestors). So we cannot apply
  // those attributes with our saveLayer normally.
  // However, some color filters can commute themselves with an opacity
  // modulation so in that case we can apply the opacity on behalf of our
  // ancestors - otherwise we can apply no attributes.
  if (filter_) {
    child_bounds.offset(offset_);
    set_paint_bounds(child_bounds);

    context->renderable_state_flags =
        filter_->can_commute_with_opacity()
            ? LayerStateStack::kCallerCanApplyOpacity
            : 0;
  } else {
    // else - we can apply whatever our children can apply.
    set_paint_bounds(child_bounds);
  }
}

void ColorFilterLayer::Paint(PaintContext& context) const {
  FML_DCHECK(needs_painting(context));

  auto mutator = context.state_stack.save();
  mutator.translate(offset_);

  if (context.raster_cache) {
    // Always apply the integral transform in the presence of a raster cache
    // whether or not we will draw from the cache
    mutator.integralTransform();

    // Try drawing the layer cache item from the cache before applying the
    // color filter if it was cached with the filter applied.
    if (!layer_raster_cache_item_->IsCacheChildren()) {
      SkPaint sk_paint;
      if (layer_raster_cache_item_->Draw(context,
                                         context.state_stack.fill(sk_paint))) {
        return;
      }
    }
  }

  // Now apply the color filter and then try rendering children either from
  // cache or directly.
  mutator.applyColorFilter(paint_bounds(), filter_);

  if (context.raster_cache && layer_raster_cache_item_->IsCacheChildren()) {
    SkPaint sk_paint;
    if (layer_raster_cache_item_->Draw(context,
                                       context.state_stack.fill(sk_paint))) {
      return;
    }
  }

  PaintChildren(context);
}

}  // namespace flutter
