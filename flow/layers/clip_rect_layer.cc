// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rect_layer.h"
#include "flutter/flow/system_compositor_context.h"

namespace flow {

ClipRectLayer::ClipRectLayer() = default;

ClipRectLayer::~ClipRectLayer() = default;

void ClipRectLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);
  if (child_paint_bounds.intersect(clip_rect_)) {
    set_paint_bounds(child_paint_bounds);
  }
}


void ClipRectLayer::UpdateScene(SystemCompositorContext &context) {
  FXL_DCHECK(needs_system_composite());

  // scenic_lib::Rectangle shape(context.session(),   // session
  //                             clip_rect_.width(),  //  width
  //                             clip_rect_.height()  //  height
  // );

  // SceneUpdateContext::Clip clip(context, shape, clip_rect_);
      // FXL_LOG(INFO) << "cliprect";

  context.PushLayer(paint_bounds());
  context.ClipFrame();
  UpdateSceneChildren(context);
  context.PopLayer();

}


void ClipRectLayer::Paint(PaintContext& context) {
  TRACE_EVENT0("flutter", "ClipRectLayer::Paint");
  FXL_DCHECK(needs_painting());

  // if (needs_system_composite()) {
  //   context.layers.PushLayer(clip_rect_);
  //   context.layers.ClipRect();
  //   PaintChildren(context);
  //   context.layers.PopLayer();
  // } else {
    SkAutoCanvasRestore save(&context.canvas, true);
    context.canvas.clipRect(paint_bounds());
    PaintChildren(context);
 // }
}

}  // namespace flow
