// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/container_layer.h"

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

  if (!needs_system_composite()) {
    set_paint_bounds(child_paint_bounds);
  }
}

void ContainerLayer::PrerollChildren(PrerollContext* context,
                                     const SkMatrix& matrix,
                                     SkRect* child_paint_bounds) {
  for (auto& layer : layers_) {
    PrerollContext child_context = *context;
    layer->Preroll(&child_context, matrix);

    if (layer->needs_system_composite()) {
      set_needs_system_composite(true);
    } else {
      child_paint_bounds->join(layer->paint_bounds());
    }
  }

#if defined(OS_FUCHSIA)
  if (needs_system_composite()) {
    scale_x_ = matrix.getScaleX();
    scale_y_ = matrix.getScaleY();
  }
#endif  // defined(OS_FUCHSIA)
}

void ContainerLayer::PaintChildren(PaintContext& context) const {
  FTL_DCHECK(!needs_system_composite());

  // Intentionally not tracing here as there should be no self-time
  // and the trace event on this common function has a small overhead.
  for (auto& layer : layers_)
    layer->Paint(context);
}

#if defined(OS_FUCHSIA)

void ContainerLayer::UpdateScene(SceneUpdateContext& context,
                                 mozart::client::ContainerNode& container) {
  UpdateSceneChildren(context, container);
}

void ContainerLayer::UpdateSceneChildren(
    SceneUpdateContext& context,
    mozart::client::ContainerNode& container) {
  FTL_DCHECK(needs_system_composite());

  for (auto& layer : layers_) {
    if (layer->needs_system_composite()) {
      context.FinalizeCurrentPaintTaskIfNeeded(container, scale_x_, scale_y_);
      layer->UpdateScene(context, container);
    } else {
      context.AddLayerToCurrentPaintTask(layer.get());
    }
  }
  context.FinalizeCurrentPaintTaskIfNeeded(container, scale_x_, scale_y_);
}

#endif  // defined(OS_FUCHSIA)

}  // namespace flow
