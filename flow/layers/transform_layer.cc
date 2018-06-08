// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/transform_layer.h"

namespace flow {

TransformLayer::TransformLayer() = default;

TransformLayer::~TransformLayer() = default;

void TransformLayer::Preroll(PrerollContext* context,
                             const SkMatrix& matrix,
                             const SkIRect& device_clip) {
  SkMatrix child_matrix;
  child_matrix.setConcat(matrix, transform_);

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, child_matrix, &child_paint_bounds, device_clip);

  transform_.mapRect(&child_paint_bounds);
  set_paint_bounds(child_paint_bounds);

  device_paint_bounds_ = ComputeDeviceIRect(matrix, paint_bounds());
  if (!device_paint_bounds_.intersect(device_clip)) {
    device_paint_bounds_.setEmpty();
  }
}

#if defined(OS_FUCHSIA)

void TransformLayer::UpdateScene(SceneUpdateContext& context) {
  FXL_DCHECK(needs_system_composite());

  SceneUpdateContext::Transform transform(context, transform_);
  UpdateSceneChildren(context);
}

#endif  // defined(OS_FUCHSIA)

void TransformLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "TransformLayer::Paint");
  FXL_DCHECK(needs_painting());

  SkAutoCanvasRestore save(&context.canvas, true);
  context.canvas.concat(transform_);
  PaintChildren(context);
}

}  // namespace flow
