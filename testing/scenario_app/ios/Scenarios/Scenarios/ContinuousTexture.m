// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ContinuousTexture.h"

@implementation ContinuousTexture

+ (void)registerWithRegistrar:(nonnull NSObject<FlutterPluginRegistrar> *)registrar {
  NSObject<FlutterTextureRegistry> *textureRegistry = [registrar textures];
  FlutterScenarioTestTexture *texture = [[FlutterScenarioTestTexture alloc] init];
  int64_t textureId = [textureRegistry registerTexture:texture];
  [NSTimer scheduledTimerWithTimeInterval:0.05 repeats:YES block:^(NSTimer * _Nonnull timer) {
    [textureRegistry textureFrameAvailable:textureId];
  }];
}

@end

@interface FlutterScenarioTestTexture()

@property (strong, nonatomic) UIImage *image;

@end

@implementation FlutterScenarioTestTexture

- (instancetype)init
{
  self = [super init];
  if (self) {
    UIGraphicsBeginImageContext(CGSizeMake(100, 100));
    [[UIColor yellowColor] setFill];
    UIRectFill(CGRectMake(0, 0, 100, 100));
    self.image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
  }
  return self;
}

- (CVPixelBufferRef _Nullable)copyPixelBuffer {
  return [self pixelBuffer];
}

- (CVPixelBufferRef)pixelBuffer
{
    NSDictionary *options = @{
                              (NSString*)kCVPixelBufferCGImageCompatibilityKey : @YES,
                              (NSString*)kCVPixelBufferCGBitmapContextCompatibilityKey : @YES,
                              (NSString*)kCVPixelBufferMetalCompatibilityKey: @YES
                              };
    size_t width = CGImageGetWidth(self.image.CGImage);
    size_t height =  CGImageGetHeight(self.image.CGImage);
    CVPixelBufferRef pxbuffer = NULL;
    CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault, width,
                                          height, kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef) options,
                        &pxbuffer);
    if (status!=kCVReturnSuccess) {
        NSLog(@"Operation failed");
    }
    NSParameterAssert(status == kCVReturnSuccess && pxbuffer != NULL);

    CVPixelBufferLockBaseAddress(pxbuffer, 0);
    void *pxdata = CVPixelBufferGetBaseAddress(pxbuffer);

    CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(pxdata, width,
                                                 height, 8, 4*width, rgbColorSpace,
                                                 kCGImageAlphaNoneSkipFirst);
    NSParameterAssert(context);

    CGContextDrawImage(context, CGRectMake(0, 0, width,
                                          height), self.image.CGImage);
    CGColorSpaceRelease(rgbColorSpace);
    CGContextRelease(context);

    CVPixelBufferUnlockBaseAddress(pxbuffer, 0);
    return pxbuffer;
}


@end
