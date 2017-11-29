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

  // Adds a rectangular hole to this layer and punches a suitably clipped and/or
  // transformed hole through all ancestors of this layer. The intention is to
  // make visible any platform views placed below the Flutter view, so a hole
  // fills its allotted canvas area using a transparent color and paint mode
  // SkBlendMode::kSrc. As each parent layer above the hole may involve painting
  // its children to a separate bitmap and blending that in using
  // SkBlendMode::kSrcOver, we need to do extra work at each such layer. Rather
  // than adding hole-related logic to every layer, this is done by tree
  // surgery, ensuring that the main canvas at each level already has
  // transparent pixels in the hole when the childrens' pixels are blended in.
  //
  // Consider a layer tree like the following where the intention is to make
  // H a hole, so that underlying views are visible through R, A, B, D, E,
  // and G, while any painting done by I, F, and C should be visible on top of
  // the hole.
  //
  // R
  // +--A
  // +--B
  // |  +--D
  // |  +--E
  // |  |  +--G
  // |  |  +--H
  // |  |  +--I
  // |  +--F
  // +--C
  //
  // During tree building, we call E.AddHole(...) at a point where I, F, and C
  // have not yet been added. That operation transforms the layer tree into
  //
  // R
  // +--A
  // +--B(E(H))
  // +--B
  //    +--D
  //    +--E(H)
  //    +--E
  //       +--G
  //       +--H
  //
  // Here, E(H) is a layer that wraps a copy of H in whatever clip and/or
  // transformation is implemented by E (that may be the identity operation).
  // Similarly, B(E(H)) further wraps to take account of clip or transformation
  // done by layer B.
  //
  // Layers whose Paint method involves internal layering (like painting
  // backgrounds and shadows before painting children) need to implement
  // their own hole punching of those backgrounds.
  virtual void AddHole(const SkPoint& offset, const SkSize& size);

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)

  const std::vector<std::unique_ptr<Layer>>& layers() const { return layers_; }

 protected:
  // Punches a hole through the specified ancestor of this layer. The given hole
  // layer is wrapped in any clips and transforms done by this layer and its
  // parents before being inserted into the ancestor immediately to the left of
  // the ancestral chain of this layer.
  virtual void PunchHoleIn(ContainerLayer* ancestor, std::unique_ptr<Layer> hole);

  // Wraps the specified hole layer as necessary to reflect any clipping or
  // transformation performed by this layer. Template method used by the default
  // PunchHoleIn implementation.
  virtual std::unique_ptr<Layer> WrapHoleForAncestor(std::unique_ptr<Layer> hole) { return hole; }

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
