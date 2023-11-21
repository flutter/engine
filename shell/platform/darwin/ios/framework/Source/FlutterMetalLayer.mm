// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <IOSurface/IOSurfaceObjC.h>
#include <Metal/Metal.h>
#include <UIKit/UIKit.h>

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterMetalLayer.h"

@interface DisplayLinkManager : NSObject
@property(class, nonatomic, readonly) BOOL maxRefreshRateEnabledOnIPhone;
+ (double)displayRefreshRate;
@end

@class FlutterDrawable;

extern CFTimeInterval display_link_target;

@interface FlutterMetalLayer () {
  id<MTLDevice> _preferredDevice;
  CGSize _drawableSize;

  NSUInteger _nextDrawableId;

  NSMutableSet<FlutterDrawable*>* _availableDrawables;
  NSUInteger _totalDrawables;

  FlutterDrawable* _front;

  // There must be a CADisplayLink scheduled *on main thread* otherwise
  // core animation only updates layers 60 times a second.
  CADisplayLink* _displayLink;
  NSUInteger _displayLinkPauseCountdown;

  // Used to track whether the content was set during this display link.
  // When unlocking phone the layer (main thread) display link and raster thread
  // display link get out of sync for several seconds. Even worse, layer display
  // link does not seem to reflect actual vsync. Forcing the layer link
  // to max rate (instead range) temporarily seems to fix the issue.
  BOOL _didSetContentsDuringThisDisplayLinkPeriod;

  // Whether layer displayLink is forced to max rate.
  BOOL _displayLinkForcedMaxRate;
}

- (void)presentDrawable:(FlutterDrawable*)drawable;

@end

@interface FlutterDrawable : NSObject <CAMetalDrawable> {
  id<MTLTexture> _texture;
  __weak FlutterMetalLayer* _layer;
  NSUInteger _drawableId;
  IOSurface* _surface;
  CFTimeInterval _presentedTime;
}

- (instancetype)initWithTexture:(id<MTLTexture>)texture
                          layer:(FlutterMetalLayer*)layer
                     drawableId:(NSUInteger)drawableId
                        surface:(IOSurface*)surface;

@property(readonly) IOSurface* surface;
@property(readwrite, nonatomic) CFTimeInterval presentedTime;

@end

@implementation FlutterMetalLayer

@synthesize preferredDevice = _preferredDevice;
@synthesize device = _device;
@synthesize pixelFormat = _pixelFormat;
@synthesize framebufferOnly = _framebufferOnly;
@synthesize colorspace = _colorspace;
@synthesize wantsExtendedDynamicRangeContent = _wantsExtendedDynamicRangeContent;

- (instancetype)init {
  if (self = [super init]) {
    _preferredDevice = MTLCreateSystemDefaultDevice();
    self.device = self.preferredDevice;
    _availableDrawables = [[NSMutableSet alloc] init];

    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(onDisplayLink:)];
    [self setMaxRefreshRate:[DisplayLinkManager displayRefreshRate] forceMax:NO];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(didEnterBackground:)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
  }
  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)setMaxRefreshRate:(double)refreshRate forceMax:(BOOL)forceMax {
  if (!DisplayLinkManager.maxRefreshRateEnabledOnIPhone) {
    return;
  }
  double maxFrameRate = fmax(refreshRate, 60);
  double minFrameRate = fmax(maxFrameRate / 2, 60);
  if (@available(iOS 15.0, *)) {
    _displayLink.preferredFrameRateRange =
        CAFrameRateRangeMake(forceMax ? maxFrameRate : minFrameRate, maxFrameRate, maxFrameRate);
  } else {
    _displayLink.preferredFramesPerSecond = maxFrameRate;
  }
}

- (void)onDisplayLink:(CADisplayLink*)link {
  _didSetContentsDuringThisDisplayLinkPeriod = NO;
  // Do not pause immediately, this seems to prevent 120hz while touching.
  if (_displayLinkPauseCountdown == 3) {
    _displayLink.paused = YES;
    if (_displayLinkForcedMaxRate) {
      [self setMaxRefreshRate:[DisplayLinkManager displayRefreshRate] forceMax:NO];
      _displayLinkForcedMaxRate = NO;
    }
  } else {
    ++_displayLinkPauseCountdown;
  }
}

- (BOOL)isKindOfClass:(Class)aClass {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"
  // Pretend that we're a CAMetalLayer so that the rest of Flutter plays along
  if ([aClass isEqual:[CAMetalLayer class]]) {
    return YES;
  }
#pragma clang diagnostic pop
  return [super isKindOfClass:aClass];
}

- (void)setDrawableSize:(CGSize)drawableSize {
  [_availableDrawables removeAllObjects];
  _front = nil;
  _totalDrawables = 0;
  _drawableSize = drawableSize;
}

- (void)didEnterBackground:(id)notification {
  [_availableDrawables removeAllObjects];
  _totalDrawables = _front != nil ? 1 : 0;
  _displayLink.paused = YES;
}

- (CGSize)drawableSize {
  return _drawableSize;
}

- (IOSurface*)createIOSurface {
  unsigned pixelFormat;
  unsigned bytesPerElement;
  if (self.pixelFormat == MTLPixelFormatRGBA16Float) {
    pixelFormat = kCVPixelFormatType_64RGBAHalf;
    bytesPerElement = 8;
  } else {
    pixelFormat = kCVPixelFormatType_32BGRA;
    bytesPerElement = 4;
  }
  size_t bytesPerRow =
      IOSurfaceAlignProperty(kIOSurfaceBytesPerRow, _drawableSize.width * bytesPerElement);
  size_t totalBytes =
      IOSurfaceAlignProperty(kIOSurfaceAllocSize, _drawableSize.height * bytesPerRow);
  NSDictionary* options = @{
    (id)kIOSurfaceWidth : @(_drawableSize.width),
    (id)kIOSurfaceHeight : @(_drawableSize.height),
    (id)kIOSurfacePixelFormat : @(pixelFormat),
    (id)kIOSurfaceBytesPerElement : @(bytesPerElement),
    (id)kIOSurfaceBytesPerRow : @(bytesPerRow),
    (id)kIOSurfaceAllocSize : @(totalBytes),
  };

  IOSurfaceRef res = IOSurfaceCreate((CFDictionaryRef)options);
  if (self.colorspace != nil) {
    CFStringRef name = CGColorSpaceGetName(self.colorspace);
    IOSurfaceSetValue(res, CFSTR("IOSurfaceColorSpace"), name);
  } else {
    IOSurfaceSetValue(res, CFSTR("IOSurfaceColorSpace"), kCGColorSpaceSRGB);
  }
  return (__bridge_transfer IOSurface*)res;
}

- (id<CAMetalDrawable>)nextDrawable {
  @synchronized(self) {
    // IOSurface.isInUse to determine when compositor is done with the surface.
    // That a rather blunt instrument and we are probably waiting longer than
    // we really need to. With triple buffering at 120Hz that results in about
    // 2-3 milliseconds wait time at beginning of display link callback.
    // With four buffers this number gets close to zero.
    if (_totalDrawables < 3) {
      ++_totalDrawables;
      IOSurface* surface = [self createIOSurface];
      MTLTextureDescriptor* textureDescriptor =
          [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:_pixelFormat
                                                             width:_drawableSize.width
                                                            height:_drawableSize.height
                                                         mipmapped:NO];
      textureDescriptor.usage =
          MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget | MTLTextureUsageShaderWrite;
      id<MTLTexture> texture = [self.device newTextureWithDescriptor:textureDescriptor
                                                           iosurface:(__bridge IOSurfaceRef)surface
                                                               plane:0];
      FlutterDrawable* drawable = [[FlutterDrawable alloc] initWithTexture:texture
                                                                     layer:self
                                                                drawableId:_nextDrawableId++
                                                                   surface:surface];
      return drawable;
    } else {
      // Return first drawable that is not in use or the one that was presented
      // the longest time ago.
      FlutterDrawable* res = nil;
      for (FlutterDrawable* drawable in _availableDrawables) {
        if (!drawable.surface.isInUse) {
          res = drawable;
          break;
        }
        if (res == nil || drawable.presentedTime < res.presentedTime) {
          res = drawable;
        }
      }
      if (res != nil) {
        [_availableDrawables removeObject:res];
      }
      return res;
    }
  }
}

- (void)presentOnMainThread:(FlutterDrawable*)drawable {
  // This is needed otherwise frame gets skipped on touch begin / end. Go figure.
  // Might also be placebo
  [self setNeedsDisplay];

  [CATransaction begin];
  [CATransaction setDisableActions:YES];
  self.contents = drawable.surface;
  drawable.presentedTime = CACurrentMediaTime();
  [CATransaction commit];
  _displayLink.paused = NO;
  _displayLinkPauseCountdown = 0;
  if (_didSetContentsDuringThisDisplayLinkPeriod) {
    _didSetContentsDuringThisDisplayLinkPeriod = YES;
  } else if (!_displayLinkForcedMaxRate) {
    _displayLinkForcedMaxRate = YES;
    [self setMaxRefreshRate:[DisplayLinkManager displayRefreshRate] forceMax:YES];
  }
}

- (void)presentDrawable:(FlutterDrawable*)drawable {
  @synchronized(self) {
    if (_front != nil) {
      [_availableDrawables addObject:_front];
    }
    _front = drawable;
    dispatch_async(dispatch_get_main_queue(), ^{
      [self presentOnMainThread:drawable];
    });
  }
}

@end

@implementation FlutterDrawable

@synthesize presentedTime = _presentedTime;

- (instancetype)initWithTexture:(id<MTLTexture>)texture
                          layer:(FlutterMetalLayer*)layer
                     drawableId:(NSUInteger)drawableId
                        surface:(IOSurface*)surface {
  if (self = [super init]) {
    _texture = texture;
    _layer = layer;
    _drawableId = drawableId;
    _surface = surface;
  }
  return self;
}

- (id<MTLTexture>)texture {
  return self->_texture;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"
- (CAMetalLayer*)layer {
  return (id)self->_layer;
}
#pragma clang diagnostic pop

- (NSUInteger)drawableID {
  return self->_drawableId;
}

- (void)present {
  [_layer presentDrawable:self];
}

- (void)addPresentedHandler:(nonnull MTLDrawablePresentedHandler)block {
}

- (void)presentAtTime:(CFTimeInterval)presentationTime {
}

- (void)presentAfterMinimumDuration:(CFTimeInterval)duration {
}

@end
