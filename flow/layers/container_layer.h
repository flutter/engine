// Copyright 2015 The Chromium Authors. All rights reserved.
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

  void Add(std::unique_ptr<Layer> layer);

  // Begin hole management.
  //
  // Hole management is concerned with punching holes through the back of the
  // Flutter UI to accommodate HoleLayers. The intention is that pixels
  // drawn by other platform views placed below the Flutter view should be
  // visible through HoleLayers.
  //
  // Hole punching is achieved by restructuring the layer tree at and above the
  // HoleLayer insertion point. Adding a HoleLayer involves following ancestral
  // ContainerLayers up to the root and inserting a suitably clipped or
  // transformed HoleLayer as left sibling of each ancestor.
  //
  // Layers whose Paint method involves internal layering (like painting
  // backgrounds and shadows before painting children) need to implement
  // their own hole punching of those backgrounds internally.

  // Adds a hole layer to this container.
  virtual void AddHole(std::unique_ptr<Layer> hole);

  // Punches a hole through the specified ancestor by inserting the specified
  // hole, suitably decorated with clips and transforms, as a child of the
  // ancestor immediately to the left of the ancestral chain of this container.
  virtual void PunchHoleIn(ContainerLayer* ancestor, std::unique_ptr<Layer> hole);

  // Decorates the specified hole, as necessary, to reflect any clipping or
  // transformation performed by this container. Template method used by
  // the default PunchHoleIn implementation.
  virtual std::unique_ptr<Layer> WrapHoleForAncestor(std::unique_ptr<Layer> hole) { return hole; }
  // End hole management.

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)

  const std::vector<std::unique_ptr<Layer>>& layers() const { return layers_; }

 protected:
  void DefaultPunchHoleIn(ContainerLayer* ancestor, std::unique_ptr<Layer> hole);
  void PrerollChildren(PrerollContext* context,
                       const SkMatrix& child_matrix,
                       SkRect* child_paint_bounds);
  void PaintChildren(PaintContext& context) const;

#if defined(OS_FUCHSIA)
  void UpdateSceneChildren(SceneUpdateContext& context);
#endif  // defined(OS_FUCHSIA)

 private:
  std::vector<std::unique_ptr<Layer>> layers_;

  FXL_DISALLOW_COPY_AND_ASSIGN(ContainerLayer);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
