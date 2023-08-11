// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rrect_layer.h"

namespace flutter {

ClipRRectLayer::ClipRRectLayer(const DlFRRect& clip_rrect, Clip clip_behavior)
    : ClipShapeLayer(clip_rrect, clip_behavior) {}

const DlFRect ClipRRectLayer::clip_shape_bounds() const {
  return clip_shape().Bounds();
}

void ClipRRectLayer::ApplyClip(LayerStateStack::MutatorContext& mutator) const {
  mutator.clipRRect(clip_shape(), clip_behavior() != Clip::hardEdge);
}

}  // namespace flutter
