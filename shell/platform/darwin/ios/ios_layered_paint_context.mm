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

static int count = 0;

static CATransform3D skiaToCATransform(SkMatrix transform) {
  CATransform3D result;
  result.m11 = transform[0];
  result.m12 = transform[1];
  result.m13 = transform[2];
  result.m14 = 0.0;

  result.m21 = transform[3];
  result.m22 = transform[4];
  result.m23 = transform[5];
  result.m24 = 0.0;

  result.m31 = transform[6];
  result.m32 = transform[7];
  result.m33 = transform[8];
  result.m34 = 0.0;

  result.m41 = 0.0;
  result.m42 = 0.0;
  result.m43 = 0.0;
  result.m44 = 1.0;
  return result;
} 

IOSLayeredPaintContext::Surface::Surface(CAEAGLLayer *caLayer, IOSSurfaceGL *iosSurface) :
  caLayer(caLayer), iosSurface(iosSurface), gpuSurface(iosSurface->CreateGPUSurface()) {}

IOSLayeredPaintContext::Layer::Layer() :
  background_color_(SK_ColorTRANSPARENT),
  id(count++) { }

SkCanvas *IOSLayeredPaintContext::Layer::canvas() {
  CGSize size = layer().frame.size;
  canvas_ = surface->gpuSurface->AcquireRenderSurface(SkISize::Make(size.width * 2, size.height * 2))->getCanvas();
  return canvas_;
}

void IOSLayeredPaintContext::Layer::present() {
   canvas_->flush();
   surface->iosSurface->GLContextPresent();
}

void IOSLayeredPaintContext::Layer::makePainted(Surface *surface) {
  this->surface = surface;
  layer_ = surface->caLayer;
}

void IOSLayeredPaintContext::Layer::makeBasic(CALayer *layer) {
  layer_ = layer;
}

void IOSLayeredPaintContext::Layer::manifest() {
  int alpha = SkColorGetA(background_color_);
  layer_.backgroundColor = [UIColor colorWithRed: SkColorGetR(background_color_) green: SkColorGetG(background_color_) blue: SkColorGetB(background_color_) alpha: alpha].CGColor;
  if (alpha == 0xff) {
    layer_.opaque = true;
  } else {
    layer_.opaque = NO;
  }

  layer_.opacity = 1;

  if (layer_.masksToBounds != clip_) {
    layer_.masksToBounds = clip_;
  }

  if (layer_.cornerRadius != corner_radius_) {
    layer_.cornerRadius =  corner_radius_;
  }

  if (elevation_ == 0.0f) {
    layer_.shadowOpacity = 0.0;
  } else {
     layer_.shadowOpacity = 0.3;
     layer_.opaque = YES; /// XXX
     // CurrentLayer().shadowPath = CGPathCreateWithRect(CurrentLayer().bounds, nullptr);
     layer_.shadowRadius = 2.0;
     layer_.shadowOffset = CGSizeMake(0 , 2);
     layer_.borderWidth = 0.0;
  }

  CGRect newFrame = CGRectMake(frame_.x(), frame_.y(), frame_.width(), frame_.height());
  if (!CGRectEqualToRect(layer_.frame, newFrame)) {
    layer_.frame = newFrame;
  }

  for (size_t i = 0; i < externalLayers.size(); i++) {
    CALayer *externalLayer = externalLayers[i];
    SkRect externalFrame = externalLayerFrames[i];
    if (externalLayer.superlayer != layer_) {
      [layer_ addSublayer: externalLayer];
         CGFloat screenScale = [UIScreen mainScreen].scale;
         externalLayer.contentsScale = screenScale;
    }
    CGRect newFrame = CGRectMake(externalFrame.x(), externalFrame.y(), externalFrame.width(), externalFrame.height());
  
    if (!CGRectEqualToRect(externalLayer.frame, newFrame)) {
      externalLayer.frame = newFrame;
      FXL_DLOG(INFO) << "New frame";
    }
  }
  externalLayers.clear();
  externalLayerFrames.clear();
}

CALayer *IOSLayeredPaintContext::Layer::layer() { return layer_; }

bool IOSLayeredPaintContext::Layer::makeCurrent() { return surface->gpuSurface->MakeCurrent(); }

void  IOSLayeredPaintContext::Layer::AddPaintedLayer(flow::Layer *layer) {
  paint_layers_.push_back(layer);
}


IOSLayeredPaintContext::IOSLayeredPaintContext(IOSSurface *rootSurface) {
  IOSSurfaceGL *rootSurfaceGL = static_cast<IOSSurfaceGL*>(rootSurface);
  Layer *rootLayer = new Layer();

  rootLayer->surface = new Surface(rootSurfaceGL->layer(), rootSurfaceGL);
  rootLayer->layer_ = rootSurfaceGL->layer();

  stack_.push_back(rootLayer);

  offsets_.push_back(SkPoint::Make(0, 0));
  transforms_.push_back(SkMatrix::Concat(SkMatrix::I(), SkMatrix::I()));
// root_surface_ = reinterpret_cast<IOSSurfaceGL*>(rootSurface);
}

CALayer *IOSLayeredPaintContext::basicLayer() {
  CALayer *result;
  if (CALayerCache_index_ >= CALayerCache_.size()) {
    result = [[CALayer alloc] init];
    CALayerCache_.push_back(result);
  } else {
    result = CALayerCache_[CALayerCache_index_];
     //       [result removeFromSuperlayer];
  }
  CALayerCache_index_++;
  return result;
}

IOSLayeredPaintContext::Surface *IOSLayeredPaintContext::drawLayer() {
  Surface *result;
  if (CAEAGLLayerCache_index_ >= CAEAGLLayerCache_.size()) {
    CAEAGLLayer *newCALayer = [[CAEAGLLayer alloc] init];
    CGFloat screenScale = [UIScreen mainScreen].scale;
    newCALayer.contentsScale = screenScale;
    newCALayer.rasterizationScale = screenScale;
    newCALayer.presentsWithTransaction = YES;
    const PlatformView::SurfaceConfig surface_config = {
      .red_bits = 8,
      .green_bits = 8,
      .blue_bits = 8,
      .alpha_bits = 8,
      .depth_bits = 0,
      .stencil_bits = 8,
    };
    IOSSurfaceGL *iosSurface = new IOSSurfaceGL(surface_config, newCALayer);
    result = new Surface(newCALayer, iosSurface);
    CAEAGLLayerCache_.push_back(result);
  } else {
    result = CAEAGLLayerCache_[CAEAGLLayerCache_index_];
    //  [result->caLayer removeFromSuperlayer];

  }
  CAEAGLLayerCache_index_++;
  return result;
}

void IOSLayeredPaintContext::Reset() {
  system_composited_rect_ = SkRect::MakeEmpty();
  cache_index_ = 0;
  CALayerCache_index_ = 0;
  CAEAGLLayerCache_index_ = 0;
  NSLog(@"Begin");
  [CATransaction begin];
  [CATransaction setValue:(id)kCFBooleanTrue
                   forKey:kCATransactionDisableActions];
}

void IOSLayeredPaintContext::Finish() {
  for (size_t i = CALayerCache_index_; i < CALayerCache_.size(); i++) {
    [CALayerCache_[i] removeFromSuperlayer];
  }
  for (size_t i = CAEAGLLayerCache_index_; i < CAEAGLLayerCache_.size(); i++) {
    [CAEAGLLayerCache_[i]->caLayer removeFromSuperlayer];
  }

  NSLog(@"Finishing %lu",stack_.size());

  [CATransaction commit];
}

 IOSLayeredPaintContext::Layer *IOSLayeredPaintContext::acquireLayer() {
 Layer *result;
 if (cache_index_ >= cache_.size()) {
     result = new Layer();
     cache_.push_back(result);
  } else {
      result = cache_[cache_index_];
//      [newCALayer setAffineTransform: CGAffineTransformIdentity];
  }
  cache_index_++;
  return result;
}

void IOSLayeredPaintContext::PushLayer(SkRect bounds) {

//  std::stringstream indent;
//  for (uint i = 0; i < stack_.size(); i++) {
//    indent << "  ";
//  }
//   FXL_LOG(INFO) << indent.str() << "push" << bounds.x() << ", " << bounds.y() << " " << bounds.width() << " " << bounds.height();
  Layer *newLayer = acquireLayer();

  SkRect newBounds = bounds.makeOffset(-current_offset().x(), -current_offset().y());
  
  transforms_.back().mapRect(&newBounds);
  newLayer->frame_ = newBounds;
  // FXL_DLOG(INFO) << indent.str() << "frame " << newBounds.x() << ", " << newBounds.y();

  if (! stack_.empty()) {
    newLayer->parent_ = stack_.back();
  }

  stack_.push_back(newLayer);
  offsets_.push_back(SkPoint::Make(bounds.x(), bounds.y()));
}

void IOSLayeredPaintContext::PopLayer() {
  Layer* top = stack_.back();
  SkPoint top_offset = offsets_.back();
 // if (!top->paint_layers_.empty()) {
    paint_tasks_.push_back({
    .layer = top,
    .transform = transforms_.back(),
    .left = top_offset.x(),
    .top  = top_offset.y(),
    .background_color = top->background_color_,
    .layers = top->paint_layers_,
  });

  if (false || top->paint_layers_.empty()) {
    top->makeBasic(basicLayer());
  } else {
    top->makePainted(drawLayer());
  }

  top->paint_layers_.clear();
 // }
  stack_.pop_back();
  offsets_.pop_back(); 
  // std::stringstream indent;
  // for (uint i = 0; i < stack_.size(); i++) {
  //   indent << "  ";
  // }
  // FXL_LOG(INFO) << indent.str() << "pop";
 // transforms_.pop_back();
}

SkPoint IOSLayeredPaintContext::current_offset() { return offsets_.back(); }

SkCanvas *IOSLayeredPaintContext::CurrentCanvas() {
  return stack_.back()->canvas();
}

SkRect IOSLayeredPaintContext::SystemCompositedRect() {
  return system_composited_rect_;
}

void IOSLayeredPaintContext::AddExternalLayer(flow::Texture* texture, SkRect bounds) {

  CALayer *externalLayer = (static_cast<IOSExternalTextureLayer*>(texture))->layer();
  stack_.back()->externalLayers.push_back(externalLayer);
  
  SkRect newBounds = bounds.makeOffset(-current_offset().x(), -current_offset().y());
  stack_.back()->externalLayerFrames.push_back(newBounds);

  system_composited_rect_.join(newBounds);
}

void IOSLayeredPaintContext::SetColor(SkColor color) {
  stack_.back()->background_color_ = color;
}

void IOSLayeredPaintContext::SetCornerRadius(float radius) {
 // NSLog(@"Setting corner radius to %f", radius);
  stack_.back()->corner_radius_ = radius;
}

void IOSLayeredPaintContext::ClipRect() {
  stack_.back()->clip_ = true;
}

void IOSLayeredPaintContext::Elevate(float elevation) {
 // CurrentLayer().zPosition = elevation;
  stack_.back()->elevation_ = elevation;
//  CurrentLayer().borderColor = [UIColor purpleColor].CGColor;
}

  // SkShadowFlags flags = transparentOccluder
  //                           ? SkShadowFlags::kTransparentOccluder_ShadowFlag
  //                           : SkShadowFlags::kNone_ShadowFlag;
  // const SkRect& bounds = path.getBounds();
  // SkScalar shadow_x = (bounds.left() + bounds.right()) / 2;
  // SkScalar shadow_y = bounds.top() - 600.0f;
  // SkShadowUtils::DrawShadow(canvas, path, dpr * elevation,
  //                           SkPoint3::Make(shadow_x, shadow_y, dpr * 600.0f),
  //                           dpr * 800.0f, 0.039f, 0.25f, color, flags);


  void IOSLayeredPaintContext::ExecutePaintTasks(flow::CompositorContext::ScopedFrame& frame) {
//      NSLog(@"Execute %lu", paint_tasks_.size());
  FXL_DCHECK(glGetError() == GL_NO_ERROR);
  for (auto& task : paint_tasks_) {
    FXL_DCHECK(task.layer);
  //  FXL_DLOG(INFO) << "Layer id: " << task.layer->id;
  //  FXL_DLOG(INFO) << "Layer Size: " << task.layer->layer().frame.size.width << "x" << task.layer->layer().frame.size.height;
    CALayer *parentCALayer = task.layer->parent_->layer();
    CALayer *childCALayer = task.layer->layer();

    if ([childCALayer superlayer] != parentCALayer) {
      [parentCALayer addSublayer:childCALayer];
    }
    task.layer->manifest();
    if (!task.layers.empty()) {
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
      canvas->clear(task.background_color);
      context.canvas.scale(2, 2);
      canvas->translate(-task.left, -task.top);
      context.canvas.concat(task.transform);
      // NSLog(@"Task  %f %f", task.left, task.top);

      // NSLog(@"Canvastransform %f %f %f", task.transform[0], task.transform[1], task.transform[2]);
      // NSLog(@"CanvasTransform %f %f %f", task.transform[3], task.transform[4], task.transform[5]);
      // NSLog(@"CanvasTransform %f %f %f", task.transform[6], task.transform[7], task.transform[8]);

      //    canvas->scale(task.scale_x, task.scale_y);

      SkPaint paint;
      paint.setColor(SK_ColorGREEN);
      paint.setStyle(SkPaint::kStroke_Style);

      for (flow::Layer* layer : task.layers) {
        layer->Paint(context);
      }
      std::stringstream a;
      a << "Layer " << task.layer->id;
      canvas->drawString(a.str().c_str(), task.left + 2, task.top + 20, paint);
      canvas->restoreToCount(saveCount);
      task.layers.clear();
      task.layer->present();
      FXL_DCHECK(glGetError() == GL_NO_ERROR);
    }
  }
  paint_tasks_.clear();
}

void IOSLayeredPaintContext::Transform(SkMatrix transform) {
  // [CurrentLayer() setAffineTransform:
  //  CGAffineTransformMake(
  //    transform[0], transform[1], 
  //    transform[3], transform[4], 
  //    transform[6], transform[7])];
  // CurrentLayer().transform = 
       skiaToCATransform(transform); // XXX
  transforms_.push_back(SkMatrix::Concat(transforms_.back(), transform));
//  SkMatrix currentTransform = transforms_.back();

    // NSLog(@"Pushtransform %f %f %f", transform[0], transform[1], transform[2]);
    // NSLog(@"PushTransform %f %f %f", transform[3], transform[4], transform[5]);
    // NSLog(@"PushTransform %f %f %f", transform[6], transform[7], transform[8]);
}

void IOSLayeredPaintContext::PopTransform() {
  // NSLog(@"Poptransform");
  transforms_.pop_back();
}

void IOSLayeredPaintContext::AddPaintedLayer(flow::Layer *layer) {
//  NSLog(@"Adding painted layer to layer %d", stack_.back()->id);
  stack_.back()->AddPaintedLayer(layer);
}

} // namespace shell
