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

@interface FlutterTouchIgnoringCALayer : CALayer
@end

@interface FlutterTouchIgnoringCAEGLLayer : CAEAGLLayer
@end

@interface FlutterTouchIgnoringGLView : UIView
@end

@interface FlutterTouchIgnoringView : UIView
@end

@implementation FlutterTouchIgnoringView
+ (Class)layerClass {
  return [FlutterTouchIgnoringCALayer class];
}
@end

@implementation FlutterTouchIgnoringGLView
+ (Class)layerClass {
#if TARGET_IPHONE_SIMULATOR
  return [FlutterTouchIgnoringCALayer class];
#else   // TARGET_IPHONE_SIMULATOR
  return [FlutterTouchIgnoringCAEGLLayer class];
#endif  // TARGET_IPHONE_SIMULATOR
}
@end

static BOOL layerHasSublayerContainingPoint(CALayer* layer, CGPoint point) {
  // Letting all touches fall through, unless they land on some sublayer
  // interested in the event.
  if (!CGRectContainsPoint(layer.bounds, point)) {
    return false;
  }
  for (CALayer* sublayer in layer.sublayers) {
    CGPoint converted = [layer convertPoint:point toLayer:sublayer];
    if ([sublayer containsPoint:converted]) {
      return true;
    }
  }
  // TODO(sigurdm): Do transparency checking to allow getting touches from
  // flutter layers on top of of external views.
  return false;
}

@implementation FlutterTouchIgnoringCALayer

- (BOOL)containsPoint:(CGPoint)point {
  return layerHasSublayerContainingPoint(self, point);
}

@end

@implementation FlutterTouchIgnoringCAEGLLayer
- (BOOL)containsPoint:(CGPoint)point {
  return layerHasSublayerContainingPoint(self, point);
}
@end

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
  while (true) {
    SkPath::Verb verb = iter.next(points);
    switch (verb) {
      case SkPath::kMove_Verb:
        CGPathMoveToPoint(result, nil, points[0].x(), points[0].y());
        break;
      case SkPath::kLine_Verb:
        CGPathAddLineToPoint(result, nil, points[1].x(), points[1].y());
        break;
      case SkPath::kQuad_Verb:
        CGPathAddQuadCurveToPoint(result, nil, points[1].x(), points[1].y(), points[2].x(),
                                  points[2].y());
        break;
      case SkPath::kConic_Verb:

        SkPath::ConvertConicToQuads(points[0], points[1], points[2], iter.conicWeight(), conicQuads,
                                    pow2);
        for (size_t i = 0; i < quadCount; i++) {
          CGPathAddQuadCurveToPoint(result, nil, conicQuads[1 + 2 * i].x(),
                                    conicQuads[1 + 2 * i].y(), conicQuads[1 + 2 * i + 1].x(),
                                    conicQuads[1 + 2 * i + 1].y());
        }
        break;
      case SkPath::kCubic_Verb:
        CGPathAddCurveToPoint(result, nil, points[1].x(), points[1].y(), points[2].x(),
                              points[2].y(), points[3].x(), points[3].y());
        break;
      case SkPath::kClose_Verb:
        CGPathCloseSubpath(result);
        break;
      case SkPath::kDone_Verb:
        return result;
    }
  }
}

IOSSystemCompositorContext::Surface::Surface(UIView* view,
                                             IOSSurfaceGL* iosSurface,
                                             GrContext* grContext)
    : view(view), iosSurface(iosSurface), gpuSurface(iosSurface->CreateGPUSurface(grContext)) {}

IOSSystemCompositorContext::FlowCompositingLayer::FlowCompositingLayer()
    : background_color_(SK_ColorTRANSPARENT) {}

SkCanvas* IOSSystemCompositorContext::FlowCompositingLayer::canvas() {
  CGSize size = layer().frame.size;
  canvas_ =
      surface_->gpuSurface->AcquireRenderSurface(SkISize::Make(size.width * 2, size.height * 2))
          ->getCanvas();
  return canvas_;
}

void IOSSystemCompositorContext::FlowCompositingLayer::present() {
  canvas_->flush();
  surface_->gpuSurface->PresentSurface(canvas_);
}

IOSSystemCompositorContext::ExternalCompositingLayer::ExternalCompositingLayer(
    UIView* externalView) {
  [externalView retain];
  view_.reset(externalView);
}

void IOSSystemCompositorContext::ExternalCompositingLayer::installChildren() {
  CALayer* externalLayer = view_.get().layer;
  externalLayer.needsDisplayOnBoundsChange = true;
  externalLayer.anchorPoint = CGPointMake(0, 0);
  CGPoint newPosition = CGPointMake(frame_.x(), frame_.y());
  if (!CGPointEqualToPoint(externalLayer.position, newPosition)) {
    externalLayer.position = newPosition;
  }
}

void IOSSystemCompositorContext::FlowCompositingLayer::installChildren() {
  UIView* parentView = view();
  // Using zPosition to keep the right stacking order even if the cached Views happens to be in
  // the wrong order.
  int zPosition = 0;
  for (auto& child : children_) {
    UIView* childView = child->view();

    if ([childView superview] != parentView) {
      [parentView addSubview:childView];
    }
    if (childView.layer.zPosition != zPosition) {
      childView.layer.zPosition = zPosition;
    }
    zPosition++;
    child->installChildren();
  }
  children_.clear();
}

void IOSSystemCompositorContext::ExternalCompositingLayer::manifest(
    IOSSystemCompositorContext& context) {
  // TODO(sigurdm): Handle transforms...
  // externalLayer.transform = skMatrixToCATransform(externalLayerTransforms[i]);
}

void IOSSystemCompositorContext::FlowCompositingLayer::manifest(
    IOSSystemCompositorContext& context) {
  UIView* view;

  if (paint_layers_.empty()) {
    view = context.createBasicLayer();
  } else {
    Surface* surface = context.createDrawLayer();
    surface_ = surface;
    view = surface->view;
  }
  [view retain];
  view_.reset(view);

  int alpha = SkColorGetA(background_color_);
  view.layer.backgroundColor = [UIColor colorWithRed:SkColorGetR(background_color_)
                                               green:SkColorGetG(background_color_)
                                                blue:SkColorGetB(background_color_)
                                               alpha:alpha]
                                   .CGColor;
  if (alpha == 0xff) {
    view.layer.opaque = true;
  } else {
    view.layer.opaque = false;
  }

  // TODO(sigurdm): handle opacity.
  view.layer.opacity = 1;

  if (view.layer.masksToBounds != clip_) {
    view.layer.masksToBounds = clip_;
  }

  if (!path_.isEmpty()) {
    CAShapeLayer* shapeLayer = [[CAShapeLayer alloc] init];
    CGPathRef path = SkPathToCGPath(path_);
    shapeLayer.path = path;
    CGPathRelease(path);
    view.layer.mask = shapeLayer;
  } else {
    view.layer.mask = nil;
  }

  if (!frame_.isEmpty()) {
    CGRect newFrame = CGRectMake(frame_.x(), frame_.y(), frame_.width(), frame_.height());
    if (!CGRectEqualToRect(view.layer.frame, newFrame)) {
      view.layer.frame = newFrame;
    }
  }
}

CALayer* IOSSystemCompositorContext::CompositingLayer::layer() {
  return view_.get().layer;
}

UIView* IOSSystemCompositorContext::CompositingLayer::view() {
  return view_;
}

bool IOSSystemCompositorContext::FlowCompositingLayer::makeCurrent() {
  return surface_->gpuSurface->MakeCurrent();
}

void IOSSystemCompositorContext::FlowCompositingLayer::AddPaintedLayer(flow::Layer* layer) {
  paint_layers_.push_back(layer);
}

IOSSystemCompositorContext::IOSSystemCompositorContext(PlatformView::SurfaceConfig surfaceConfig,
                                                       UIView* root_view)
    : eaglContext_([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]),
      root_surface_(IOSSurface::Create(surfaceConfig, root_view.layer, eaglContext_.get())),
      surface_config_(surfaceConfig) {
  IOSSurfaceGL* rootSurfaceGL = static_cast<IOSSurfaceGL*>(root_surface_.get());
  root_layer_ = std::make_unique<FlowCompositingLayer>();
  [EAGLContext setCurrentContext:eaglContext_.get()];
  gr_context_ = std::make_unique<GpuGrContext>();
  root_layer_->surface_ = new Surface(root_view, rootSurfaceGL, gr_context_->GetContext());
  [root_view retain];
  root_layer_->view_.reset(root_view);

  root_layer_->frame_ = SkRect::MakeWH(root_view.bounds.size.width, root_view.bounds.size.height);
  stack_.push_back(root_layer_.get());

  offsets_.push_back(SkPoint::Make(0, 0));
  transforms_.push_back(SkMatrix::Concat(SkMatrix::I(), SkMatrix::I()));
}

UIView* IOSSystemCompositorContext::createBasicLayer() {
  UIView* result;
  if (CALayerCache_index_ >= CALayerCache_.size()) {
    result = [[FlutterTouchIgnoringView alloc] init];
    CALayerCache_.emplace_back(result);
  } else {
    result = CALayerCache_[CALayerCache_index_];
  }
  CALayerCache_index_++;
  return result;
}

IOSSystemCompositorContext::Surface* IOSSystemCompositorContext::createDrawLayer() {
  Surface* result;
  if (CAEAGLLayerCache_index_ >= CAEAGLLayerCache_.size()) {
    UIView* view = [[FlutterTouchIgnoringGLView alloc] init];
    CAEAGLLayer* newCALayer = (CAEAGLLayer*)(view.layer);
    CGFloat screenScale = [UIScreen mainScreen].scale;
    newCALayer.contentsScale = screenScale;
    newCALayer.rasterizationScale = screenScale;
    if (@available(iOS 9.0, *)) {
      newCALayer.presentsWithTransaction = YES;
    }

    IOSSurfaceGL* iosSurface = new IOSSurfaceGL(surface_config_, newCALayer, eaglContext_);
    result = new Surface(view, iosSurface, gr_context_->GetContext());
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
  [CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
  // paint_tasks_.push_back(root_layer_.get());
}

void IOSSystemCompositorContext::Finish() {
  for (size_t i = CALayerCache_index_; i < CALayerCache_.size(); i++) {
    [CALayerCache_[i] removeFromSuperview];
  }
  for (size_t i = CAEAGLLayerCache_index_; i < CAEAGLLayerCache_.size(); i++) {
    [CAEAGLLayerCache_[i]->view removeFromSuperview];
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
  FlowCompositingLayer* pointer = compositingLayer.get();
  stack_.back()->children_.push_back(std::move(compositingLayer));
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

SkCanvas* IOSSystemCompositorContext::CurrentCanvas() {
  return stack_.back()->canvas();
}

void IOSSystemCompositorContext::AddChildScene(flow::Texture* texture, SkRect bounds) {
  UIView* externalView = (static_cast<IOSExternalTextureLayer*>(texture))->layer();
  auto compositingLayer = std::make_unique<ExternalCompositingLayer>(externalView);
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
          false,
      };
      int saveCount = canvas->save();
      canvas->clear(task->background_color_);
      CGFloat screenScale = [UIScreen mainScreen].scale;
      context.canvas.scale(screenScale, screenScale);  // TODO(sigurdm): is this right?
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

void IOSSystemCompositorContext::AddPaintedLayer(flow::Layer* layer) {
  stack_.back()->AddPaintedLayer(layer);
}

void IOSSystemCompositorContext::TearDown() {
  // TODO(sigurdm): Implement.
}

void IOSSystemCompositorContext::Clear() {
  // TODO(sigurdm): Implement.
  // if (surface_ == nullptr) {
  //   return;
  // }

  // auto frame = surface_->AcquireFrame(size);

  // if (frame == nullptr) {
  //   return;
  // }

  // SkCanvas* canvas = frame->SkiaCanvas();

  // if (canvas == nullptr) {
  //   return;
  // }

  // canvas->clear(color);

  // frame->Submit();
}

GrContext* IOSSystemCompositorContext::GetGrContext() {
  return gr_context_->GetContext();
}

}  // namespace shell
