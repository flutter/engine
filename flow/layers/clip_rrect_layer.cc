// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rrect_layer.h"
#include "flutter/flow/layers/hole_rrect_layer.h"

namespace flow {

ClipRRectLayer::ClipRRectLayer() = default;

ClipRRectLayer::~ClipRRectLayer() = default;

std::unique_ptr<Layer> ClipRRectLayer::WrapHoleForAncestor(std::unique_ptr<Layer> hole) {
  std::unique_ptr<HoleRRectLayer> clippedHole = std::make_unique<HoleRRectLayer>();
  clippedHole->set_clip_rrect(clip_rrect_);
  clippedHole->Add(std::move(hole));
  return clippedHole;
}

void ClipRRectLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);

  if (child_paint_bounds.intersect(clip_rrect_.getBounds())) {
    set_paint_bounds(child_paint_bounds);
  }
}

#if defined(OS_FUCHSIA)

void ClipRRectLayer::UpdateScene(SceneUpdateContext& context) {
  FXL_DCHECK(needs_system_composite());

  // TODO(MZ-137): Need to be able to express the radii as vectors.
  scenic_lib::RoundedRectangle shape(
      context.session(),                                   // session
      clip_rrect_.width(),                                 //  width
      clip_rrect_.height(),                                //  height
      clip_rrect_.radii(SkRRect::kUpperLeft_Corner).x(),   //  top_left_radius
      clip_rrect_.radii(SkRRect::kUpperRight_Corner).x(),  //  top_right_radius
      clip_rrect_.radii(SkRRect::kLowerRight_Corner)
          .x(),                                          //  bottom_right_radius
      clip_rrect_.radii(SkRRect::kLowerLeft_Corner).x()  //  bottom_left_radius
  );

  SceneUpdateContext::Clip clip(context, shape, clip_rrect_.getBounds());
  UpdateSceneChildren(context);
}

#endif  // defined(OS_FUCHSIA)

void ClipRRectLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "ClipRRectLayer::Paint");
  FXL_DCHECK(needs_painting());

  Layer::AutoSaveLayer save(context, paint_bounds(), nullptr);
  context.canvas.clipRRect(clip_rrect_, true);
  PaintChildren(context);
}

}  // namespace flow
