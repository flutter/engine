// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_system_compositor_context.h"
#include "flutter/shell/platform/darwin/ios/ios_external_texture_layer.h"
#include "flutter/shell/platform/darwin/ios/ios_surface_gl.h"

#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

#include <UIKit/UIKit.h>

namespace shell {

// static CATransform3D skMatrixToCATransform(SkMatrix transform) {
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

IOSSystemCompositorContext::Surface::Surface(CALayer *caLayer, IOSSurfaceGL *iosSurface) :
  caLayer(caLayer), iosSurface(iosSurface), gpuSurface(iosSurface->CreateGPUSurface()) {}

IOSSystemCompositorContext::FlowCompositingLayer::FlowCompositingLayer() :
  background_color_(SK_ColorTRANSPARENT) {}

SkCanvas *IOSSystemCompositorContext::FlowCompositingLayer::canvas() {
  CGSize size = layer().frame.size;
  canvas_ = surface_->gpuSurface->AcquireRenderSurface(SkISize::Make(size.width * 2, size.height * 2))->getCanvas();
  return canvas_;
}

void IOSSystemCompositorContext::FlowCompositingLayer::present() {
   canvas_->flush();
   surface_->iosSurface->GLContextPresent();
}

IOSSystemCompositorContext::ExternalCompositingLayer::ExternalCompositingLayer(CALayer *externalLayer) {
  [externalLayer retain];
  layer_.reset(externalLayer);
}

void IOSSystemCompositorContext::ExternalCompositingLayer::installChildren() {
    CALayer *externalLayer = layer_.get();
    externalLayer.needsDisplayOnBoundsChange = true;
    externalLayer.anchorPoint = CGPointMake(0, 0);
    CGPoint newPosition = CGPointMake(frame_.x(), frame_.y());
    if (!CGPointEqualToPoint(externalLayer.position, newPosition)) {
      externalLayer.position = newPosition;
    }
  // No children.
}

void IOSSystemCompositorContext::FlowCompositingLayer::installChildren() {
  CALayer *parentCALayer = layer();
  // Using zPosition to keep the right stackingorder even if the cached CALayers happens to be in the wrong order.
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

void IOSSystemCompositorContext::ExternalCompositingLayer::manifest(IOSSystemCompositorContext &context) {



   // TODO(sigurdm): Handle transforms...
   // externalLayer.transform = skMatrixToCATransform(externalLayerTransforms[i]);
}


void IOSSystemCompositorContext::FlowCompositingLayer::manifest(IOSSystemCompositorContext &context) {
  CALayer *layer;

  if (paint_layers_.empty()) {
    layer = context.createBasicLayer();
  } else {
    Surface *surface = context.createDrawLayer();
    surface_ = surface;
    layer = surface->caLayer;
  }

  [layer retain];
  layer_.reset(layer);

  int alpha = SkColorGetA(background_color_);
  layer.backgroundColor = [UIColor colorWithRed: SkColorGetR(background_color_)
         green: SkColorGetG(background_color_) blue: SkColorGetB(background_color_) alpha: alpha].CGColor;
  if (alpha == 0xff) {
    layer.opaque = true;
  } else {
    layer.opaque = false;
  }

  // TODO(sigurdm): handle opacity.
  layer.opacity = 1;

  if (layer.masksToBounds != clip_) {
    layer.masksToBounds = clip_;
  }

  if (!path_.isEmpty()) {
    CAShapeLayer *shapeLayer = [[CAShapeLayer alloc] init];
    shapeLayer.path = SkPathToCGPath(path_);
    layer.mask = shapeLayer;
  } else {
    layer.mask = nil;
  }

  if (!frame_.isEmpty()) {
    CGRect newFrame = CGRectMake(frame_.x(), frame_.y(), frame_.width(), frame_.height());
    if (!CGRectEqualToRect(layer.frame, newFrame)) {
      layer.frame = newFrame;
    }
  }
}

CALayer *IOSSystemCompositorContext::CompositingLayer::layer() { return layer_; }

bool IOSSystemCompositorContext::FlowCompositingLayer::makeCurrent() { return surface_->gpuSurface->MakeCurrent(); }

void  IOSSystemCompositorContext::FlowCompositingLayer::AddPaintedLayer(flow::Layer *layer) {
  paint_layers_.push_back(layer);
}

IOSSystemCompositorContext::IOSSystemCompositorContext(PlatformView::SurfaceConfig surfaceConfig,
      CALayer* layer) : 
      eaglContext_([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]),
      root_surface_(IOSSurface::Create(surfaceConfig, layer, eaglContext_.get())),
      surface_config_(surfaceConfig) {
  IOSSurfaceGL *rootSurfaceGL = static_cast<IOSSurfaceGL*>(root_surface_.get());
  root_layer_ = std::make_unique<FlowCompositingLayer>();
  root_layer_->surface_ = new Surface(layer, rootSurfaceGL);
  [layer retain];
  root_layer_->layer_.reset(layer);

  root_layer_->frame_ = SkRect::MakeWH(layer.bounds.size.width, layer.bounds.size.height);
  // FXL_DCHECK(!root_layer_->frame_.isEmpty());
  stack_.push_back(root_layer_.get());
  
  offsets_.push_back(SkPoint::Make(0, 0));
  transforms_.push_back(SkMatrix::Concat(SkMatrix::I(), SkMatrix::I()));
}

CALayer *IOSSystemCompositorContext::createBasicLayer() {
  CALayer *result;
  if (CALayerCache_index_ >= CALayerCache_.size()) {
    result = [[CALayer alloc] init];
    CALayerCache_.push_back(result);
  } else {
    result = CALayerCache_[CALayerCache_index_];
  }
  CALayerCache_index_++;
  return result;
}

IOSSystemCompositorContext::Surface *IOSSystemCompositorContext::createDrawLayer() {
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

  }
  CAEAGLLayerCache_index_++;
  return result;
}

void IOSSystemCompositorContext::Reset() {
  CALayerCache_index_ = 0;
  CAEAGLLayerCache_index_ = 0;
  [CATransaction begin];
  [CATransaction setValue:(id)kCFBooleanTrue
                   forKey:kCATransactionDisableActions];
  //paint_tasks_.push_back(root_layer_.get());
}

void IOSSystemCompositorContext::Finish() {
  for (size_t i = CALayerCache_index_; i < CALayerCache_.size(); i++) {
    [CALayerCache_[i] removeFromSuperlayer];
  }
  for (size_t i = CAEAGLLayerCache_index_; i < CAEAGLLayerCache_.size(); i++) {
    [CAEAGLLayerCache_[i]->caLayer removeFromSuperlayer];
  }

  [CATransaction commit];
}

void IOSSystemCompositorContext::PushLayer(SkRect bounds) {
  auto compositingLayer = std::make_unique<FlowCompositingLayer>();

  SkRect transformedBounds = bounds.makeOffset(-currentOffset().x(), -currentOffset().y());
  transforms_.back().mapRect(&transformedBounds);
  compositingLayer->frame_ = transformedBounds;
  FXL_DCHECK(!compositingLayer->frame_.isEmpty());
  
  compositingLayer->transform_ = transforms_.back();
  SkPoint offset = SkPoint::Make(bounds.x(), bounds.y());
  compositingLayer->offset_ = offset;
  FlowCompositingLayer *pointer = compositingLayer.get();
  stack_.back()->children_.push_back(std::move(compositingLayer)) ;
  stack_.push_back(pointer);
  paint_tasks_.push_back(pointer);
  offsets_.push_back(offset);
}

void IOSSystemCompositorContext::PopLayer() {
  stack_.back();
  stack_.pop_back();
  offsets_.pop_back();
}

SkPoint IOSSystemCompositorContext::currentOffset() {
  return offsets_.back();
  }

SkCanvas *IOSSystemCompositorContext::CurrentCanvas() {
  return stack_.back()->canvas();
}

void IOSSystemCompositorContext::AddExternalLayer(flow::Texture* texture, SkRect bounds) {
  CALayer *externalLayer = (static_cast<IOSExternalTextureLayer*>(texture))->layer();
  auto compositingLayer = std::make_unique<ExternalCompositingLayer>(externalLayer);
  SkRect newBounds = bounds.makeOffset(-currentOffset().x(), -currentOffset().y());
  compositingLayer->frame_ = newBounds;

  compositingLayer->transform_ = transforms_.back();

  stack_.back()->children_.push_back(std::move(compositingLayer));
}

void IOSSystemCompositorContext::SetColor(SkColor color) {
  stack_.back()->background_color_ = color;
}

void IOSSystemCompositorContext::SetClipPath(SkPath path) {
  path.offset(-currentOffset().x(), -currentOffset().y(), &(stack_.back()->path_));
}

void IOSSystemCompositorContext::ClipFrame() {
  stack_.back()->clip_ = true;
}

void IOSSystemCompositorContext::ExecutePaintTasks(flow::CompositorContext::ScopedFrame& frame) {
  FXL_DCHECK(glGetError() == GL_NO_ERROR);

  for (auto& task : paint_tasks_) {
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
      int saveCount = canvas->save();
      canvas->clear(task->background_color_);
      CGFloat screenScale = [UIScreen mainScreen].scale;
      context.canvas.scale(screenScale, screenScale); // TODO(sigurdm): is this right?
      canvas->translate(-task->offset_.x(), -task->offset_.y());
      context.canvas.concat(task->transform_);

      for (flow::Layer* layer : task->paint_layers_) {
        layer->Paint(context);
      }
        
      canvas->restoreToCount(saveCount);
      task->present();
      FXL_DCHECK(glGetError() == GL_NO_ERROR);
    }
  }
  // Setting up parent-child relationship from the root to avoid creating transient cycles.
  stack_[0]->installChildren();
  paint_tasks_.clear();
}

void IOSSystemCompositorContext::Transform(SkMatrix transform) {
  transforms_.push_back(SkMatrix::Concat(transforms_.back(), transform));
}

void IOSSystemCompositorContext::PopTransform() {
  transforms_.pop_back();
}

void IOSSystemCompositorContext::AddPaintedLayer(flow::Layer *layer) {
  stack_.back()->AddPaintedLayer(layer);
}

} // namespace shell
