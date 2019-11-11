// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/container_layer.h"

#include "flutter/fml/trace_event.h"

#if defined(OS_FUCHSIA)
#include "flutter/flow/scene_update_context.h"  //nogncheck
#endif                                          // defined(OS_FUCHSIA)

namespace flutter {
namespace {

float ClampElevation(float elevation,
                     float parent_elevation,
                     float max_elevation) {
  // TODO(mklim): Deal with bounds overflow more elegantly. We'd like to be
  // able to have developers specify the behavior here to alternatives besides
  // clamping, like normalization on some arbitrary curve.
  float clamped_elevation = elevation;
  if (max_elevation > -1 && (parent_elevation + elevation) > max_elevation) {
    // Clamp the local z coordinate at our max bound. Take into account the
    // parent z position here to fix clamping in cases where the child is
    // overflowing because of its parents.
    clamped_elevation = max_elevation - parent_elevation;
  }

  return clamped_elevation;
}

}  // namespace

void ContainerLayer::Add(std::shared_ptr<Layer> layer) {
  layers_.emplace_back(std::move(layer));
}

void ContainerLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "ContainerLayer::Preroll");

  // Platform views have no children, so context->has_platform_view should
  // always be false.
  FML_DCHECK(!context->has_platform_view);

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  bool child_has_platform_view = false;
  for (auto& layer : layers_) {
    // Reset context->has_platform_view to false so that layers aren't treated
    // as if they have a platform view based on one being previously found in a
    // sibling tree.
    context->has_platform_view = false;

    layer->Preroll(context, matrix);
    if (layer->needs_system_composite()) {
      set_needs_system_composite(true);
    }
    child_paint_bounds.join(layer->paint_bounds());

    child_has_platform_view =
        child_has_platform_view || context->has_platform_view;
  }

  context->has_platform_view = child_has_platform_view;
  set_paint_bounds(child_paint_bounds);
}

void ContainerLayer::Paint(PaintContext& context) const {
  FML_DCHECK(needs_painting());

  // Intentionally not tracing here as there should be no self-time
  // and the trace event on this common function has a small overhead.
  for (auto& layer : layers_) {
    if (layer->needs_painting()) {
      layer->Paint(context);
    }
  }
}

#if defined(OS_FUCHSIA)

void ContainerLayer::UpdateScene(SceneUpdateContext& context) {
  FML_DCHECK(needs_system_composite());

  // Update all of the Layers which are part of the container.  This may cause
  // additional child |Frame|s to be created.
  for (auto& layer : layers()) {
    if (layer->needs_system_composite()) {
      layer->UpdateScene(context);
    }
  }
}

#endif  // defined(OS_FUCHSIA)

ElevatedContainerLayer::ElevatedContainerLayer(float elevation)
    : elevation_(elevation), clamped_elevation_(elevation) {}

void ElevatedContainerLayer::Preroll(PrerollContext* context,
                                     const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "ElevatedContainerLayer::Preroll");

  // Track total elevation as we walk the tree, in order to deal with bounds
  // overflow in z.
  parent_elevation_ = context->total_elevation;
  clamped_elevation_ = ClampElevation(elevation_, parent_elevation_,
                                      context->frame_physical_depth);
  context->total_elevation += clamped_elevation_;

  ContainerLayer::Preroll(context, matrix);

  // Restore the elevation for our parent.
  context->total_elevation = parent_elevation_;
}

FuchsiaSystemCompositedContainerLayer::FuchsiaSystemCompositedContainerLayer(
    SkColor color,
    SkAlpha opacity,
    float elevation)
    : ElevatedContainerLayer(elevation), color_(color), opacity_(opacity) {}

void FuchsiaSystemCompositedContainerLayer::Preroll(PrerollContext* context,
                                                    const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "SystemCompositedContainerLayer::Preroll");
#if !defined(OS_FUCHSIA)
  FML_NOTIMPLEMENTED();
#endif  // !defined(OS_FUCHSIA)

  // System-composite this layer.
  set_needs_system_composite(true);

  const float parent_is_opaque = context->is_opaque;
  context->mutators_stack.PushOpacity(opacity_);
  context->is_opaque = parent_is_opaque && (opacity_ == 255);
  ElevatedContainerLayer::Preroll(context, matrix);
  context->is_opaque = parent_is_opaque;
  context->mutators_stack.Pop();
}

void FuchsiaSystemCompositedContainerLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "SystemCompositedContainerLayer::Paint");
  FML_DCHECK(needs_painting());

  if (needs_system_composite()) {
#if !defined(OS_FUCHSIA)
    FML_NOTIMPLEMENTED();
#endif  // !defined(OS_FUCHSIA)

    // If we are being rendered into our own frame using the system compositor,
    // then it is neccesary to "punch a hole" in the canvas/frame behind us so
    // that group opacity looks correct.
    SkPaint paint;
    paint.setColor(SK_ColorTRANSPARENT);
    paint.setBlendMode(SkBlendMode::kSrc);
    context.leaf_nodes_canvas->drawRect(paint_bounds(), paint);
  }
}

#if defined(OS_FUCHSIA)

void FuchsiaSystemCompositedContainerLayer::UpdateScene(
    SceneUpdateContext& context) {
  FML_DCHECK(needs_system_composite());

  // Retained rendering: speedup by reusing a retained entity node if
  // possible. When an entity node is reused, no paint layer is added to the
  // frame so we won't call Paint.
  LayerRasterCacheKey key(unique_id(), context.Matrix());
  if (context.HasRetainedNode(key)) {
    const scenic::EntityNode& retained_node = context.GetRetainedNode(key);
    FML_DCHECK(context.top_entity());
    FML_DCHECK(retained_node.session() == context.session());
    context.top_entity()->embedder_node().AddChild(retained_node);
    return;
  }

  SceneUpdateContext::Frame frame(context, rrect_, color_, opacity_ / 255.0f,
                                  elevation(), this);
  // Paint the child layers into the Frame.
  for (auto& layer : layers()) {
    if (layer->needs_painting()) {
      frame.AddPaintLayer(layer.get());
    }
  }

  ContainerLayer::UpdateScene(context);
}

#endif  // defined(OS_FUCHSIA)

}  // namespace flutter
