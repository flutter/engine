// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/Flutter.h>
#import <XCTest/XCTest.h>
#import "AppDelegate.h"

@interface FlutterViewControllerTest : XCTestCase
@property(nonatomic, strong) FlutterViewController* flutterViewController;
@end

@implementation FlutterViewControllerTest

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;
}

- (void)tearDown {
  if (self.flutterViewController) {
    [self.flutterViewController removeFromParentViewController];
  }
  [super tearDown];
}

- (void)testFirstFrameCallback {
  XCTestExpectation* firstFrameRendered = [self expectationWithDescription:@"firstFrameRendered"];

  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"test" project:nil];
  [engine runWithEntrypoint:nil];
  self.flutterViewController = [[FlutterViewController alloc] initWithEngine:engine
                                                                     nibName:nil
                                                                      bundle:nil];

  XCTAssertFalse(self.flutterViewController.isDisplayingFlutterUI);

  XCTestExpectation* displayingFlutterUIExpectation =
      [self keyValueObservingExpectationForObject:self.flutterViewController
                                          keyPath:@"displayingFlutterUI"
                                    expectedValue:@YES];
  displayingFlutterUIExpectation.assertForOverFulfill = YES;

  [self.flutterViewController setFlutterViewDidRenderCallback:^{
    [firstFrameRendered fulfill];
  }];

  AppDelegate* appDelegate = (AppDelegate*)UIApplication.sharedApplication.delegate;
  UIViewController* rootVC = appDelegate.window.rootViewController;
  [rootVC presentViewController:self.flutterViewController animated:NO completion:nil];

  [self waitForExpectationsWithTimeout:30.0 handler:nil];
}

- (void)testSettingInitialRoute {
  self.flutterViewController = [[FlutterViewController alloc] initWithProject:nil
                                                            withInitialRoute:@"myCustomInitialRoute"
                                                                     nibName:nil
                                                                      bundle:nil];

  FlutterBinaryMessenger* binaryMessenger = self.flutterViewController.binaryMessenger;

  [binaryMessenger setMessageHandlerOnChannel:@"waiting_for_status"
                              binaryMessageHandler:^(NSData* message, FlutterBinaryReply reply) {
                                FlutterMethodChannel* channel = [FlutterMethodChannel
                                    methodChannelWithName:@"driver"
                                          binaryMessenger:self.binaryMessenger
                                                    codec:[FlutterJSONMethodCodec sharedInstance]];
                                [channel invokeMethod:@"set_scenario"
                                            arguments:@{@"name" : @"initial_route_reply"}];

  XCTestExpectation* customInitialRouteSet = [self expectationWithDescription:@"Custom initial route was set on the Dart side"];
  [binaryMessenger setMessageHandlerOnChannel:@"initial_route_test_channel"
                              binaryMessageHandler:^(NSData* message, FlutterBinaryReply reply) {
                                  [engine.binaryMessenger
              NSDictionary* dict = [NSJSONSerialization JSONObjectWithData:message
                                                                   options:0
                                                                     error:nil];
              NSString* initialRoute = dict[@"method"];
              if ([initialRoute isEqualToString:@"myCustomInitialRoute"]) {
                [customInitialRouteSet fulfill];
              } else {
                XCTFail(@"Expected initial route to be set to myCustomInitialRoute. Was set to %@ instead", initialRoute);
              }
                              }];

  AppDelegate* appDelegate = (AppDelegate*)UIApplication.sharedApplication.delegate;
  UIViewController* rootVC = appDelegate.window.rootViewController;
  [rootVC presentViewController:self.flutterViewController animated:NO completion:nil];

  [self waitForExpectationsWithTimeout:30.0 handler:nil];
}

@end
