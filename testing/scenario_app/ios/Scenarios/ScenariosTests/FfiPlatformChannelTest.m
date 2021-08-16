// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/Flutter.h>
#import <XCTest/XCTest.h>

#import "AppDelegate.h"

@interface FfiPlatformChannelTest : XCTestCase
@end

@implementation FfiPlatformChannelTest

- (void)performTestCalls:(NSObject<FlutterBinaryMessenger>*)binaryMessenger
             expectation:(XCTestExpectation*)ffiChannelCalled {
  NSString* text = @"hello";
  NSData* payload = [text dataUsingEncoding:NSUTF8StringEncoding];
  FlutterFFIChannel* ffiChannel =
      [[FlutterFFIChannel alloc] initWithName:@"ffi-platform-channel"
                              binaryMessenger:binaryMessenger
                                        codec:[FlutterBinaryCodec sharedInstance]];
  [ffiChannel setMessageHandler:^id _Nullable(id _Nullable message) {
    NSString* receivedText = [NSString stringWithUTF8String:[message bytes]];
    XCTAssertEqualObjects(receivedText, text);
    [ffiChannelCalled fulfill];
    return nil;
  }];

  FlutterBasicMessageChannel* ffiChannelController =
      [[FlutterBasicMessageChannel alloc] initWithName:@"ffi-platform-channel-control"
                                       binaryMessenger:binaryMessenger
                                                 codec:[FlutterBinaryCodec sharedInstance]];
  [ffiChannelController sendMessage:payload];
}

- (void)testCalls {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"test" project:nil];
  XCTAssertTrue([engine runWithEntrypoint:nil]);
  XCTestExpectation* ffiChannelCalled = [self expectationWithDescription:@"ffi channel called"];
  FlutterBinaryMessengerConnection waitingForStatusConnection = [engine.binaryMessenger
      setMessageHandlerOnChannel:@"waiting_for_status"
            binaryMessageHandler:^(NSData* message, FlutterBinaryReply reply) {
              [self performTestCalls:engine.binaryMessenger expectation:ffiChannelCalled];
            }];
  [self waitForExpectations:@[ ffiChannelCalled ] timeout:30.0];
  [engine.binaryMessenger cleanupConnection:waitingForStatusConnection];
  [engine destroyContext];
}

@end
