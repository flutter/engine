// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_path_layer.h"

#if defined(OS_FUCHSIA)

#include "apps/mozart/lib/scene/session_helpers.h"  // nogncheck

#endif  // defined(OS_FUCHSIA)

namespace flow {

ClipPathLayer::ClipPathLayer() = default;

ClipPathLayer::~ClipPathLayer() = default;

void ClipPathLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  PrerollChildren(context, matrix);
  if (!context->child_paint_bounds.intersect(clip_path_.getBounds()))
    context->child_paint_bounds.setEmpty();
  set_paint_bounds(context->child_paint_bounds);
}

#if defined(OS_FUCHSIA)

void ClipPathLayer::UpdateScene(mozart::client::Session& session,
                                SceneUpdateContext& context,
                                ContainerNode& container) {
  // TODO(MZ-140): Must be able to specify paths as shapes to nodes.
  //               Treating the shape as a rectangle for now.
  auto rect = clip_path_.getBounds();
  // TODO(MZ-138): Need to be able to specify an origin.
  mozart::client::Rectangle clip_shape(&session,      // session
                                       rect.width(),  //  width
                                       rect.height()  //  height
                                       );
  mozart::client::ShapeNode shape_node(&session);
  shape_node.SetShape(clip_shape);

  mozart::client::EntityNode node(&session);
  node.AddPart(shape_node);
  node.SetClip(shape_node.id(), true /* clip to self */);

  UpdateSceneChildrenInsideNode(session, context, container, node);
}

#endif  // defined(OS_FUCHSIA)

void ClipPathLayer::Paint(PaintContext& context) {
  TRACE_EVENT0("flutter", "ClipPathLayer::Paint");
  FTL_DCHECK(!needs_system_composite());

  Layer::AutoSaveLayer save(context, paint_bounds(), nullptr);
  context.canvas.clipPath(clip_path_, true);
  PaintChildren(context);
}

}  // namespace flow
