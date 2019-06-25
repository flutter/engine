// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Source/FlutterPlatformViews_Internal.h"

#include "flutter/shell/platform/darwin/ios/ios_surface.h"

namespace flutter {

FlutterPlatformViewLayer::FlutterPlatformViewLayer(fml::scoped_nsobject<UIView> overlay_view,
                                                   std::unique_ptr<IOSSurface> ios_surface,
                                                   std::unique_ptr<Surface> surface)
    : overlay_view(std::move(overlay_view)),
      ios_surface(std::move(ios_surface)),
      surface(std::move(surface)){};

FlutterPlatformViewLayer::~FlutterPlatformViewLayer() = default;

FlutterPlatformViewsController::FlutterPlatformViewsController() = default;

FlutterPlatformViewsController::~FlutterPlatformViewsController() = default;

CATransform3D GetCATransform3DFromSkMatrix(const SkMatrix& matrix) {
  // Skia only supports 2D transform so we don't map z.
  CATransform3D transform = CATransform3DIdentity;
  transform.m11 = matrix.getScaleX();
  transform.m21 = matrix.getSkewX();
  transform.m41 = matrix.getTranslateX();
  transform.m14 = matrix.getPerspX();

  transform.m12 = matrix.getSkewY();
  transform.m22 = matrix.getScaleY();
  transform.m42 = matrix.getTranslateY();
  transform.m24 = matrix.getPerspY();
  return transform;
}

void ResetAnchor(CALayer* layer) {
  // Flow uses (0, 0) to apply transform matrix so we need to match that in Quartz.
  layer.anchorPoint = CGPointZero;
  layer.position = CGPointZero;
}

}  // namespace flutter

@implementation ChildClippingView

+ (CGRect)getCGRectFromSkRect:(const SkRect&)clipSkRect {
  return CGRectMake(clipSkRect.fLeft, clipSkRect.fTop, clipSkRect.fRight - clipSkRect.fLeft,
                    clipSkRect.fBottom - clipSkRect.fTop);
}

- (void)clipRect:(const SkRect&)clipSkRect {
  CGRect clipRect = [ChildClippingView getCGRectFromSkRect:clipSkRect];
  CGPathRef pathRef = CGPathCreateWithRect(clipRect, nil);
  CAShapeLayer* clip = [[CAShapeLayer alloc] init];
  clip.path = pathRef;
  self.layer.mask = clip;
  CGPathRelease(pathRef);
}

- (void)clipRRect:(const SkRRect&)clipSkRRect {
  CGPathRef pathRef = nullptr;
  switch (clipSkRRect.getType()) {
    case SkRRect::kEmpty_Type: {
      break;
    }
    case SkRRect::kRect_Type: {
      [self clipRect:clipSkRRect.rect()];
      return;
    }
    case SkRRect::kOval_Type:
    case SkRRect::kSimple_Type: {
      CGRect clipRect = [ChildClippingView getCGRectFromSkRect:clipSkRRect.rect()];
      pathRef = CGPathCreateWithRoundedRect(clipRect, clipSkRRect.getSimpleRadii().x(),
                                            clipSkRRect.getSimpleRadii().y(), nil);
      break;
    }
    case SkRRect::kNinePatch_Type:
    case SkRRect::kComplex_Type: {
      CGMutablePathRef mutablePathRef = CGPathCreateMutable();
      // Complex types, we manually add each corner.
      SkRect clipSkRect = clipSkRRect.rect();
      SkVector topLeftRadii = clipSkRRect.radii(SkRRect::kUpperLeft_Corner);
      SkVector topRightRadii = clipSkRRect.radii(SkRRect::kUpperRight_Corner);
      SkVector bottomRightRadii = clipSkRRect.radii(SkRRect::kLowerRight_Corner);
      SkVector bottomLeftRadii = clipSkRRect.radii(SkRRect::kLowerLeft_Corner);

      // Start drawing RRect
      // Move point to the top left corner adding the top left radii's x.
      CGPathMoveToPoint(mutablePathRef, nil, clipSkRect.fLeft + topLeftRadii.x(), clipSkRect.fTop);
      // Move point horizontally right to the top right corner and add the top right curve.
      CGPathAddLineToPoint(mutablePathRef, nil, clipSkRect.fRight - topRightRadii.x(),
                           clipSkRect.fTop);
      CGPathAddCurveToPoint(mutablePathRef, nil, clipSkRect.fRight, clipSkRect.fTop,
                            clipSkRect.fRight, clipSkRect.fTop + topRightRadii.y(),
                            clipSkRect.fRight, clipSkRect.fTop + topRightRadii.y());
      // Move point vertically down to the bottom right corner and add the bottom right curve.
      CGPathAddLineToPoint(mutablePathRef, nil, clipSkRect.fRight,
                           clipSkRect.fBottom - bottomRightRadii.y());
      CGPathAddCurveToPoint(mutablePathRef, nil, clipSkRect.fRight, clipSkRect.fBottom,
                            clipSkRect.fRight - bottomRightRadii.x(), clipSkRect.fBottom,
                            clipSkRect.fRight - bottomRightRadii.x(), clipSkRect.fBottom);
      // Move point horizontally left to the bottom left corner and add the bottom left curve.
      CGPathAddLineToPoint(mutablePathRef, nil, clipSkRect.fLeft + bottomLeftRadii.x(),
                           clipSkRect.fBottom);
      CGPathAddCurveToPoint(mutablePathRef, nil, clipSkRect.fLeft, clipSkRect.fBottom,
                            clipSkRect.fLeft, clipSkRect.fBottom - bottomLeftRadii.y(),
                            clipSkRect.fLeft, clipSkRect.fBottom - bottomLeftRadii.y());
      // Move point vertically up to the top left corner and add the top left curve.
      CGPathAddLineToPoint(mutablePathRef, nil, clipSkRect.fLeft,
                           clipSkRect.fTop + topLeftRadii.y());
      CGPathAddCurveToPoint(mutablePathRef, nil, clipSkRect.fLeft, clipSkRect.fTop,
                            clipSkRect.fLeft + topLeftRadii.x(), clipSkRect.fTop,
                            clipSkRect.fLeft + topLeftRadii.x(), clipSkRect.fTop);
      CGPathCloseSubpath(mutablePathRef);

      pathRef = mutablePathRef;
      break;
    }
  }
  // TODO(cyanglaz): iOS does not seem to support hard edge on CAShapeLayer. It clearly stated that
  // the CAShaperLayer will be drawn antialiased. Need to figure out a way to do the hard edge
  // clipping on iOS.
  CAShapeLayer* clip = [[CAShapeLayer alloc] init];
  clip.path = pathRef;
  self.layer.mask = clip;
  CGPathRelease(pathRef);
}

- (void)setClip:(flutter::MutatorType)type
           rect:(const SkRect&)rect
          rrect:(const SkRRect&)rrect
           path:(const SkPath&)path {
  FML_CHECK(type == flutter::clip_rect || type == flutter::clip_rrect ||
            type == flutter::clip_path);
  switch (type) {
    case flutter::clip_rect:
      [self clipRect:rect];
      break;
    case flutter::clip_rrect:
      [self clipRRect:rrect];
      break;
    case flutter::clip_path:
      // TODO(cyanglaz): Add clip path
      break;
    default:
      break;
  }
}

// The ChildClippingView is as big as the FlutterView, we only want touches to be hit tested and
// consumed by this view if they are inside the smaller child view.
- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent*)event {
  for (UIView* view in self.subviews) {
    if ([view pointInside:[self convertPoint:point toView:view] withEvent:event]) {
      return YES;
    }
  }
  return NO;
}

@end
