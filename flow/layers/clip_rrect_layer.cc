// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rrect_layer.h"

namespace flow {

ClipRRectLayer::ClipRRectLayer() = default;

ClipRRectLayer::~ClipRRectLayer() = default;

void ClipRRectLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  PrerollChildren(context, matrix);
  if (needs_system_composite())
    return;

  if (!context->child_paint_bounds.intersect(clip_rrect_.getBounds()))
    context->child_paint_bounds.setEmpty();
  set_paint_bounds(context->child_paint_bounds);
}

#if defined(OS_FUCHSIA)

void ClipRRectLayer::UpdateScene(SceneUpdateContext& context,
                                 mozart::client::ContainerNode& container) {
  FTL_DCHECK(needs_system_composite());

  // TODO(MZ-137): Need to be able to express the radii as vectors.
  // TODO(MZ-138): Need to be able to specify an origin.
  mozart::client::RoundedRectangle clip_shape(
      context.session(),                                   // session
      clip_rrect_.width(),                                 //  width
      clip_rrect_.height(),                                //  height
      clip_rrect_.radii(SkRRect::kUpperLeft_Corner).x(),   //  top_left_radius
      clip_rrect_.radii(SkRRect::kUpperRight_Corner).x(),  //  top_right_radius
      clip_rrect_.radii(SkRRect::kLowerRight_Corner)
          .x(),                                          //  bottom_right_radius
      clip_rrect_.radii(SkRRect::kLowerLeft_Corner).x()  //  bottom_left_radius
      );

  mozart::client::ShapeNode shape_node(context.session());
  shape_node.SetShape(clip_shape);

  mozart::client::EntityNode node(context.session());
  node.AddPart(shape_node);
  node.SetClip(0u, true /* clip to self */);

  UpdateSceneChildrenInsideNode(context, container, node);
}

#endif  // defined(OS_FUCHSIA)

void ClipRRectLayer::Paint(PaintContext& context) {
  TRACE_EVENT0("flutter", "ClipRRectLayer::Paint");
  FTL_DCHECK(!needs_system_composite());

  Layer::AutoSaveLayer save(context, paint_bounds(), nullptr);
  context.canvas.clipRRect(clip_rrect_, true);
  PaintChildren(context);
}

}  // namespace flow
