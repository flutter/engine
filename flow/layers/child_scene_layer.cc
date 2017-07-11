// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/child_scene_layer.h"

namespace flow {

ChildSceneLayer::ChildSceneLayer() = default;

ChildSceneLayer::~ChildSceneLayer() = default;

void ChildSceneLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  set_needs_system_composite(true);
}

void ChildSceneLayer::Paint(PaintContext& context) {
  FTL_NOTREACHED() << "UpdateScene() should have been called instead.";
}

void ChildSceneLayer::UpdateScene(SceneUpdateContext& context,
                                  mozart::client::ContainerNode& container) {
  FTL_DCHECK(needs_system_composite());

  // TODO(jeffbrown): Set clip.
  // It's worth asking whether all children should be clipped implicitly
  // or whether we should leave this up to the Flutter application to decide.
  // In some situations, it might be useful to allow children to draw
  // outside of their layout bounds.
  //
  // float inverse_device_pixel_ratio = 1.f / device_pixel_ratio_;
  // SkRect bounds =
  //     SkRect::MakeXYWH(offset_.x(), offset_.y(),
  //                      physical_size_.width() * inverse_device_pixel_ratio,
  //                      physical_size_.height() * inverse_device_pixel_ratio);
  if (export_node_) {
    export_node_->Bind(context, container, offset_, 1.f / device_pixel_ratio_,
                       hit_testable_);
  }
}

}  // namespace flow
