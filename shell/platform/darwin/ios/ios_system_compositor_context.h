// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SYSTEM_COMPOSITOR_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SYSTEM_COMPOSITOR_CONTEXT_H_

#import <QuartzCore/CAEAGLLayer.h>
#include "flutter/flow/system_compositor_context.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/platform/darwin/ios/ios_gl_context.h"

namespace shell {

class IOSSurface;
class IOSSurfaceGL;
class GPUSurfaceGL;

class IOSSystemCompositorContext : public flow::SystemCompositorContext {
 public:
  explicit IOSSystemCompositorContext(PlatformView::SurfaceConfig surface_config,
      CALayer* layer);
  void Reset() override;
  void Finish() override;
  void PushLayer(SkRect bounds) override;
  void PopLayer() override;
  SkCanvas *CurrentCanvas() override;
  void ClipFrame() override;
  void SetColor(SkColor color) override;
  void SetClipPath(SkPath path) override;
  void AddExternalLayer(flow::Texture* texture, SkRect bounds) override;
  void ExecutePaintTasks(flow::CompositorContext::ScopedFrame& frame) override;
  void AddPaintedLayer(flow::Layer *layer) override;
  void Transform(SkMatrix transform) override;
  void PopTransform() override;
  IOSSurface *rootIOSSurface() { return root_surface_.get(); }
 private:

  // TODO(sigurdm): Consider replacing this with just an IOSSurface.
  struct Surface {
    Surface(CALayer *caLayer, IOSSurfaceGL *iosSurface);
    CALayer *caLayer;
    IOSSurfaceGL *iosSurface;
    std::unique_ptr<GPUSurfaceGL> gpuSurface;
  };

  class CompositingLayer {
   public:
    virtual ~CompositingLayer() = default;
    // Set up the child hierarchy of CALayers.
    virtual void installChildren() = 0;

    // Update the backing CALayer properties to reflect `this`.
    virtual void manifest(IOSSystemCompositorContext &context) = 0;
    CALayer *layer();

    fml::scoped_nsobject<CALayer> layer_;
    SkMatrix transform_;
    SkPoint offset_;
    SkRect frame_;
  };

  class ExternalCompositingLayer : public CompositingLayer {
   public:
    ExternalCompositingLayer(CALayer *externalLayer);
    ~ExternalCompositingLayer() override = default;
    void installChildren() override;
    void manifest(IOSSystemCompositorContext &context) override;
  };

  class FlowCompositingLayer : public CompositingLayer  {
   public:
    FlowCompositingLayer();
    ~FlowCompositingLayer() override = default; 
    bool makeCurrent();
    void present();
    SkCanvas *canvas();
    void AddPaintedLayer(flow::Layer *layer);
    void installChildren() override;
    void manifest(IOSSystemCompositorContext &context) override;
    
    std::vector<flow::Layer*> paint_layers_;
    SkColor background_color_;
    float corner_radius_ = 0.0;
    bool clip_ = false;
    std::vector<std::unique_ptr<CompositingLayer>> children_;
    Surface *surface_;
    SkPath path_;
   private:
    SkCanvas *canvas_;
  };

  CALayer *createBasicLayer();
  Surface *createDrawLayer();

  SkPoint currentOffset();
  std::vector<FlowCompositingLayer*> stack_;
  std::vector<SkPoint> offsets_;

  // TODO(sigurdm): Make a proper cache of these layers indexed by size and deallocating after aging.
  // Like  vulkan_surface_pool.
  std::vector<CALayer*> CALayerCache_;
  std::vector<Surface*> CAEAGLLayerCache_;
  size_t CALayerCache_index_;
  size_t CAEAGLLayerCache_index_;

  std::unique_ptr<FlowCompositingLayer> root_layer_;

  std::vector<FlowCompositingLayer*> paint_tasks_;
  std::vector<SkMatrix> transforms_;
  fml::scoped_nsobject<EAGLContext> eaglContext_;

  std::unique_ptr<IOSSurface> root_surface_;

  PlatformView::SurfaceConfig surface_config_;
};

} // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SYSTEM_COMPOSITOR_CONTEXT_H_
