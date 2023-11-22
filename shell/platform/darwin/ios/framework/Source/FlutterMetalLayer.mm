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

@class FlutterTexture;
@class FlutterDrawable;

static MTLPixelFormat _defaultPixelFormat = MTLPixelFormatBGRA8Unorm;

@interface FlutterMetalLayer () {
  id<MTLDevice> _preferredDevice;
  CGSize _drawableSize;
  MTLPixelFormat _pixelFormat;

  NSUInteger _nextDrawableId;

  NSMutableSet<FlutterTexture*>* _availableTextures;
  NSUInteger _totalTextures;

  FlutterTexture* _front;

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

- (void)presentTexture:(FlutterTexture*)texture;
- (void)returnTexture:(FlutterTexture*)texture;

@end

@interface FlutterTexture : NSObject {
  id<MTLTexture> _texture;
  IOSurface* _surface;
  CFTimeInterval _presentedTime;
}

@property(readonly, nonatomic) id<MTLTexture> texture;
@property(readonly, nonatomic) IOSurface* surface;
@property(readwrite, nonatomic) CFTimeInterval presentedTime;

@end

@implementation FlutterTexture

@synthesize texture = _texture;
@synthesize surface = _surface;
@synthesize presentedTime = _presentedTime;

- (instancetype)initWithTexture:(id<MTLTexture>)texture surface:(IOSurface*)surface {
  if (self = [super init]) {
    _texture = texture;
    _surface = surface;
  }
  return self;
}

@end

@interface FlutterDrawable : NSObject <CAMetalDrawable> {
  FlutterTexture* _texture;
  __weak FlutterMetalLayer* _layer;
  NSUInteger _drawableId;
  BOOL _presented;
}

- (instancetype)initWithTexture:(FlutterTexture*)texture
                          layer:(FlutterMetalLayer*)layer
                     drawableId:(NSUInteger)drawableId;

@end

@implementation FlutterDrawable

- (instancetype)initWithTexture:(FlutterTexture*)texture
                          layer:(FlutterMetalLayer*)layer
                     drawableId:(NSUInteger)drawableId {
  if (self = [super init]) {
    _texture = texture;
    _layer = layer;
    _drawableId = drawableId;
  }
  return self;
}

- (id<MTLTexture>)texture {
  return self->_texture.texture;
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

- (CFTimeInterval)presentedTime {
  return 0;
}

- (void)present {
  [_layer presentTexture:self->_texture];
  self->_presented = YES;
}

- (void)dealloc {
  if (!_presented) {
    [_layer returnTexture:self->_texture];
  }
}

- (void)addPresentedHandler:(nonnull MTLDrawablePresentedHandler)block {
}

- (void)presentAtTime:(CFTimeInterval)presentationTime {
}

- (void)presentAfterMinimumDuration:(CFTimeInterval)duration {
}

@end

@implementation FlutterMetalLayer

@synthesize preferredDevice = _preferredDevice;
@synthesize device = _device;
@synthesize framebufferOnly = _framebufferOnly;
@synthesize colorspace = _colorspace;
@synthesize wantsExtendedDynamicRangeContent = _wantsExtendedDynamicRangeContent;

- (instancetype)init {
  if (self = [super init]) {
    _preferredDevice = MTLCreateSystemDefaultDevice();
    self.device = self.preferredDevice;
    _pixelFormat = _defaultPixelFormat;
    _availableTextures = [[NSMutableSet alloc] init];

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

- (MTLPixelFormat)pixelFormat {
  return _pixelFormat;
}

- (void)setPixelFormat:(MTLPixelFormat)pixelFormat {
  _pixelFormat = pixelFormat;

  // FlutterView updates pixel format on it's layer, but the overlay views
  // don't - they get correct pixel format right magically.
  _defaultPixelFormat = pixelFormat;
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
  [_availableTextures removeAllObjects];
  _front = nil;
  _totalTextures = 0;
  _drawableSize = drawableSize;
}

- (void)didEnterBackground:(id)notification {
  [_availableTextures removeAllObjects];
  _totalTextures = _front != nil ? 1 : 0;
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

- (FlutterTexture*)nextTexture {
  @synchronized(self) {
    if (_totalTextures < 3) {
      ++_totalTextures;
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
      FlutterTexture* flutterTexture = [[FlutterTexture alloc] initWithTexture:texture
                                                                       surface:surface];
      return flutterTexture;
    } else {
      // Make sure raster thread doesn't have too many drawables in flight.
      if (_availableTextures.count == 0) {
        CFTimeInterval start = CACurrentMediaTime();
        while (_availableTextures.count == 0 && CACurrentMediaTime() - start < 1.0) {
          usleep(100);
        }
        CFTimeInterval elapsed = CACurrentMediaTime() - start;
        if (_availableTextures.count == 0) {
          NSLog(@"Waited %f seconds for a drawable, giving up.", elapsed);
          return nil;
        } else {
          NSLog(@"Had to wait %f seconds for a drawable", elapsed);
        }
      }
      // Return first drawable that is not in use or the one that was presented
      // the longest time ago.
      FlutterTexture* res = nil;
      for (FlutterTexture* texture in _availableTextures) {
        if (!texture.surface.isInUse) {
          res = texture;
          break;
        }
        if (res == nil || texture.presentedTime < res.presentedTime) {
          res = texture;
        }
      }
      [_availableTextures removeObject:res];
      return res;
    }
  }
}

- (id<CAMetalDrawable>)nextDrawable {
  FlutterTexture* texture = [self nextTexture];
  if (texture == nil) {
    return nil;
  }
  FlutterDrawable* drawable = [[FlutterDrawable alloc] initWithTexture:texture
                                                                 layer:self
                                                            drawableId:_nextDrawableId++];
  return drawable;
}

- (void)presentOnMainThread:(FlutterTexture*)texture {
  // This is needed otherwise frame gets skipped on touch begin / end. Go figure.
  // Might also be placebo
  [self setNeedsDisplay];

  [CATransaction begin];
  [CATransaction setDisableActions:YES];
  self.contents = texture.surface;
  texture.presentedTime = CACurrentMediaTime();
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

- (void)presentTexture:(FlutterTexture*)texture {
  @synchronized(self) {
    if (_front != nil) {
      [_availableTextures addObject:_front];
    }
    _front = texture;
    if ([NSThread isMainThread]) {
      [self presentOnMainThread:texture];
    } else {
      dispatch_async(dispatch_get_main_queue(), ^{
        [self presentOnMainThread:texture];
      });
    }
  }
}

- (void)returnTexture:(FlutterTexture*)texture {
  @synchronized(self) {
    [_availableTextures addObject:texture];
  }
}

@end
