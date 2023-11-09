// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterPlatformViews_Internal.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTouchInterceptingView_Test.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/SemanticsObject.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/SemanticsObjectTestMocks.h"

FLUTTER_ASSERT_NOT_ARC

@interface SemanticsObjectTestMRC : XCTestCase
@end

@implementation SemanticsObjectTestMRC

- (void)testFlutterPlatformViewSemanticsContainer {
  fml::WeakPtrFactory<flutter::MockAccessibilityBridge> factory(
      new flutter::MockAccessibilityBridge());
  fml::WeakPtr<flutter::MockAccessibilityBridge> bridge = factory.GetWeakPtr();
  FlutterTouchInterceptingView* platformView =
      [[[FlutterTouchInterceptingView alloc] init] autorelease];
  @autoreleasepool {
    FlutterPlatformViewSemanticsContainer* container =
        [[[FlutterPlatformViewSemanticsContainer alloc] initWithBridge:bridge
                                                                   uid:1
                                                          platformView:platformView] autorelease];
    XCTAssertEqualObjects(platformView.accessibilityContainer, container);
    XCTAssertEqual(platformView.retainCount, 2u);
  }
  // Check if there's no more strong references to `platformView` after container and platformView
  // are released.
  XCTAssertEqual(platformView.retainCount, 1u);
}

@end
