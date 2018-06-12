// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rrect_layer.h"

namespace flow {

ClipRRectLayer::ClipRRectLayer() {
  set_needs_system_composite(true);
}

ClipRRectLayer::~ClipRRectLayer() = default;

SkIRect ClipRRectLayer::OnPreroll(PrerollContext* context,
                                  const SkMatrix& matrix,
                                  const SkIRect& device_clip) {
  SkIRect new_device_clip = ComputeDeviceIRect(matrix, clip_rrect_.getBounds());
  IntersectOrSetEmpty(new_device_clip, device_clip);

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds, new_device_clip);
  if (child_paint_bounds.intersect(clip_rrect_.getBounds())) {
    set_paint_bounds(child_paint_bounds);
  }

  return new_device_clip;
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

  SkAutoCanvasRestore save(&context.canvas, true);
  context.canvas.clipRRect(clip_rrect_, true);
  PaintChildren(context);
}

}  // namespace flow
