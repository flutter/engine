// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
#define FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_

#include <vector>
#include "flutter/flow/layers/layer.h"

namespace flutter {

class ContainerLayer : public Layer {
 public:
  ContainerLayer();
  ~ContainerLayer() override;

  virtual void Add(std::shared_ptr<Layer> layer);

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;
#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)
  void Paint(PaintContext& context) const override;

  const std::vector<std::shared_ptr<Layer>>& layers() const { return layers_; }

 protected:
  void PrerollChildren(PrerollContext* context,
                       const SkMatrix& child_matrix,
                       SkRect* child_paint_bounds);
  void PaintChildren(PaintContext& context) const;

#if defined(OS_FUCHSIA)
  void UpdateSceneChildren(SceneUpdateContext& context);
#endif  // defined(OS_FUCHSIA)

  // For OpacityLayer to restructure to have a single child.
  void ClearChildren() { layers_.clear(); }

 private:
  std::vector<std::shared_ptr<Layer>> layers_;

  FML_DISALLOW_COPY_AND_ASSIGN(ContainerLayer);
};

class ElevatedContainerLayer : public ContainerLayer {
 public:
  ElevatedContainerLayer(float elevation);
  ~ElevatedContainerLayer() override = default;

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  float elevation() const { return clamped_elevation_; }
  float total_elevation() const {
    return parent_elevation_ + clamped_elevation_;
  }

 private:
  float parent_elevation_ = 0.0f;
  float elevation_ = 0.0f;
  float clamped_elevation_ = 0.0f;

  FML_DISALLOW_COPY_AND_ASSIGN(ElevatedContainerLayer);
};

class FuchsiaSystemCompositedContainerLayer : public ElevatedContainerLayer {
 public:
  static bool should_system_composite() { return true; }

  FuchsiaSystemCompositedContainerLayer(SkColor color, SkAlpha opacity, float elevation);
  ~FuchsiaSystemCompositedContainerLayer() override = default;

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;
#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)
  void Paint(PaintContext& context) const override;

  void set_dimensions(SkRRect rrect) {
    rrect_ = rrect;
  }

  SkColor color() const { return color_; }
  SkAlpha opacity() const { return opacity_; }

 private:
  SkRRect rrect_ = SkRRect::MakeEmpty();
  SkColor color_ = SK_ColorTRANSPARENT;
  SkAlpha opacity_ = 255;

  FML_DISALLOW_COPY_AND_ASSIGN(FuchsiaSystemCompositedContainerLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
