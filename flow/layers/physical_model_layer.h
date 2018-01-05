// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_PHYSICAL_MODEL_LAYER_H_
#define FLUTTER_FLOW_LAYERS_PHYSICAL_MODEL_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

namespace flow {

class PhysicalLayerShape;

class PhysicalModelLayer : public ContainerLayer {
 public:
  PhysicalModelLayer() : isRect_(false){};
  ~PhysicalModelLayer() override;

  void set_path(const SkPath& path) {
    path_ = path;
    SkRRect rrect = SkRRect::MakeEmpty();
    if (path.isRRect(&rrect)) {
      isRect_ = rrect.isRect();
    } else {
      isRect_ = false;
    }
#if defined(OS_FUCHSIA)
    if (path.isRRect(&rrect)) {
      frameRRect_ = rrect;
    } else {
      isRect_ = false;
    }
#endif  // defined(OS_FUCHSIA)
  }

  void set_elevation(float elevation) { elevation_ = elevation; }
  void set_color(SkColor color) { color_ = color; }
  void set_device_pixel_ratio(SkScalar dpr) { device_pixel_ratio_ = dpr; }

  static void DrawShadow(SkCanvas* canvas,
                         const SkPath& path,
                         SkColor color,
                         float elevation,
                         bool transparentOccluder,
                         SkScalar dpr);

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)

 private:
  float elevation_;
  SkColor color_;
  SkScalar device_pixel_ratio_;
  SkPath path_;
  bool isRect_;
#if defined(OS_FUCHSIA)
  SkRRect frameRRect_;
#endif  // defined(OS_FUCHSIA)
};

}  // namespace flow

#endif  // FLUTTER_FLOW_LAYERS_PHYSICAL_MODEL_LAYER_H_
