// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/child_scene_layer.h"

namespace flow {

ChildSceneLayer::ChildSceneLayer() = default;

ChildSceneLayer::~ChildSceneLayer() = default;

void ChildSceneLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  set_needs_system_composite(true);

  float inverse_device_pixel_ratio = 1.f / device_pixel_ratio_;
  SkRect bounds =
      SkRect::MakeXYWH(offset_.x(), offset_.y(),
                       physical_size_.width() * inverse_device_pixel_ratio,
                       physical_size_.height() * inverse_device_pixel_ratio);
  set_paint_bounds(bounds);
  context->child_paint_bounds = bounds;
}

void ChildSceneLayer::Paint(PaintContext& context) {
  FTL_DCHECK(false) << "Failed to composite child scene.";
}

void ChildSceneLayer::UpdateScene(SceneUpdateContext& context,
                                  mozart::client::ContainerNode& container) {
  FTL_DCHECK(needs_system_composite());

  // TODO(jeffbrown): Set hit test behavior and clip.
  // It's worth asking whether all children should be clipped implicitly
  // or whether we should leave this up to the Flutter application to decide.
  // In some situations, it might be useful to allow children to draw
  // outside of their layout bounds.
  if (export_node_) {
    export_node_->Bind(context, container, offset_, 1.f / device_pixel_ratio_);
  }
}

}  // namespace flow
