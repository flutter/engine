// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_RASTER_TRANSFORM_LAYER_H_
#define FLUTTER_FLOW_LAYERS_RASTER_TRANSFORM_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

namespace flutter {

class RasterTransformLayer : public MergedContainerLayer {
 public:
  RasterTransformLayer(const SkMatrix& transform,
                       const SkV2& raster_scale,
                       const SkSamplingOptions& sampling);

  void Diff(DiffContext* context, const Layer* old_layer) override;

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

 private:
  SkMatrix transform_;
  SkV2 raster_scale_;
  SkSamplingOptions sampling_;

  FML_DISALLOW_COPY_AND_ASSIGN(RasterTransformLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_RASTER_TRANSFORM_LAYER_H_
