// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
#define FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_

#include <vector>
#include "flutter/flow/layers/layer.h"

namespace flow {

class ContainerLayer : public Layer {
 public:
  ContainerLayer();
  ~ContainerLayer() override;

  void Add(std::shared_ptr<Layer> layer);

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  // Sets the elevation. This needs to be set before preroll because it's then
  // cached by any children of this layer. Setting it after preroll will break
  // their elevation calculations.
  void set_elevation(float elevation) { elevation_ = elevation; }

  // Returns the cumulative height of this layer. Value is computed and cached
  // during preroll.
  float get_cached_total_elevation() const { return cached_total_elevation_; }

#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)

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

  void cacheTotalElevation();

  float elevation_ = 0.0f;
  float cached_total_elevation_ = 0.0f;

 private:
  std::vector<std::shared_ptr<Layer>> layers_;

  FML_DISALLOW_COPY_AND_ASSIGN(ContainerLayer);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
