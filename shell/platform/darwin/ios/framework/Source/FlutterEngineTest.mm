// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterEngine_Internal.h"

@interface FlutteEngineTest : XCTestCase
@end

@implementation FlutteEngineTest

- (void)setUp {
}

- (void)tearDown {
}

- (void)testCreate {
  FlutterEngine* engine = [[[FlutterEngine alloc] initWithName:@"foobar" project:nil] autorelease];
  XCTAssertNotNil(engine);
}


@end
