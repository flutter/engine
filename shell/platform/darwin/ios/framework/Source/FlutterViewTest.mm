// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterView.h"

@interface FakeDelegate : FlutterEngine
@property(nonatomic) BOOL ensureSemanticsEnabledCalled;
@end

@implementation FakeDelegate

- (void)ensureSemanticsEnabled {
  _ensureSemanticsEnabledCalled = YES;
}

@end

@interface FlutterViewTest : XCTestCase
@end

@implementation FlutterViewTest

- (void)testFlutterViewEnableSemanticsWhenIsAccessibilityElementIsCalled {
  FakeDelegate* delegate = [[FakeDelegate alloc] initWithName:@"foobar"];
  FlutterView* view = [[FlutterView alloc] initWithDelegate:delegate opaque:NO];
  delegate.ensureSemanticsEnabledCalled = NO;
  XCTAssertFalse(view.isAccessibilityElement);
  XCTAssertTrue(delegate.ensureSemanticsEnabledCalled);
}

@end
