// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_path_layer.h"

namespace flutter {

ClipPathLayer::ClipPathLayer(const DlPath& clip_path, Clip clip_behavior)
    : ClipShapeLayer(clip_path, clip_behavior) {}

const DlFRect ClipPathLayer::clip_shape_bounds() const {
  return clip_shape().Bounds();
}

void ClipPathLayer::ApplyClip(LayerStateStack::MutatorContext& mutator) const {
  mutator.clipPath(clip_shape(), clip_behavior() != Clip::hardEdge);
}

}  // namespace flutter
