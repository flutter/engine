// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_OPACITY_LAYER_H_
#define FLUTTER_FLOW_LAYERS_OPACITY_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

namespace flutter {

// Don't add an OpacityLayer with no children to the layer tree. Painting an
// OpacityLayer is very costly due to the saveLayer call. If there's no child,
// having the OpacityLayer or not has the same effect. In debug_unopt build,
// |Preroll| will assert if there are no children.
class OpacityLayer : public ContainerLayer {
 public:
  // An offset is provided here because OpacityLayer.addToScene method in the
  // Flutter framework can take an optional offset argument.
  //
  // By default, that offset is always zero, and all the offsets are handled by
  // some parent TransformLayers. But we allow the offset to be non-zero for
  // backward compatibility. If it's non-zero, the old behavior is to propage
  // that offset to all the leaf layers (e.g., PictureLayer). That will make
  // the retained rendering inefficient as a small offset change could propagate
  // to many leaf layers. Therefore we try to capture that offset here to stop
  // the propagation as repainting the OpacityLayer is expensive.
  OpacityLayer(SkAlpha alpha, const SkPoint& offset);

  void Add(std::shared_ptr<Layer> layer) override;

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)

 private:
  /**
   * @brief Returns the ContainerLayer used to hold all of the children
   * of the OpacityLayer.
   *
   * Typically these layers should only have a single child according to
   * the contract of the associated Flutter widget, but the engine API
   * cannot enforce that contract and must be prepared to support the
   * possibility of adding more than one child. In either case, the
   * child container will hold all children. Note that since the child
   * container is created fresh with each new OpacityLayer, it may not
   * be the best choice to cache to speed up rendering during an animation.
   *
   * @see GetCacheableChild()
   * @return the ContainerLayer child used to hold the children
   */
  ContainerLayer* GetChildContainer() const;

  /**
   * @brief Returns the best choice for a Layer object that can be used
   * in RasterCache operations to cache the children of the OpacityLayer.
   *
   * The returned Layer must represent all children and try to remain stable
   * if the OpacityLayer is reconstructed in subsequent frames of the scene.
   * The OpacityLayer needs to be recreated on frames where its opacity
   * values change, but it often contains the same child on each such frame.
   * Since the child container is created fresh each time the OpacityLayer
   * is constructed, the return value from GetChildContainer will be different
   * on each such frame even if the same child is used. This method will
   * determine if there is a (single) stable child and return that Layer
   * so that caching will be more successful.
   *
   * @see GetChildContainer()
   * @return the best candidate Layer for caching the children
   */
  Layer* GetCacheableChild() const;

  SkAlpha alpha_;
  SkPoint offset_;
  SkRRect frameRRect_;

  FML_DISALLOW_COPY_AND_ASSIGN(OpacityLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_OPACITY_LAYER_H_
