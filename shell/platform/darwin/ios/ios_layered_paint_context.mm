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

// static CATransform3D skiaToCATransform(SkMatrix transform) {
//   CATransform3D result;
//   result.m11 = transform[0];
//   result.m12 = transform[1];
//   result.m13 = transform[2];
//   result.m14 = 0.0;

//   result.m21 = transform[3];
//   result.m22 = transform[4];
//   result.m23 = transform[5];
//   result.m24 = 0.0;

//   result.m31 = transform[6];
//   result.m32 = transform[7];
//   result.m33 = transform[8];
//   result.m34 = 0.0;

//   result.m41 = 0.0;
//   result.m42 = 0.0;
//   result.m43 = 0.0;
//   result.m44 = 1.0;
//   return result;
// }

static CGPathRef SkPathToCGPath(SkPath path) {
      CGMutablePathRef result = CGPathCreateMutable();
      SkPoint points[4];
      const size_t pow2 = 3;
      const size_t quadCount = 1 << pow2;

      SkPoint conicQuads[1 + 2 * quadCount];
      SkPath::Iter iter(path, true);
      while(true) {
        SkPath::Verb verb = iter.next(points);
        switch(verb) {
          case SkPath::kMove_Verb:
          CGPathMoveToPoint(result, nil, points[0].x(), points[0].y());
          break;
          case SkPath::kLine_Verb:
          CGPathAddLineToPoint(result, nil,points[1].x(), points[1].y());
          break;
          case SkPath::kQuad_Verb:
          CGPathAddQuadCurveToPoint(result, nil, points[1].x(), points[1].y(), points[2].x(), points[2].y());
          break;
          case SkPath::kConic_Verb:
           
          SkPath::ConvertConicToQuads	(points[0], points[1], points[2], iter.conicWeight(), conicQuads, pow2);
          for (size_t i = 0; i < quadCount; i++) {
            CGPathAddQuadCurveToPoint(
              result, nil,
              conicQuads[1 + 2 * i].x(), conicQuads[1 + 2 * i].y(),
              conicQuads[1 + 2 * i + 1].x(), conicQuads[1 + 2 * i + 1].y());
          }
          break;
          case SkPath::kCubic_Verb:
          CGPathAddCurveToPoint(result, nil, points[1].x(), points[1].y(), points[2].x(), points[2].y(), points[3].x(), points[3].y());
          break;
          case SkPath::kClose_Verb:
          CGPathCloseSubpath(result);
          break;
          case SkPath::kDone_Verb:
          return result;
        }
      }
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

void IOSLayeredPaintContext::Layer::installChildren() {
  CALayer *parentCALayer = layer();
  int zPosition = 0;
  for (auto &childLayer : children_) {
    CALayer *childCALayer = childLayer->layer();
    if ([childCALayer superlayer] != parentCALayer) {
      [parentCALayer addSublayer:childCALayer];
    }
    if (childCALayer.zPosition != zPosition) {
      childCALayer.zPosition = zPosition;
    }
    zPosition++;
    childLayer->installChildren();
  }
  children_.clear();
}

void IOSLayeredPaintContext::Layer::manifest(IOSLayeredPaintContext &context) {
  if (paint_layers_.empty()) {
    layer_  = context.basicLayer();
  } else {
    Surface *surface = context.drawLayer();
    this->surface = surface;
    layer_ = surface->caLayer;
  }

  int alpha = SkColorGetA(background_color_);
  layer_.backgroundColor = [UIColor colorWithRed: SkColorGetR(background_color_)
         green: SkColorGetG(background_color_) blue: SkColorGetB(background_color_) alpha: alpha].CGColor;
  if (alpha == 0xff) {
    layer_.opaque = true;
  } else {
    layer_.opaque = false;
  }

  layer_.opacity = 1;

  if (layer_.masksToBounds != clip_) {
    layer_.masksToBounds = clip_;
  }

  if (!path.isEmpty()) {
    CAShapeLayer *shapeLayer = [[CAShapeLayer alloc] init];
    shapeLayer.path = SkPathToCGPath(path);
    layer_.mask = shapeLayer;
  } else {
    layer_.mask = nil;
  }

  if (elevation_ == 0.0f) {
    layer_.shadowOpacity = 0.0;
  } else {
     layer_.shadowOpacity = 0.0;
     
     // 0.2 XXX
    // layer_.opaque = YES; /// XXX
     // CurrentLayer().shadowPath = CGPathCreateWithRect(CurrentLayer().bounds, nullptr);
    // layer_.shadowRadius = 2.0;
    // layer_.shadowOffset = CGSizeMake(0 , 2);
  }
  layer_.borderWidth = 0.0;
  layer_.borderColor = [UIColor purpleColor].CGColor;

  CGRect newFrame = CGRectMake(frame_.x(), frame_.y(), frame_.width(), frame_.height());
  if (!CGRectEqualToRect(layer_.frame, newFrame)) {
    layer_.frame = newFrame;
  }

  for (size_t i = 0; i < externalLayers.size(); i++) {
    CALayer *externalLayer = externalLayers[i];
    //[externalLayer setNeedsDisplay];
    //UIView *delegate = (UIView*)externalLayer.delegate ;
    //[delegate setNeedsDisplay];
  //  NSLog(@"Delegate %@", delegate);
    // for (CALayer *sublayer in [externalLayer sublayers]) {
    //   [sublayer setNeedsDisplay];
    //   for (CALayer *sublayer2 in [sublayer sublayers]) {
    //           [sublayer2 setNeedsDisplay];
    //   }
    // }
    externalLayer.needsDisplayOnBoundsChange = true;
    SkRect externalFrame = externalLayerFrames[i];
    size_t height = externalLayerHeights[i];
    if (externalLayer.superlayer != layer_) {
      [layer_ addSublayer: externalLayer];
      externalLayer.anchorPoint = CGPointMake(0, 0);
        //  CGFloat screenScale = [UIScreen mainScreen].scale;
        //  externalLayer.contentsScale = screenScale;
    }
    CGPoint newPosition = CGPointMake(externalFrame.x(), externalFrame.y());
    if (!CGPointEqualToPoint(externalLayer.position, newPosition)) {
      externalLayer.position = newPosition;
    }
   
   // TODO(sigurdm): Handle transforms...
   // externalLayer.transform = skiaToCATransform(externalLayerTransforms[i]);
    externalLayer.zPosition = height + 0.5;
      // NSLog(@"Canvastransform %f %f %f", externalLayer.transform.m11, externalLayer.transform.m21, externalLayer.transform.m31);
      // NSLog(@"CanvasTransform %f %f %f", externalLayer.transform.m12, externalLayer.transform.m22, externalLayer.transform.m32);
      // NSLog(@"CanvasTransform %f %f %f", externalLayer.transform.m13, externalLayer.transform.m23, externalLayer.transform.m33);
  }
}

CALayer *IOSLayeredPaintContext::Layer::layer() { return layer_; }

bool IOSLayeredPaintContext::Layer::makeCurrent() { return surface->gpuSurface->MakeCurrent(); }

void  IOSLayeredPaintContext::Layer::AddPaintedLayer(flow::Layer *layer) {
  paint_layers_.push_back(layer);
}

IOSLayeredPaintContext::IOSLayeredPaintContext(PlatformView::SurfaceConfig surfaceConfig,
      CALayer* layer) : 
      eaglContext_([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]),
      root_surface_(IOSSurface::Create(surfaceConfig, layer, eaglContext_.get())),
      surface_config_(surfaceConfig) {
  IOSSurfaceGL *rootSurfaceGL = static_cast<IOSSurfaceGL*>(root_surface_.get());
  Layer *rootLayer = new Layer();

  rootLayer->surface = new Surface(rootSurfaceGL->layer(), rootSurfaceGL);
  rootLayer->layer_ = rootSurfaceGL->layer();

  stack_.push_back(rootLayer);

  offsets_.push_back(SkPoint::Make(0, 0));
  transforms_.push_back(SkMatrix::Concat(SkMatrix::I(), SkMatrix::I()));
//  = reinterpret_cast<IOSSurfaceGL*>(rootSurface);
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
    CAEAGLLayer *newCALayer = [[[root_surface_->GetLayer() class] alloc] init];
    CGFloat screenScale = [UIScreen mainScreen].scale;
    newCALayer.contentsScale = screenScale;
    newCALayer.rasterizationScale = screenScale;
    newCALayer.presentsWithTransaction = YES;

    IOSSurfaceGL *iosSurface = new IOSSurfaceGL(surface_config_, newCALayer, eaglContext_);
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
  CALayerCache_index_ = 0;
  CAEAGLLayerCache_index_ = 0;
  [CATransaction begin];
  [CATransaction setValue:(id)kCFBooleanTrue
                   forKey:kCATransactionDisableActions];
  //NSLog(@"Begin");
}

void IOSLayeredPaintContext::Finish() {
  for (size_t i = CALayerCache_index_; i < CALayerCache_.size(); i++) {
    [CALayerCache_[i] removeFromSuperlayer];
  }
  for (size_t i = CAEAGLLayerCache_index_; i < CAEAGLLayerCache_.size(); i++) {
    [CAEAGLLayerCache_[i]->caLayer removeFromSuperlayer];
  }

  //NSLog(@"Finishing %lu",stack_.size());

  [CATransaction commit];
}

void IOSLayeredPaintContext::PushLayer(SkRect bounds) {
  stack_.back()->children_.push_back(std::make_unique<Layer>());
  Layer *newLayer = stack_.back()->children_.back().get();

  SkRect transformedBounds = bounds.makeOffset(-current_offset().x(), -current_offset().y());
  transforms_.back().mapRect(&transformedBounds);
  newLayer->frame_ = transformedBounds;
  newLayer->transform = transforms_.back();
  SkPoint offset = SkPoint::Make(bounds.x(), bounds.y());
  newLayer->offset = offset;
  stack_.push_back(newLayer);
  offsets_.push_back(offset);
}

void IOSLayeredPaintContext::PopLayer() {
  Layer* top = stack_.back();
  paint_tasks_.push_back(top);
  stack_.pop_back();
  offsets_.pop_back();
}

SkPoint IOSLayeredPaintContext::current_offset() { return stack_.back()->offset; }

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
  stack_.back()->externalLayerHeights.push_back(stack_.back()->children_.size());
  SkMatrix transform = transforms_.back();
  //transform.preTranslate(newBounds.x(), newBounds.y());
  stack_.back()->externalLayerTransforms.push_back(transform);

  system_composited_rect_.join(newBounds);
}

void IOSLayeredPaintContext::SetColor(SkColor color) {
  stack_.back()->background_color_ = color;
}

void IOSLayeredPaintContext::SetClipPath(SkPath path) {
  path.offset(-current_offset().x(), -current_offset().y(), &(stack_.back()->path));
}

// void IOSLayeredPaintContext::SetCornerRadius(float radius) {
//   // NSLog(@"SetCornerRadius %f", radius);
//   stack_.back()->corner_radius_ = radius;
// }

void IOSLayeredPaintContext::ClipRect() {
  stack_.back()->clip_ = true;
}

void IOSLayeredPaintContext::Elevate(float elevation) {
  stack_.back()->elevation_ = elevation;
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
  //  FXL_DLOG(INFO) << "Layer id: " << task.layer->id;
  //  FXL_DLOG(INFO) << "Layer Size: " << task.layer->layer().frame.size.width << "x" << task.layer->layer().frame.size.height;
    task->manifest(*this);
    if (!task->paint_layers_.empty()) {
      task->makeCurrent();
      SkCanvas* canvas = task->canvas();
      flow::Layer::PaintContext context = {
        *canvas,
        frame.context().frame_time(),
        frame.context().engine_time(),
        frame.context().memory_usage(),
        frame.context().texture_registry(),
        false,};
      //canvas->restoreToCount(1);
      int saveCount = canvas->save();
      canvas->clear(task->background_color_);
      context.canvas.scale(2, 2); // Why??
      canvas->translate(-task->offset.x(), -task->offset.y());
      context.canvas.concat(task->transform);

      SkPaint paint;
      paint.setColor(SK_ColorGREEN);
      paint.setStyle(SkPaint::kStroke_Style);

      for (flow::Layer* layer : task->paint_layers_) {
        layer->Paint(context);
      }
      // std::stringstream a;
      // a << "Layer " << task.layer->id;
      // canvas->drawString(a.str().c_str(), task.offset.x() + 2, task.offset.y() + 20, paint);
      canvas->restoreToCount(saveCount);
      task->present();
      FXL_DCHECK(glGetError() == GL_NO_ERROR);
    }
  }
  // Setting up parent-child relationship from the bottom to avoid creating temporary cycles.
  stack_[0]->installChildren();
  paint_tasks_.clear();
}

void IOSLayeredPaintContext::Transform(SkMatrix transform) {
  // [CurrentLayer() setAffineTransform:
  //  CGAffineTransformMake(
  //    transform[0], transform[1],
  //    transform[3], transform[4],
  //    transform[6], transform[7])];
  transforms_.push_back(SkMatrix::Concat(transforms_.back(), transform));
//  NSLog(@"Transform {");
}

void IOSLayeredPaintContext::PopTransform() {
  // NSLog(@"Poptransform");
  transforms_.pop_back();
 // NSLog(@"} Transform ");
}

void IOSLayeredPaintContext::AddPaintedLayer(flow::Layer *layer) {
//  NSLog(@"Adding painted layer to layer %d", stack_.back()->id);
  stack_.back()->AddPaintedLayer(layer);
}

} // namespace shell
