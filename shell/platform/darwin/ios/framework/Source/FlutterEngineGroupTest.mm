// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterEngineGroup.h"

FLUTTER_ASSERT_ARC

@interface FlutterEngineGroupTest : XCTestCase
@end

@implementation FlutterEngineGroupTest

- (void)testMake {
  FlutterEngineGroup* group = [[FlutterEngineGroup alloc] initWithName:@"foo" project:nil];
  FlutterEngine* engine = [group makeEngineWithEntrypoint:nil];
  XCTAssertNotNil(engine);
}

- (void)testSpawn {
  FlutterEngineGroup* group = [[FlutterEngineGroup alloc] initWithName:@"foo" project:nil];
  FlutterEngine* spawner = [group makeEngineWithEntrypoint:nil];
  FlutterEngine* spawnee = [group makeEngineWithEntrypoint:nil];
  XCTAssertNotNil(spawner);
  XCTAssertNotNil(spawnee);
}

- (void)testDeleteLastEngine {
  FlutterEngineGroup* group = [[FlutterEngineGroup alloc] initWithName:@"foo" project:nil];
  @autoreleasepool {
    FlutterEngine* spawner = [group makeEngineWithEntrypoint:nil];
    XCTAssertNotNil(spawner);
  }
  FlutterEngine* spawnee = [group makeEngineWithEntrypoint:nil];
  XCTAssertNotNil(spawnee);
}

@end
