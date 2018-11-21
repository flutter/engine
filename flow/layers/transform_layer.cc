// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/transform_layer.h"

namespace flow {

TransformLayer::TransformLayer() = default;

TransformLayer::~TransformLayer() = default;

void TransformLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  SkMatrix child_matrix;
  child_matrix.setConcat(matrix, transform_);

  SkRect previous_clip_rect = context->clip_rect;
  SkMatrix inverse_transform_;
  if (transform_.invert(&inverse_transform_)) {
    inverse_transform_.mapRect(&context->clip_rect);
  } else {
    context->clip_rect = SkRect::MakeLTRB(-1e6, -1e6, 1e6, 1e6);
  }

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, child_matrix, &child_paint_bounds);

  transform_.mapRect(&child_paint_bounds);
  set_paint_bounds(child_paint_bounds);

  context->clip_rect = previous_clip_rect;
}

#if defined(OS_FUCHSIA)

void TransformLayer::UpdateScene(SceneUpdateContext& context) {
  FML_DCHECK(needs_system_composite());

  SceneUpdateContext::Transform transform(context, transform_);
  UpdateSceneChildren(context);
}

#endif  // defined(OS_FUCHSIA)

void TransformLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "TransformLayer::Paint");
  FML_DCHECK(needs_painting());

  SkAutoCanvasRestore save(context.internal_nodes_canvas, true);
  context.internal_nodes_canvas->concat(transform_);
  PaintChildren(context);
}

}  // namespace flow
