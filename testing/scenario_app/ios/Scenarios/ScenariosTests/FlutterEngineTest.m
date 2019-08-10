// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/Flutter.h>
#import <XCTest/XCTest.h>
#import "AppDelegate.h"

@interface FlutterEngineTest : XCTestCase
@end

@implementation FlutterEngineTest
XCTestExpectation* _isolateIdSetExpectation;

- (void)testIsolateId {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"test" project:nil];
  XCTAssertNil(engine.isolateId);
  _isolateIdSetExpectation = [self expectationWithDescription:@"Isolate ID set"];
  [engine addObserver:self forKeyPath:@"isolateId" options:0 context:nil];
  
  XCTAssertTrue([engine runWithEntrypoint:nil]);
  
  [self waitForExpectationsWithTimeout:30.0 handler:nil];

  XCTAssertNotNil(engine.isolateId);
  XCTAssertTrue([engine.isolateId hasPrefix:@"isolates/"]);
  
  [engine destroyContext];
  
  XCTAssertNil(engine.isolateId);
}

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context {
  if ([keyPath isEqualToString:@"isolateId"]) {
    [_isolateIdSetExpectation fulfill];
    _isolateIdSetExpectation = nil;
  }
}
@end
