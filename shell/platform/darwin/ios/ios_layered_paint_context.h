// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_LAYERED_PAINT_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_LAYERED_PAINT_CONTEXT_H_

#import <QuartzCore/CAEAGLLayer.h>
#include "flutter/flow/layered_paint_context.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/platform/darwin/ios/ios_gl_context.h"

namespace shell {

class IOSSurface;
class IOSSurfaceGL;
class GPUSurfaceGL;

class IOSLayeredPaintContext : public flow::LayeredPaintContext {
 public:
  explicit IOSLayeredPaintContext(PlatformView::SurfaceConfig surface_config,
      CALayer* layer);
  void Reset() override;
  void Finish() override;
  void PushLayer(SkRect bounds) override;
 // void PushFrame(SkRect bounds) override;
  void PopLayer() override;
  SkCanvas *CurrentCanvas() override;
  void ClipRect() override;
  void Elevate(float elevation) override;
  void SetColor(SkColor color) override;
  void SetClipPath(SkPath path) override;
  void AddExternalLayer(flow::Texture* texture, SkRect bounds) override;
  void ExecutePaintTasks(flow::CompositorContext::ScopedFrame& frame) override;
  void AddPaintedLayer(flow::Layer *layer) override;
  void Transform(SkMatrix transform) override;
  void PopTransform() override;
  SkRect SystemCompositedRect() override;
  IOSSurface *rootIOSSurface() { return root_surface_.get(); }
 private:
  struct Surface {
    Surface(CAEAGLLayer *caLayer, IOSSurfaceGL *iosSurface);
    CAEAGLLayer *caLayer;
    IOSSurfaceGL *iosSurface;
    std::unique_ptr<GPUSurfaceGL> gpuSurface;
  };

  // class ExternalLayer {
  //  public:
  //   size_t height;

  // }

  class Layer {
   public:
    explicit Layer();
    bool makeCurrent();
    CALayer *layer();
    void present();
    SkCanvas *canvas();
    void AddPaintedLayer(flow::Layer *layer);
    void installChildren();
    void manifest(IOSLayeredPaintContext &context);
    
    std::vector<flow::Layer*> paint_layers_;
    SkColor background_color_;
    float corner_radius_ = 0.0;
    bool clip_ = false;
    float elevation_ = 0.0f;

    int id;
    std::vector<std::unique_ptr<Layer>> children_;
    SkRect frame_;
    std::vector<size_t> externalLayerHeights;
    std::vector<CALayer*> externalLayers;
    std::vector<SkRect> externalLayerFrames;
    std::vector<SkMatrix> externalLayerTransforms;
    Surface *surface;
    CALayer *layer_;
    SkMatrix transform;
    SkPoint offset;
    SkPath path;
   private:
    SkCanvas *canvas_ = nullptr;
  };

  Layer *acquireLayer();
  CALayer *basicLayer();
  Surface *drawLayer();
  
  SkPoint current_offset();
  CAEAGLLayer *alloc_layer();
  CAEAGLLayer *bottomLayer;
  std::vector<Layer*> stack_;
  std::vector<SkPoint> offsets_;

  // TODO(sigurdm): Make a proper cache of these layers indexed by size and deallocating after aging.
  // Similar to vulkan_surface_pool.
  std::vector<CALayer*> CALayerCache_;
  std::vector<Surface*> CAEAGLLayerCache_;
  size_t CALayerCache_index_;
  size_t CAEAGLLayerCache_index_;

  std::unique_ptr<Layer> root_layer_;

  std::vector<Layer*> paint_tasks_;
  std::vector<SkMatrix> transforms_;
  SkRect system_composited_rect_;
  fml::scoped_nsobject<EAGLContext> eaglContext_;
  std::unique_ptr<IOSSurface> root_surface_;

  PlatformView::SurfaceConfig surface_config_;
};

} // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_LAYERED_PAINT_CONTEXT_H_
