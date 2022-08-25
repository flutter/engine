// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/physical_shape_layer.h"

#include "flutter/display_list/display_list_canvas_dispatcher.h"
#include "flutter/flow/paint_utils.h"

namespace flutter {

PhysicalShapeLayer::PhysicalShapeLayer(SkColor color,
                                       SkColor shadow_color,
                                       float elevation,
                                       const SkPath& path,
                                       Clip clip_behavior)
    : color_(color),
      shadow_color_(shadow_color),
      elevation_(elevation),
      path_(path),
      clip_behavior_(clip_behavior) {}

void PhysicalShapeLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const PhysicalShapeLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (color_ != prev->color_ || shadow_color_ != prev->shadow_color_ ||
        elevation_ != prev->elevation() || path_ != prev->path_ ||
        clip_behavior_ != prev->clip_behavior_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }

  SkRect bounds;
  if (elevation_ == 0) {
    bounds = path_.getBounds();
  } else {
    bounds = DisplayListCanvasDispatcher::ComputeShadowBounds(
        path_, elevation_, context->frame_device_pixel_ratio(),
        context->GetTransform());
  }

  context->AddLayerBounds(bounds);

  // Only push cull rect if there is clip.
  if (clip_behavior_ == Clip::none || context->PushCullRect(bounds)) {
    DiffChildren(context, prev);
  }
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void PhysicalShapeLayer::Preroll(PrerollContext* context,
                                 const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "PhysicalShapeLayer::Preroll");
  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context, UsesSaveLayer());

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);

  SkRect paint_bounds;
  if (elevation_ == 0) {
    paint_bounds = path_.getBounds();
  } else {
    // We will draw the shadow in Paint(), so add some margin to the paint
    // bounds to leave space for the shadow.
    paint_bounds = DisplayListCanvasDispatcher::ComputeShadowBounds(
        path_, elevation_, context->frame_device_pixel_ratio, matrix);
  }

  if (clip_behavior_ == Clip::none) {
    paint_bounds.join(child_paint_bounds);
  }

  set_paint_bounds(paint_bounds);
}

void PhysicalShapeLayer::Paint(PaintContext& context) const {
  //
  // Is this layer even used any more ????
  //

  TRACE_EVENT0("flutter", "PhysicalShapeLayer::Paint");
  FML_DCHECK(needs_painting(context));

  if (elevation_ != 0) {
    DisplayListCanvasDispatcher::DrawShadow(
        context.canvas, path_, shadow_color_, elevation_,
        SkColorGetA(color_) != 0xff, context.frame_device_pixel_ratio);
  }

  auto save = context.state_stack.save();
  if (clip_behavior_ == Clip::antiAliasWithSaveLayer) {
    context.state_stack.clipPath(path_, true);
    auto saveLayer = context.state_stack.saveLayer(
        &paint_bounds(), context.checkerboard_offscreen_layers);
    context.canvas->drawColor(color_);
    PaintChildren(context);
  } else {
    SkPaint paint;
    paint.setColor(color_);
    paint.setAntiAlias(true);

    // Call drawPath without clip if possible for better performance.
    context.canvas->drawPath(path_, paint);
    if (clip_behavior_ != Clip::none) {
      context.state_stack.clipPath(path_, clip_behavior_ == Clip::antiAlias);
    }
    PaintChildren(context);
  }
}

}  // namespace flutter
