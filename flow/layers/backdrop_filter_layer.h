// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_BACKDROP_FILTER_LAYER_H_
#define FLUTTER_FLOW_LAYERS_BACKDROP_FILTER_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

#include "third_party/skia/include/core/SkImageFilter.h"

namespace flutter {

class BackdropFilterLayer : public ContainerLayer {
 public:
  BackdropFilterLayer(sk_sp<SkImageFilter> filter);
  ~BackdropFilterLayer() override;

  void initRetained(std::shared_ptr<Layer> retainedLayer);

  void Paint(PaintContext& context) const override;

 private:
  sk_sp<SkImageFilter> filter_;
  std::shared_ptr<Layer> retainedBackdrop_ = nullptr;

  FML_DISALLOW_COPY_AND_ASSIGN(BackdropFilterLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_BACKDROP_FILTER_LAYER_H_
