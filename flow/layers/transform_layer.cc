// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/transform_layer.h"
#include "flutter/flow/system_compositor_context.h"

namespace flow {

TransformLayer::TransformLayer() = default;

TransformLayer::~TransformLayer() = default;

void TransformLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  SkMatrix child_matrix;
  child_matrix.setConcat(matrix, transform_);

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, child_matrix, &child_paint_bounds);

  transform_.mapRect(&child_paint_bounds);
  set_paint_bounds(child_paint_bounds);
}

void TransformLayer::UpdateScene(SystemCompositorContext& context) {
  FXL_DCHECK(needs_system_composite());
  context.Transform(transform_);
  std::string s = "";
  for (int i = 0; i < indent; i++) {
    s += "  ";
  }

  FXL_DLOG(INFO) << s << "Transform";
  UpdateSceneChildren(context);

  context.PopTransform();
}

void TransformLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "TransformLayer::Paint");
  FXL_DCHECK(needs_painting());

  SkAutoCanvasRestore save(&context.canvas, true);
  context.canvas.concat(transform_);
  PaintChildren(context);
}

}  // namespace flow
