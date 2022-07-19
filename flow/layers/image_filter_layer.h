// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_IMAGE_FILTER_LAYER_H_
#define FLUTTER_FLOW_LAYERS_IMAGE_FILTER_LAYER_H_

#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/layer.h"
#include "third_party/skia/include/core/SkImageFilter.h"

namespace flutter {

class ImageFilterLayer : public CacheableContainerLayer {
 public:
  explicit ImageFilterLayer(std::shared_ptr<const DlImageFilter> filter);

  void Diff(DiffContext* context, const Layer* old_layer) override;

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

  sk_sp<SkImageFilter> sk_filter() const { return sk_filter_; }

 private:
  sk_sp<SkImageFilter> sk_filter_;
  sk_sp<SkImageFilter> transformed_filter_;
  std::shared_ptr<const DlImageFilter> filter_;
  FML_DISALLOW_COPY_AND_ASSIGN(ImageFilterLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_IMAGE_FILTER_LAYER_H_
