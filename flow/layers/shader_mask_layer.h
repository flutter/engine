// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_SHADER_MASK_LAYER_H_
#define FLUTTER_FLOW_LAYERS_SHADER_MASK_LAYER_H_

#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/container_layer.h"
#include "third_party/skia/include/core/SkShader.h"

namespace flutter {

class ShaderMaskLayer : public ContainerLayer, public Cacheable {
 public:
  ShaderMaskLayer(sk_sp<SkShader> shader,
                  const SkRect& mask_rect,
                  SkBlendMode blend_mode);

  void Diff(DiffContext* context, const Layer* old_layer) override;

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

  Layer* asLayer() override { return this; };

  void TryToCache(PrerollContext* context, const SkMatrix& ctm) override;

 private:
  sk_sp<SkShader> shader_;
  SkRect mask_rect_;
  SkBlendMode blend_mode_;

  static constexpr int kMinimumRendersBeforeCachingFilterLayer = 3;
  int render_count_;

  FML_DISALLOW_COPY_AND_ASSIGN(ShaderMaskLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_SHADER_MASK_LAYER_H_
