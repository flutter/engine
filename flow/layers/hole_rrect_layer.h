// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_HOLE_RRECT_LAYER_H_
#define FLUTTER_FLOW_LAYERS_HOLE_RRECT_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

namespace flow {

class HoleRRectLayer : public ContainerLayer {
 public:
  HoleRRectLayer();
  ~HoleRRectLayer() override;

  void set_clip_rrect(const SkRRect& clip_rrect) { clip_rrect_ = clip_rrect; }
  
  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

 private:
  SkRRect clip_rrect_;

  FXL_DISALLOW_COPY_AND_ASSIGN(HoleRRectLayer);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_LAYERS_HOLE_RRECT_LAYER_H_
