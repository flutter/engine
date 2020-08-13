// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>
#include "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterDartProject_Internal.h"

FLUTTER_ASSERT_ARC

@interface FlutterDartProjectTest : XCTestCase
@end

@implementation FlutterDartProjectTest

- (void)setUp {
}

- (void)tearDown {
}

- (void)testOldGenHeapSizeSetting {
  FlutterDartProject* project = [[FlutterDartProject alloc] init];
  int64_t old_gen_heap_size = std::round([NSProcessInfo processInfo].physicalMemory / 2);
  XCTAssertEqual(project.settings.old_gen_heap_size, old_gen_heap_size);
}

@end
