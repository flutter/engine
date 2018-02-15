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
  explicit IOSLayeredPaintContext(IOSSurface *context);
  void Reset() override;
  void Finish() override;
  void PushLayer(SkRect bounds) override;
  void PopLayer() override;
  SkCanvas *CurrentCanvas() override;
  void ClipRect() override;
  void Elevate(float elevation) override;
  void SetColor(SkColor color) override;
  void SetCornerRadius(float radius) override;
  void AddExternalLayer(flow::Texture* texture, SkRect bounds) override;
  void ExecutePaintTasks(flow::CompositorContext::ScopedFrame& frame) override;
  void AddPaintedLayer(flow::Layer *layer) override;
  void Transform(SkMatrix transform) override;
  void PopTransform() override;
  SkRect SystemCompositedRect() override;

//  void AddPaintTask(PaintTask task) override;

 private:
  struct Surface {
    Surface(CAEAGLLayer *caLayer, IOSSurfaceGL *iosSurface);
    CAEAGLLayer *caLayer;
    IOSSurfaceGL *iosSurface;
    std::unique_ptr<GPUSurfaceGL> gpuSurface;
  };
  
  class Layer {
   public:
    explicit Layer();
    bool makeCurrent();
    CALayer *layer();
    void present();
    SkCanvas *canvas();
    void AddPaintedLayer(flow::Layer *layer);
    void makePainted(Surface *surface);
    void makeBasic(CALayer *layer);
    
    std::vector<flow::Layer*> paint_layers_;
    SkColor background_color_;
    float corner_radius_;
    bool clip_ = false;
    float elevation_ = 0.0f;

    int id;
    Layer *parent_;
    SkRect frame_;
    std::vector<CALayer*> externalLayers;
    std::vector<SkRect> externalLayerFrames;
    Surface *surface;
    CALayer *layer_;
    void manifest();
   private:
    SkCanvas *canvas_ = nullptr;
  };

  Layer *acquireLayer();
  CALayer *basicLayer();
  Surface *drawLayer();

  struct PaintTask {
    Layer *layer;
    SkScalar left;
    SkScalar top;
    SkMatrix transform;
    SkColor background_color;
    std::vector<flow::Layer*> layers;
  };
  
  SkPoint current_offset();
  CAEAGLLayer *alloc_layer();
  IOSSurfaceGL *root_surface_;
  CAEAGLLayer *bottomLayer;
  std::vector<Layer*> stack_;
  std::vector<SkPoint> offsets_;
  std::vector<Layer*> cache_;

  std::vector<CALayer*> CALayerCache_;
  std::vector<Surface*> CAEAGLLayerCache_;
  size_t CALayerCache_index_;
  size_t CAEAGLLayerCache_index_;

  size_t cache_index_;
  std::vector<PaintTask> paint_tasks_;
  std::vector<SkMatrix> transforms_;
  int height;
  SkRect system_composited_rect_;
};

} // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_LAYERED_PAINT_CONTEXT_H_
