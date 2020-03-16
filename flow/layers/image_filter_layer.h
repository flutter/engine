// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_IMAGE_FILTER_LAYER_H_
#define FLUTTER_FLOW_LAYERS_IMAGE_FILTER_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

#include "third_party/skia/include/core/SkImageFilter.h"

namespace flutter {

class ImageFilterLayer : public MergedContainerLayer {
 public:
  ImageFilterLayer(sk_sp<SkImageFilter> filter);

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

 private:
  // The default minimum number of times to render a filtered layer before
  // we cache the output of the filter. Note that until this limit is
  // reached we may continue to cache the children anyway.
  static constexpr int kMinimumRendersBeforeCachingFilterLayer = 3;

  sk_sp<SkImageFilter> filter_;
  SkRect child_paint_bounds_;
  int render_count_;

  FML_DISALLOW_COPY_AND_ASSIGN(ImageFilterLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_IMAGE_FILTER_LAYER_H_
