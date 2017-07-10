// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rect_layer.h"

namespace flow {

ClipRectLayer::ClipRectLayer() = default;

ClipRectLayer::~ClipRectLayer() = default;

void ClipRectLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  PrerollChildren(context, matrix);
  if (!context->child_paint_bounds.intersect(clip_rect_))
    context->child_paint_bounds.setEmpty();
  set_paint_bounds(context->child_paint_bounds);
}

#if defined(OS_FUCHSIA)

void ClipRectLayer::UpdateScene(mozart::client::Session& session,
                                SceneUpdateContext& context,
                                ContainerNode& container) {
  // TODO(MZ-138): Need to be able to specify an origin.
  mozart::client::Rectangle clip_shape(&session,            // session
                                       clip_rect_.width(),  //  width
                                       clip_rect_.height()  //  height
                                       );
  mozart::client::ShapeNode shape_node(&session);
  shape_node.SetShape(clip_shape);

  mozart::client::EntityNode node(&session);
  node.AddPart(shape_node);
  node.SetClip(shape_node.id(), true /* clip to self */);

  UpdateSceneChildrenInsideNode(session, context, container, node);
}

#endif  // defined(OS_FUCHSIA)

void ClipRectLayer::Paint(PaintContext& context) {
  TRACE_EVENT0("flutter", "ClipRectLayer::Paint");
  FTL_DCHECK(!needs_system_composite());

  SkAutoCanvasRestore save(&context.canvas, true);
  context.canvas.clipRect(paint_bounds());
  PaintChildren(context);
}

}  // namespace flow
