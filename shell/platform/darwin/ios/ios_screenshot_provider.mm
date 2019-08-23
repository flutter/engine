// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_screenshot_provider.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkRefCnt.h"

namespace flutter {

sk_sp<SkImage> IOSScreenShotProvider::TakeScreenShotForView(UIView* view) {
  CGRect rect = [UIScreen mainScreen].bounds;

//  NSLog(@"view %@", view);
//  UIImage *image = nil;
//  if (@available(iOS 10, *)) {
//    UIGraphicsImageRenderer *renderer = [[UIGraphicsImageRenderer alloc] initWithSize:view.frame.size];
//    image = [renderer imageWithActions:^(UIGraphicsImageRendererContext * _Nonnull context) {
//      [view drawViewHierarchyInRect:view.frame afterScreenUpdates:NO];
//    }];
//  }

  //layer
  UIGraphicsBeginImageContextWithOptions(rect.size, NO, [UIScreen mainScreen].scale);
  //[view drawViewHierarchyInRect:rect afterScreenUpdates:NO];
  CGContext *ctx = UIGraphicsGetCurrentContext();

  CGContextTranslateCTM(ctx, view.frame.origin.x, view.frame.origin.y);

  [view.layer.presentationLayer renderInContext:ctx];

  UIImage* screenshot = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();

//  //mask
//  UIGraphicsBeginImageContextWithOptions(rect.size, NO, [UIScreen mainScreen].scale);
//  [view.layer.mask renderInContext:UIGraphicsGetCurrentContext()];
//
//  UIImage* maskImage = UIGraphicsGetImageFromCurrentImageContext();
//  UIGraphicsEndImageContext();
//
//  CGImageRef maskRef = maskImage.CGImage;
//  CGImageRef mask = CGImageMaskCreate(CGImageGetWidth(maskRef),
//                                      CGImageGetHeight(maskRef),
//                                      CGImageGetBitsPerComponent(maskRef),
//                                      CGImageGetBitsPerPixel(maskRef),
//                                      CGImageGetBytesPerRow(maskRef),
//                                      CGImageGetDataProvider(maskRef), NULL, false);
//
//  // Apply the mask to our source image
//  CGImageRef maskedImage = CGImageCreateWithMask(screenshot.CGImage, mask);
//  screenshot = [UIImage imageWithCGImage:maskedImage scale:screenshot.scale orientation:screenshot.imageOrientation];
//
//  CGImageRelease(mask);
//  CGImageRelease(maskRef);

  // converting to skimage
  CGImageRef imageRef = CGImageRetain(screenshot.CGImage);
  CFDataRef data = CGDataProviderCopyData(CGImageGetDataProvider(imageRef));

  size_t rowBtyes = CGImageGetBytesPerRow(imageRef);

  sk_sp<SkData> rasterData = SkData::MakeWithCopy(CFDataGetBytePtr(data), CFDataGetLength(data));

  const auto image_info =
      SkImageInfo::Make(CGImageGetWidth(imageRef), CGImageGetHeight(imageRef),
                        kBGRA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());

  sk_sp<SkImage> skImage = SkImage::MakeRasterData(image_info, rasterData, rowBtyes);

// CGImageRelease(maskedImage);
  CGImageRelease(imageRef);
  CFRelease(data);
  return skImage;
}
}
