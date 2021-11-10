// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterEngineGroup.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterEngine_Test.h"

FLUTTER_ASSERT_ARC

@interface FlutterEngineGroup ()
- (FlutterEngine*)makeEngine;
@end

@interface FlutterEngineGroupTest : XCTestCase
@end

@implementation FlutterEngineGroupTest

- (void)testMake {
  FlutterEngineGroup* group = [[FlutterEngineGroup alloc] initWithName:@"foo" project:nil];
  FlutterEngine* engine = [group makeEngineWithEntrypoint:nil libraryURI:nil];
  XCTAssertNotNil(engine);
}

- (void)testSpawn {
  FlutterEngineGroup* group = [[FlutterEngineGroup alloc] initWithName:@"foo" project:nil];
  FlutterEngine* spawner = [group makeEngineWithEntrypoint:nil libraryURI:nil];
  spawner.isGpuDisabled = YES;
  FlutterEngine* spawnee = [group makeEngineWithEntrypoint:nil libraryURI:nil];
  XCTAssertNotNil(spawner);
  XCTAssertNotNil(spawnee);
  XCTAssertEqual(&spawner.threadHost, &spawnee.threadHost);
  XCTAssertEqual(spawner.isGpuDisabled, spawnee.isGpuDisabled);
}

- (void)testDeleteLastEngine {
  FlutterEngineGroup* group = [[FlutterEngineGroup alloc] initWithName:@"foo" project:nil];
  @autoreleasepool {
    FlutterEngine* spawner = [group makeEngineWithEntrypoint:nil libraryURI:nil];
    XCTAssertNotNil(spawner);
  }
  FlutterEngine* spawnee = [group makeEngineWithEntrypoint:nil libraryURI:nil];
  XCTAssertNotNil(spawnee);
}

- (void)testCustomEntrypoint {
  FlutterEngineGroup* group = OCMPartialMock([[FlutterEngineGroup alloc] initWithName:@"foo"
                                                                              project:nil]);
  FlutterEngine* mockEngine = OCMClassMock([FlutterEngine class]);
  OCMStub([group makeEngine]).andReturn(mockEngine);
  OCMStub([mockEngine spawnWithEntrypoint:[OCMArg any]
                               libraryURI:[OCMArg any]
                             initialRoute:[OCMArg any]
                           entrypointArgs:[OCMArg any]])
      .andReturn(OCMClassMock([FlutterEngine class]));
  FlutterEngine* spawner = [group makeEngineWithEntrypoint:@"firstEntrypoint"
                                                libraryURI:@"firstLibraryURI"];
  XCTAssertNotNil(spawner);
  OCMVerify([spawner runWithEntrypoint:@"firstEntrypoint"
                            libraryURI:@"firstLibraryURI"
                          initialRoute:nil
                        entrypointArgs:nil]);

  FlutterEngine* spawnee = [group makeEngineWithEntrypoint:@"secondEntrypoint"
                                                libraryURI:@"secondLibraryURI"];
  XCTAssertNotNil(spawnee);
  OCMVerify([spawner spawnWithEntrypoint:@"secondEntrypoint"
                              libraryURI:@"secondLibraryURI"
                            initialRoute:nil
                          entrypointArgs:nil]);
}

- (void)testCustomInitialRoute {
  FlutterEngineGroup* group = OCMPartialMock([[FlutterEngineGroup alloc] initWithName:@"foo"
                                                                              project:nil]);
  FlutterEngine* mockEngine = OCMClassMock([FlutterEngine class]);
  OCMStub([group makeEngine]).andReturn(mockEngine);
  OCMStub([mockEngine spawnWithEntrypoint:[OCMArg any]
                               libraryURI:[OCMArg any]
                             initialRoute:[OCMArg any]
                           entrypointArgs:[OCMArg any]])
      .andReturn(OCMClassMock([FlutterEngine class]));
  FlutterEngine* spawner = [group makeEngineWithEntrypoint:nil libraryURI:nil initialRoute:@"foo"];
  XCTAssertNotNil(spawner);
  OCMVerify([spawner runWithEntrypoint:nil libraryURI:nil initialRoute:@"foo" entrypointArgs:nil]);

  FlutterEngine* spawnee = [group makeEngineWithEntrypoint:nil libraryURI:nil initialRoute:@"bar"];
  XCTAssertNotNil(spawnee);
  OCMVerify([spawner spawnWithEntrypoint:nil
                              libraryURI:nil
                            initialRoute:@"bar"
                          entrypointArgs:nil]);
}

- (void)testCustomEntrypointArgs {
  NSArray* firstEntrypointArgs = @[ @"foo", @"first" ];
  FlutterEngineGroup* group = OCMPartialMock([[FlutterEngineGroup alloc] initWithName:@"foo"
                                                                              project:nil]);
  FlutterEngine* mockEngine = OCMClassMock([FlutterEngine class]);
  OCMStub([group makeEngine]).andReturn(mockEngine);
  OCMStub([mockEngine spawnWithEntrypoint:[OCMArg any]
                               libraryURI:[OCMArg any]
                             initialRoute:[OCMArg any]
                           entrypointArgs:[OCMArg any]])
      .andReturn(OCMClassMock([FlutterEngine class]));
  FlutterEngine* spawner = [group makeEngineWithEntrypoint:nil
                                                libraryURI:nil
                                              initialRoute:nil
                                            entrypointArgs:firstEntrypointArgs];
  XCTAssertNotNil(spawner);
  OCMVerify([spawner runWithEntrypoint:nil
                            libraryURI:nil
                          initialRoute:nil
                        entrypointArgs:firstEntrypointArgs]);

  NSArray* secondEntrypointArgs = @[ @"bar", @"second" ];
  FlutterEngine* spawnee = [group makeEngineWithEntrypoint:nil
                                                libraryURI:nil
                                              initialRoute:nil
                                            entrypointArgs:secondEntrypointArgs];
  XCTAssertNotNil(spawnee);
  OCMVerify([spawner spawnWithEntrypoint:nil
                              libraryURI:nil
                            initialRoute:nil
                          entrypointArgs:secondEntrypointArgs]);
}

@end
