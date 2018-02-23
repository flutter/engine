// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rrect_layer.h"
#include "flutter/flow/system_compositor_context.h"

namespace flow {

ClipRRectLayer::ClipRRectLayer() = default;

ClipRRectLayer::~ClipRRectLayer() = default;

void ClipRRectLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);

  if (child_paint_bounds.intersect(clip_rrect_.getBounds())) {
    set_paint_bounds(child_paint_bounds);
  }
}


void ClipRRectLayer::UpdateScene(SystemCompositorContext &context) {
  FXL_DCHECK(needs_system_composite());
  context.PushLayer(paint_bounds());
  context.ClipFrame();
  SkPath path;
  path.addRRect(clip_rrect_);
  context.SetClipPath(path);

  UpdateSceneChildren(context);
  context.PopLayer();
}


void ClipRRectLayer::Paint(PaintContext& context) {
  TRACE_EVENT0("flutter", "ClipRRectLayer::Paint");
  FXL_DCHECK(needs_painting());

  Layer::AutoSaveLayer save(context, paint_bounds(), nullptr);
  context.canvas.clipRRect(clip_rrect_, true);
  PaintChildren(context);
}

}  // namespace flow
