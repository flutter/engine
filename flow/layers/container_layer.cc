// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/container_layer.h"

namespace flow {

ContainerLayer::ContainerLayer() {};

ContainerLayer::~ContainerLayer() = default;

void ContainerLayer::Add(std::unique_ptr<Layer> layer) {
  layer->set_parent(this);
  layers_.push_back(std::move(layer));
}

SkIRect ContainerLayer::OnPreroll(PrerollContext* context,
                             const SkMatrix& matrix,
                             const SkIRect& device_clip) {
  TRACE_EVENT0("flutter", "ContainerLayer::OnPreroll");

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds, device_clip);
  set_paint_bounds(child_paint_bounds);

  return device_clip;
}

void ContainerLayer::PrerollChildren(PrerollContext* context,
                                     const SkMatrix& child_matrix,
                                     SkRect* child_paint_bounds,
                                     const SkIRect& device_clip) {
  for (auto& layer : layers_) {
    PrerollContext child_context = *context;
    if (layer->needs_system_composite()) {
      set_needs_system_composite(true);
    }
    layer->Preroll(&child_context, child_matrix, device_clip);
    child_paint_bounds->join(layer->paint_bounds());
  }

  device_paint_bounds_ = ComputeDeviceIRect(child_matrix, paint_bounds());
  if (!device_paint_bounds_.intersect(device_clip)) {
    device_paint_bounds_.setEmpty();
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

#if defined(OS_FUCHSIA)

void ContainerLayer::UpdateScene(SceneUpdateContext& context) {
  UpdateSceneChildren(context);
}

void ContainerLayer::UpdateSceneChildren(SceneUpdateContext& context) {
  FXL_DCHECK(needs_system_composite());

  // Paint all of the layers which need to be drawn into the container.
  // These may be flattened down to a containing
  for (auto& layer : layers_) {
    if (layer->needs_system_composite()) {
      layer->UpdateScene(context);
    }
  }
}

#endif  // defined(OS_FUCHSIA)

}  // namespace flow
