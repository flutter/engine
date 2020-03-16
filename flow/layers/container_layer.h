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

  virtual void Add(std::shared_ptr<Layer> layer);

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;
  void Paint(PaintContext& context) const override;
#if defined(OS_FUCHSIA)
  void CheckForChildLayerBelow(PrerollContext* context) override;
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

  // Try to prepare the raster cache for a given layer.
  //
  // The raster cache would fail if either of the followings is true:
  // 1. The context has a platform view.
  // 2. The context does not have a valid raster cache.
  // 3. The layer's paint bounds does not intersect with the cull rect.
  //
  // We make this a static function instead of a member function that directy
  // uses the "this" pointer as the layer because we sometimes need to raster
  // cache a child layer and one can't access its child's protected method.
  static void TryToPrepareRasterCache(PrerollContext* context,
                                      Layer* layer,
                                      const SkMatrix& matrix);

 private:
  std::vector<std::shared_ptr<Layer>> layers_;

  FML_DISALLOW_COPY_AND_ASSIGN(ContainerLayer);
};

class MergedContainerLayer : public ContainerLayer {
 public:
  MergedContainerLayer();

  void Add(std::shared_ptr<Layer> layer) override;

 protected:
  /**
   * @brief Returns the ContainerLayer used to hold all of the children
   * of the OpacityLayer.
   *
   * Often opacity layers will only have a single child since the associated
   * Flutter widget is specified with only a single child widget pointer.
   * But depending on the structure of the child tree that single widget at
   * the framework level can turn into multiple children at the engine
   * API level since there is no guarantee of a 1:1 correspondence of widgets
   * to engine layers. This synthetic child container layer is established to
   * hold all of the children in a single layer so that we can cache their
   * output, but this synthetic layer will typically not be the best choice
   * for the layer cache since the synthetic container is created fresh with
   * each new OpacityLayer, and so may not be stable from frame to frame.
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
   *
   * Note that since the synthetic child container returned from the
   * GetChildContainer() method is created fresh with each new OpacityLayer,
   * its return value will not be a good candidate for caching. But if the
   * standard recommendations for animations are followed and the child widget
   * is wrapped with a RepaintBoundary widget at the framework level, then
   * the synthetic child container should contain the same single child layer
   * on each frame. Under those conditions, that single child of the child
   * container will be the best candidate for caching in the RasterCache
   * and this method will return that single child if possible to improve
   * the performance of caching the children.
   *
   * Note that if GetCacheableChild() does not find a single stable child of
   * the child container it will return the child container as a fallback.
   * Even though that child is new in each frame of an animation and thus we
   * cannot reuse the cached layer raster between animation frames, the single
   * container child will allow us to paint the child onto an offscreen buffer
   * during Preroll() which reduces one render target switch compared to
   * painting the child on the fly via an AutoSaveLayer in Paint() and thus
   * still improves our performance.
   *
   * @see GetChildContainer()
   * @return the best candidate Layer for caching the children
   */
  Layer* GetCacheableChild() const;

  FML_DISALLOW_COPY_AND_ASSIGN(MergedContainerLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
