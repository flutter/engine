// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_HOLE_LAYER_H_
#define FLUTTER_FLOW_LAYERS_HOLE_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

namespace flow {

class HoleLayer : public Layer {
 public:
  HoleLayer();
  ~HoleLayer() override;

  void set_offset(const SkPoint& offset) { offset_ = offset; }
  void set_size(const SkSize& size) { size_ = size; }

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;
  void Paint(PaintContext& context) const override;

 private:
  SkPoint offset_;
  SkSize size_;

  FXL_DISALLOW_COPY_AND_ASSIGN(HoleLayer);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_LAYERS_HOLE_LAYER_H_
