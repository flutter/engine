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
  void MakeTopLayerCurrent() override;
  void PushLayer(SkRect bounds) override;
  void PopLayer() override;
  SkCanvas *CurrentCanvas() override;
  void ClipRect() override;
  void Elevate(float elevation) override;
  void SetColor(SkColor color) override;
  void SetCornerRadius(float radius) override;
  void AddExternalLayer(flow::Texture* texture, SkRect frame) override;
  void ExecutePaintTasks(flow::CompositorContext::ScopedFrame& frame) override;
  void AddPaintedLayer(flow::Layer *layer) override;
  void Transform(SkMatrix transform) override;
  void PopTransform() override;

//  void AddPaintTask(PaintTask task) override;

 private:
  class Layer {
   public:
    explicit Layer(IOSSurfaceGL *surface);
    bool makeCurrent();
    bool makeCurrent2();
    CAEAGLLayer *layer();
    void present();
    SkCanvas *canvas();
    void AddPaintedLayer(flow::Layer *layer);
    
   std::vector<flow::Layer*> paint_layers_;
   private:
   IOSSurfaceGL *iosSurface_;
   std::unique_ptr<GPUSurfaceGL> gpuSurface_;
 //   sk_sp<GrContext> grContext_;
 //   IOSGLContext* context_;
 //   sk_sp<SkSurface> surface_;
   SkCanvas *canvas_ = nullptr;
  };

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

  CAEAGLLayer *CurrentLayer();
  std::vector<Layer*> stack_;
  std::vector<SkPoint> offsets_;
  std::vector<Layer*> cache_;
  size_t cache_index_;
  std::vector<PaintTask> paint_tasks_;
  std::vector<SkMatrix> transforms_;
};

} // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_LAYERED_PAINT_CONTEXT_H_
