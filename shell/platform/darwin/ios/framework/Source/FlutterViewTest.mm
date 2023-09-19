// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterView.h"

@interface FakeDelegate : NSObject <FlutterViewEngineDelegate>
@property(nonatomic) BOOL callbackCalled;
@property(nonatomic, assign) BOOL isUsingImpeller;
@end

@implementation FakeDelegate {
  std::shared_ptr<flutter::FlutterPlatformViewsController> _platformViewsController;
}

- (instancetype)init {
  _callbackCalled = NO;
  _platformViewsController = std::shared_ptr<flutter::FlutterPlatformViewsController>(nullptr);
  return self;
}

- (flutter::Rasterizer::Screenshot)takeScreenshot:(flutter::Rasterizer::ScreenshotType)type
                                  asBase64Encoded:(BOOL)base64Encode {
  return {};
}

- (std::shared_ptr<flutter::FlutterPlatformViewsController>&)platformViewsController {
  return _platformViewsController;
}

- (void)flutterViewAccessibilityDidCall {
  _callbackCalled = YES;
}

@end

@interface FlutterViewTest : XCTestCase
@end

@implementation FlutterViewTest

- (void)testFlutterViewEnableSemanticsWhenIsAccessibilityElementIsCalled {
  FakeDelegate* delegate = [[[FakeDelegate alloc] init] autorelease];
  FlutterView* view = [[[FlutterView alloc] initWithDelegate:delegate opaque:NO
                                             enableWideGamut:NO] autorelease];
  delegate.callbackCalled = NO;
  XCTAssertFalse(view.isAccessibilityElement);
  XCTAssertTrue(delegate.callbackCalled);
}

- (void)testFlutterViewBackgroundColorIsNotNil {
  FakeDelegate* delegate = [[[FakeDelegate alloc] init] autorelease];
  FlutterView* view = [[[FlutterView alloc] initWithDelegate:delegate opaque:NO
                                             enableWideGamut:NO] autorelease];
  XCTAssertNotNil(view.backgroundColor);
}

- (void)testIgnoreWideColorWithoutImpeller {
  FakeDelegate* delegate = [[[FakeDelegate alloc] init] autorelease];
  delegate.isUsingImpeller = NO;
  FlutterView* view = [[[FlutterView alloc] initWithDelegate:delegate opaque:NO
                                             enableWideGamut:YES] autorelease];
  [view layoutSubviews];
  XCTAssertTrue([view.layer isKindOfClass:NSClassFromString(@"CAMetalLayer")]);
  CAMetalLayer* layer = (CAMetalLayer*)view.layer;
  XCTAssertEqual(layer.pixelFormat, MTLPixelFormatBGRA8Unorm);
}

- (void)testLayerOpacityIsPreservedYes {
  FakeDelegate* delegate = [[[FakeDelegate alloc] init] autorelease];
  FlutterView* view = [[[FlutterView alloc] initWithDelegate:delegate opaque:YES
                                             enableWideGamut:NO] autorelease];

  XCTAssertTrue(view.layer.opaque);
  CGColor* bgColor = view.layer.backgroundColor;
  CGColor* expectedColor = [UIColor.clearColor CGColor];
  XCTAssertEqual(bgColor, expectedColor);
}

- (void)testLayerOpacityIsPreservedNo {
  FakeDelegate* delegate = [[[FakeDelegate alloc] init] autorelease];
  FlutterView* view = [[[FlutterView alloc] initWithDelegate:delegate opaque:NO
                                             enableWideGamut:NO] autorelease];

  XCTAssertFalse(view.layer.opaque);
  CGColor* bgColor = view.layer.backgroundColor;
  CGColor* expectedColor = [UIColor.clearColor CGColor];
  XCTAssertEqual(bgColor, expectedColor);
}

@end
