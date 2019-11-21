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

  void Add(std::shared_ptr<Layer> layer);

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)

  const std::vector<std::shared_ptr<Layer>>& layers() const { return layers_; }

  void update_child_readback(Layer* layer);

 protected:
  void PrerollChildren(PrerollContext* context,
                       const SkMatrix& child_matrix,
                       SkRect* child_paint_bounds);
  void PaintChildren(PaintContext& context) const;

  virtual bool compute_tree_reads_surface() override;

#if defined(OS_FUCHSIA)
  void UpdateSceneChildren(SceneUpdateContext& context);
#endif  // defined(OS_FUCHSIA)

  void set_renders_to_save_layer(bool protects) {
    if (renders_to_save_layer_ != protects) {
      renders_to_save_layer_ = protects;
      update_screen_readback();
    }
  }

  // For OpacityLayer to restructure to have a single child.
  void ClearChildren() {
    layers_.clear();
    if (children_need_screen_readback_ > 0) {
      children_need_screen_readback_ = 0;
      update_screen_readback();
    }
  }

 private:
  std::vector<std::shared_ptr<Layer>> layers_;

  int children_need_screen_readback_;
  bool renders_to_save_layer_;

  FML_DISALLOW_COPY_AND_ASSIGN(ContainerLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
