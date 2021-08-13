// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/Flutter.h>
#import <XCTest/XCTest.h>

#import "AppDelegate.h"

@interface FfiPlatformChannelTest : XCTestCase
@end

@implementation FfiPlatformChannelTest

- (void)testCalls {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"test" project:nil];
  XCTAssertTrue([engine runWithEntrypoint:nil]);

  XCTestExpectation* ffiChannelCalled = [self expectationWithDescription:@"ffi channel called"];

  FlutterFFIChannel* ffiChannel =
      [[FlutterFFIChannel alloc] initWithName:@"ffi-platform-channel"
                              binaryMessenger:engine.binaryMessenger
                                        codec:[FlutterStandardMessageCodec sharedInstance]];
  [ffiChannel setMessageHandler:^id _Nullable(id _Nullable message) {
    [ffiChannelCalled fulfill];
    return nil;
  }];

  FlutterBasicMessageChannel* ffiChannelController = [[FlutterBasicMessageChannel alloc]
         initWithName:@"ffi-platform-channel-control"
      binaryMessenger:engine.binaryMessenger
                codec:[FlutterStandardMessageCodec sharedInstance]];
  sleep(5);
  [ffiChannelController sendMessage:nil];

  [self waitForExpectations:@[ ffiChannelCalled ] timeout:30.0];

  [engine destroyContext];
}

@end
