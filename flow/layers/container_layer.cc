// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/system_compositor_context.h"

namespace flow {

ContainerLayer::ContainerLayer() {}

ContainerLayer::~ContainerLayer() = default;

void ContainerLayer::Add(std::unique_ptr<Layer> layer) {
  layer->set_parent(this);
  layers_.push_back(std::move(layer));
}

void ContainerLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "ContainerLayer::Preroll");

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);
  set_paint_bounds(child_paint_bounds);
}

void ContainerLayer::PrerollChildren(PrerollContext* context,
                                     const SkMatrix& child_matrix,
                                     SkRect* child_paint_bounds) {
  for (auto& layer : layers_) {
    PrerollContext child_context = *context;
    layer->Preroll(&child_context, child_matrix);

    if (layer->needs_system_composite()) {
      set_needs_system_composite(true);
    }
    child_paint_bounds->join(layer->paint_bounds());
  }
}

void ContainerLayer::PaintChildren(PaintContext& context) const {
  FXL_DCHECK(needs_painting());

  // Intentionally not tracing here as there should be no self-time
  // and the trace event on this common function has a small overhead.

  for (auto& layer : layers_) {
    if (layer->needs_painting()) {
      layer->Paint(context);
    }
  }
}

void ContainerLayer::UpdateScene(SystemCompositorContext& context) {
  UpdateSceneChildren(context);
}

static void flushAccumulator(SystemCompositorContext& context,
                             int& pushCount,
                             std::vector<Layer*> accumulator,
                             SkRect& accumulatorBounds,
                             SkRect& systemLayerBounds,
                             const SkRect& paintBounds) {
  if (!accumulator.empty() && accumulatorBounds.intersects(systemLayerBounds)) {
    if (accumulatorBounds.intersect(paintBounds)) {
      context.PushLayer(accumulatorBounds);
      pushCount++;
    }
  }
  for (Layer* child : accumulator) {
    context.AddPaintedLayer(child);
  }
  accumulator.clear();
  accumulatorBounds = SkRect::MakeEmpty();
}

void ContainerLayer::UpdateSceneChildren(SystemCompositorContext& context) {
  FXL_DCHECK(needs_system_composite());

  int pushCount = 0;
  std::vector<Layer*> accumulator;
  // Bounding box join of all layers in `accumulator`.
  SkRect accumulatorBounds = SkRect::MakeEmpty();
  SkRect systemLayerBounds = SkRect::MakeEmpty();

  for (auto& layer : layers_) {
    if (layer->needs_system_composite()) {
      flushAccumulator(context, pushCount, accumulator, accumulatorBounds,
                       systemLayerBounds, paint_bounds());
      systemLayerBounds.join(layer->paint_bounds());
      layer->UpdateScene(context);
    } else if (layer->needs_painting()) {
      if (!accumulator.empty() ||
          layer->paint_bounds().intersects(systemLayerBounds)) {
        accumulator.push_back(layer.get());
        accumulatorBounds.join(layer->paint_bounds());
      } else {
        context.AddPaintedLayer(layer.get());
      }
    }
  }
  flushAccumulator(context, pushCount, accumulator, accumulatorBounds,
                   systemLayerBounds, paint_bounds());
  for (int i = 0; i < pushCount; i++) {
    context.PopLayer();
  }
}

}  // namespace flow
