// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterMetalLayer.h"

@interface FlutterMetalLayerTest : XCTestCase
@end

@interface TestFlutterMetalLayerView : UIView
@end

@implementation TestFlutterMetalLayerView

+ (Class)layerClass {
  return [FlutterMetalLayer class];
}

@end

@implementation FlutterMetalLayerTest

- (FlutterMetalLayer*)addMetalLayer {
  TestFlutterMetalLayerView* view =
      [[TestFlutterMetalLayerView alloc] initWithFrame:CGRectMake(0, 0, 100, 100)];
  FlutterMetalLayer* layer = (FlutterMetalLayer*)view.layer;
  layer.drawableSize = CGSizeMake(100, 100);
  [UIApplication.sharedApplication.keyWindow addSubview:view];
  return layer;
}

- (void)removeMetalLayer:(FlutterMetalLayer*)layer {
  [(UIView*)layer.delegate removeFromSuperview];
}

/// Waits until compositor picks up front surface.
- (void)waitUntilFrontSurfaceIsInUse:(FlutterMetalLayer*)layer {
  IOSurfaceRef surface = (__bridge IOSurfaceRef)layer.contents;
  while (!IOSurfaceIsInUse(surface)) {
    [NSRunLoop.mainRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.001]];
  }
}

- (void)testFlip {
  FlutterMetalLayer* layer = [self addMetalLayer];

  id<MTLTexture> t1, t2, t3;

  id<CAMetalDrawable> drawable = [layer nextDrawable];
  t1 = drawable.texture;
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];
  XCTAssertTrue(IOSurfaceIsInUse(t1.iosurface));

  drawable = [layer nextDrawable];
  t2 = drawable.texture;
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];
  XCTAssertTrue(IOSurfaceIsInUse(t2.iosurface));

  drawable = [layer nextDrawable];
  t3 = drawable.texture;
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];
  XCTAssertTrue(IOSurfaceIsInUse(t3.iosurface));

  // If there was no frame drop, layer should return oldest presented
  // texture.
  drawable = [layer nextDrawable];
  XCTAssertEqual(drawable.texture, t1);
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];

  drawable = [layer nextDrawable];
  XCTAssertEqual(drawable.texture, t2);
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];

  drawable = [layer nextDrawable];
  XCTAssertEqual(drawable.texture, t3);
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];

  drawable = [layer nextDrawable];
  XCTAssertEqual(drawable.texture, t1);
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];

  [self removeMetalLayer:layer];
}

- (void)testFlipWithDroppedFrame {
  FlutterMetalLayer* layer = [self addMetalLayer];

  id<MTLTexture> t1, t2, t3;

  id<CAMetalDrawable> drawable = [layer nextDrawable];
  t1 = drawable.texture;
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];
  XCTAssertTrue(IOSurfaceIsInUse(t1.iosurface));

  drawable = [layer nextDrawable];
  t2 = drawable.texture;
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];
  XCTAssertTrue(IOSurfaceIsInUse(t2.iosurface));

  drawable = [layer nextDrawable];
  t3 = drawable.texture;
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];
  XCTAssertTrue(IOSurfaceIsInUse(t3.iosurface));

  // Here the drawable is presented, but immediately replaced by another drawable
  // (before the compositor has a chance to pick it up). This should result
  // in same drawable returned in next call to nextDrawable.
  drawable = [layer nextDrawable];
  XCTAssertEqual(drawable.texture, t1);
  XCTAssertFalse(IOSurfaceIsInUse(drawable.texture.iosurface));
  [drawable present];

  drawable = [layer nextDrawable];
  XCTAssertEqual(drawable.texture, t2);
  [drawable present];
  [self waitUntilFrontSurfaceIsInUse:layer];

  // Next drawable should be t1, since it was never picked up by compositor.
  drawable = [layer nextDrawable];
  XCTAssertEqual(drawable.texture, t1);

  [self removeMetalLayer:layer];
}

- (void)testDroppedDrawableReturnsTextureToPool {
  FlutterMetalLayer* layer = [self addMetalLayer];
  // FlutterMetalLayer will keep creating new textures until it has 3.
  @autoreleasepool {
    for (int i = 0; i < 3; ++i) {
      id<CAMetalDrawable> drawable = [layer nextDrawable];
      XCTAssertNotNil(drawable);
    }
  }
  id<MTLTexture> texture;
  {
    @autoreleasepool {
      id<CAMetalDrawable> drawable = [layer nextDrawable];
      XCTAssertNotNil(drawable);
      texture = (id<MTLTexture>)drawable.texture;
      // Dropping the drawable must return texture to pool, so
      // next drawable should return the same texture.
    }
  }
  {
    id<CAMetalDrawable> drawable = [layer nextDrawable];
    XCTAssertEqual(texture, drawable.texture);
  }

  [self removeMetalLayer:layer];
}

- (void)testLayerLimitsDrawableCount {
  FlutterMetalLayer* layer = [self addMetalLayer];

  id<CAMetalDrawable> d1 = [layer nextDrawable];
  id<CAMetalDrawable> d2 = [layer nextDrawable];
  id<CAMetalDrawable> d3 = [layer nextDrawable];
  XCTAssertNotNil(d3);

  // Layer should not return more than 3 drawables.
  id<CAMetalDrawable> d4 = [layer nextDrawable];
  XCTAssertNil(d4);

  [d1 present];

  // Still no drawable, until the front buffer returns to pool
  id<CAMetalDrawable> d5 = [layer nextDrawable];
  XCTAssertNil(d5);

  [d2 present];
  id<CAMetalDrawable> d6 = [layer nextDrawable];
  XCTAssertNotNil(d6);

  [self removeMetalLayer:layer];
}

@end
