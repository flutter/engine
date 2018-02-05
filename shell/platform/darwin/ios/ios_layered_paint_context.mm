// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_layered_paint_context.h"
#include "flutter/shell/platform/darwin/ios/ios_external_texture_layer.h"
#include "flutter/shell/platform/darwin/ios/ios_surface_gl.h"

#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

#include <UIKit/UIKit.h>

namespace shell {

IOSLayeredPaintContext::Layer::Layer(IOSSurfaceGL *surface) :
  iosSurface_(surface), gpuSurface_(static_cast<std::unique_ptr<GPUSurfaceGL>>(iosSurface_->CreateGPUSurface())) {
  FXL_LOG(INFO) << "Creating layer";
}

SkCanvas *IOSLayeredPaintContext::Layer::canvas() {
//  if (canvas_ == nullptr) {
    CGSize size = layer().frame.size;
    FXL_LOG(INFO) << "Acquiring canvas " << size.width << "x" << size.height;
    canvas_ = gpuSurface_->AcquireRenderSurface(SkISize::Make(size.width, size.height))->getCanvas();
    canvas_->clear(SK_ColorBLUE);
 // }
 // FXL_LOG(INFO) << "Acquiring canvas";
  return canvas_;
}

void IOSLayeredPaintContext::Layer::present() {
//   FXL_LOG(INFO) << "bounds " << [layer() bounds] == nil << " frame " << [layer() frame]  == nil;
   canvas_->flush();
   iosSurface_->GLContextPresent();
}

CAEAGLLayer *IOSLayeredPaintContext::Layer::layer() { return iosSurface_->layer(); }

bool IOSLayeredPaintContext::Layer::makeCurrent() { return gpuSurface_->MakeCurrent(); }
bool IOSLayeredPaintContext::Layer::makeCurrent2() { return gpuSurface_->MakeCurrent2(); }

void  IOSLayeredPaintContext::Layer::AddPaintedLayer(flow::Layer *layer) {
  paint_layers_.push_back(layer);
}

IOSLayeredPaintContext::IOSLayeredPaintContext(IOSSurface *rootSurface) {
  stack_.push_back(new Layer(reinterpret_cast<IOSSurfaceGL*>(rootSurface)));

  offsets_.push_back(SkPoint::Make(0, 0));
  transforms_.push_back(SkMatrix::Concat(SkMatrix::I(), SkMatrix::I()));
// root_surface_ = reinterpret_cast<IOSSurfaceGL*>(rootSurface);
}

void IOSLayeredPaintContext::Reset() {
  cache_index_ = 0;
  NSLog(@"Begin");
  [CATransaction begin];
  [CATransaction setValue:(id)kCFBooleanTrue
                   forKey:kCATransactionDisableActions];
}

void IOSLayeredPaintContext::Finish() {
  for (size_t i = cache_index_; i < cache_.size(); i++) {
    [cache_[i]->layer() removeFromSuperlayer];
  }

  NSLog(@"Finishing %lu",stack_.size());

  [CATransaction commit];
}

void IOSLayeredPaintContext::PushLayer(SkRect bounds) {
  const PlatformView::SurfaceConfig surface_config = {
   .red_bits = 8,
   .green_bits = 8,
   .blue_bits = 8,
   .alpha_bits = 8,
   .depth_bits = 0,
   .stencil_bits = 8,
 };
 std::stringstream indent;
 for (uint i = 0; i < stack_.size(); i++) {
   indent << "  ";
 }
  FXL_LOG(INFO) << indent.str() << "push" << bounds.x() << ", " << bounds.y() << " " << bounds.width() << " " << bounds.height();
  Layer *newL;
  CAEAGLLayer *newLayer;
  if (cache_index_ >= cache_.size()) {
      NSLog(@"No reuse");
      newLayer = [[CAEAGLLayer alloc] init];
      newL = new Layer(new IOSSurfaceGL(surface_config, newLayer));
      cache_.push_back(newL);
  } else {
      NSLog(@"Reuse");
      newL = cache_[cache_index_];
      newL->paint_layers_.clear();
      newLayer = newL->layer();
  }
  cache_index_++;

  newLayer.frame = CGRectMake(bounds.x() - current_offset().x() + 20, bounds.y() - current_offset().y(), bounds.width(), bounds.height());
  FXL_LOG(INFO) << indent.str() << "frame" << bounds.x() - current_offset().x() << ", " << bounds.y() - current_offset().y();
  
  newLayer.masksToBounds = false;
  newLayer.zPosition = 0.0;
  newLayer.cornerRadius = 0.0;
  newLayer.borderWidth = 2.0;
  newLayer.borderColor = [UIColor purpleColor].CGColor;
  newLayer.backgroundColor = [UIColor clearColor].CGColor;
  newLayer.opaque = NO;
  newLayer.opacity = 1;
  newLayer.presentsWithTransaction = YES;
 // newLayer.backgroundColor = [UIColor colorWithWhite:1 / stack_.size() alpha:0.0].CGColor;

  if ([newLayer superlayer] != CurrentLayer()) {
    [CurrentLayer() addSublayer:newLayer];
  }

  stack_.push_back(newL);
  offsets_.push_back(SkPoint::Make(bounds.x(), bounds.y()));
}

void IOSLayeredPaintContext::PopLayer() {
  Layer* top = stack_.back();
  SkPoint top_offset = offsets_.back();
  if (!top->paint_layers_.empty()) {
    paint_tasks_.push_back({
    .layer = top,
    .transform = transforms_.back(),
    .left = top_offset.x(),
    .top  = top_offset.y(),
    .background_color = SK_ColorBLUE,
    .layers = top->paint_layers_,
  });
  }
  stack_.pop_back();
  offsets_.pop_back(); std::stringstream indent;
  for (uint i = 0; i < stack_.size(); i++) {
    indent << "  ";
  }
  FXL_LOG(INFO) << indent.str() << "pop";

}

SkPoint IOSLayeredPaintContext::current_offset() { return offsets_.back(); }

SkCanvas *IOSLayeredPaintContext::CurrentCanvas() {
  return stack_.back()->canvas();
}

void IOSLayeredPaintContext::MakeTopLayerCurrent() {
  stack_.back()->makeCurrent();
}

CAEAGLLayer *IOSLayeredPaintContext::CurrentLayer() {
  return stack_.back()->layer();
}

void IOSLayeredPaintContext::AddExternalLayer(flow::Texture* texture, SkRect frame) {
  CALayer *externalLayer = (static_cast<IOSExternalTextureLayer*>(texture))->layer();
  if (externalLayer.superlayer != CurrentLayer()) {
    FXL_DLOG(INFO) << "Inserting layer" << [[externalLayer description] cStringUsingEncoding: [NSString defaultCStringEncoding]] << frame.x() << " " << frame.y();
      [CurrentLayer() addSublayer: externalLayer];
        CGFloat screenScale = [UIScreen mainScreen].scale;
        externalLayer.contentsScale = screenScale;
  }
  CGRect newFrame = CGRectMake(frame.x() - current_offset().x(), frame.y() - current_offset().y(), frame.width(), frame.height());
  if (externalLayer.frame.origin.x != newFrame.origin.x ||
    externalLayer.frame.origin.y != newFrame.origin.y ||
    externalLayer.frame.size.width != newFrame.size.width ||
    externalLayer.frame.size.height != newFrame.size.height) {
    externalLayer.frame = newFrame;
     FXL_DLOG(INFO) << "New frame";
  }
}

void IOSLayeredPaintContext::SetColor(SkColor color) {
  CurrentLayer().backgroundColor = [UIColor colorWithRed: SkColorGetR(color) green: SkColorGetG(color) blue: SkColorGetB(color) alpha: SkColorGetA(color)].CGColor;
}

void IOSLayeredPaintContext::SetCornerRadius(float radius) {
  CurrentLayer().cornerRadius = radius;

}

void IOSLayeredPaintContext::ClipRect() {
  CurrentLayer().masksToBounds = true;
}

void IOSLayeredPaintContext::Elevate(float elevation) {
  CurrentLayer().zPosition = elevation;
}

  void IOSLayeredPaintContext::ExecutePaintTasks(flow::CompositorContext::ScopedFrame& frame) {
      NSLog(@"Execute %lu", paint_tasks_.size());
  int i = 0;
  // SkColor colors[] = {
  // SK_ColorRED,
  // SK_ColorYELLOW,
  // SK_ColorBLUE, 
  // SK_ColorWHITE,
  // SK_ColorGREEN,
  // };
  for (auto& task : paint_tasks_) {
    FXL_DCHECK(task.layer);
    task.layer->makeCurrent();
    SkCanvas* canvas = task.layer->canvas();
    flow::Layer::PaintContext context = {
      *canvas,
                frame.context().frame_time(),
                frame.context().engine_time(),
                frame.context().memory_usage(),
                frame.context().texture_registry(),
                false,};
    //canvas->restoreToCount(1);
    int saveCount = canvas->save();
    canvas->clear(SK_ColorTRANSPARENT);
//    canvas->scale(task.scale_x, task.scale_y);

    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    context.canvas.scale(1 / task.layer->layer().contentsScale, 1 / task.layer->layer().contentsScale);
  //  context.canvas.concat(task.transform);
    canvas->translate(-task.left, -task.top);
    NSLog(@"Painting %lu %f %f", task.layers.size(), task.left, task.top);
    paint.setStyle(SkPaint::kStroke_Style);
    std::stringstream a;
    a << "hej " << i;
        canvas->drawString(a.str().c_str(), task.left + 2, task.top + 20, paint);
    for (flow::Layer* layer : task.layers) {
       NSLog(@"Bounds %f %f %f %f", layer->paint_bounds().x(), layer->paint_bounds().y(), layer->paint_bounds().width(), layer->paint_bounds().height());
      canvas->drawRect(layer->paint_bounds().makeInset(3.0, 3.0), paint);
      layer->Paint(context);
    }
    canvas->drawString(a.str().c_str(), task.left + 2, task.top + 10, paint);
    canvas->restoreToCount(saveCount);
    task.layers.clear();
    task.layer->present();
    i ++;
  }
  paint_tasks_.clear();
}

void IOSLayeredPaintContext::Transform(SkMatrix transform) {
  transforms_.push_back(SkMatrix::Concat(transforms_.back(), transform));
}

void IOSLayeredPaintContext::PopTransform() {
  transforms_.pop_back();
}

void IOSLayeredPaintContext::AddPaintedLayer(flow::Layer *layer) {
  stack_.back()->AddPaintedLayer(layer);
}


} // namespace shell
