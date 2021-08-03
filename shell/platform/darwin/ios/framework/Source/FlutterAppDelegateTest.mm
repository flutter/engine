// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterAppDelegate.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterViewController.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterAppDelegate_Test.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterEngine_Test.h"

FLUTTER_ASSERT_ARC

@interface FlutterAppDelegateTest : XCTestCase
@end

@implementation FlutterAppDelegateTest

- (void)testLaunchUrl {
  FlutterAppDelegate* appDelegate = [[FlutterAppDelegate alloc] init];
  FlutterViewController* viewController = OCMClassMock([FlutterViewController class]);
  FlutterEngine* engine = OCMClassMock([FlutterEngine class]);
  FlutterMethodChannel* navigationChannel = OCMClassMock([FlutterMethodChannel class]);
  OCMStub([engine navigationChannel]).andReturn(navigationChannel);
  OCMStub([viewController engine]).andReturn(engine);
  // Set blockNoInvoker to a strong local to retain to end of scope.
  id blockNoInvoker = [OCMArg invokeBlockWithArgs:@NO, nil];
  OCMStub([engine waitForFirstFrame:3.0 callback:blockNoInvoker]);
  appDelegate.rootFlutterViewControllerGetter = ^{
    return viewController;
  };
  NSURL* url = [NSURL URLWithString:@"http://myApp/custom/route?query=test"];
  BOOL result = [appDelegate openURL:url
                     infoPlistGetter:^NSDictionary*() {
                       return @{@"FlutterDeepLinkingEnabled" : @YES};
                     }];
  XCTAssertTrue(result);
  OCMVerify([navigationChannel invokeMethod:@"pushRoute" arguments:@"/custom/route?query=test"]);
}

- (void)testLaunchUrlWithQueryParameterAndFragment {
  FlutterAppDelegate* appDelegate = [[FlutterAppDelegate alloc] init];
  FlutterViewController* viewController = OCMClassMock([FlutterViewController class]);
  FlutterEngine* engine = OCMClassMock([FlutterEngine class]);
  FlutterMethodChannel* navigationChannel = OCMClassMock([FlutterMethodChannel class]);
  OCMStub([engine navigationChannel]).andReturn(navigationChannel);
  OCMStub([viewController engine]).andReturn(engine);
  // Set blockNoInvoker to a strong local to retain to end of scope.
  id blockNoInvoker = [OCMArg invokeBlockWithArgs:@NO, nil];
  OCMStub([engine waitForFirstFrame:3.0 callback:blockNoInvoker]);
  appDelegate.rootFlutterViewControllerGetter = ^{
    return viewController;
  };
  NSURL* url = [NSURL URLWithString:@"http://myApp/custom/route?query=test#fragment"];
  BOOL result = [appDelegate openURL:url
                     infoPlistGetter:^NSDictionary*() {
                       return @{@"FlutterDeepLinkingEnabled" : @YES};
                     }];
  XCTAssertTrue(result);
  OCMVerify([navigationChannel invokeMethod:@"pushRoute"
                                  arguments:@"/custom/route?query=test#fragment"]);
}

- (void)testLaunchUrlWithFragmentNoQueryParameter {
  FlutterAppDelegate* appDelegate = [[FlutterAppDelegate alloc] init];
  FlutterViewController* viewController = OCMClassMock([FlutterViewController class]);
  FlutterEngine* engine = OCMClassMock([FlutterEngine class]);
  FlutterMethodChannel* navigationChannel = OCMClassMock([FlutterMethodChannel class]);
  OCMStub([engine navigationChannel]).andReturn(navigationChannel);
  OCMStub([viewController engine]).andReturn(engine);
  // Set blockNoInvoker to a strong local to retain to end of scope.
  id blockNoInvoker = [OCMArg invokeBlockWithArgs:@NO, nil];
  OCMStub([engine waitForFirstFrame:3.0 callback:blockNoInvoker]);
  appDelegate.rootFlutterViewControllerGetter = ^{
    return viewController;
  };
  NSURL* url = [NSURL URLWithString:@"http://myApp/custom/route#fragment"];
  BOOL result = [appDelegate openURL:url
                     infoPlistGetter:^NSDictionary*() {
                       return @{@"FlutterDeepLinkingEnabled" : @YES};
                     }];
  XCTAssertTrue(result);
  OCMVerify([navigationChannel invokeMethod:@"pushRoute" arguments:@"/custom/route#fragment"]);
}

#pragma mark - Deep linking

- (void)testUniversalLinkWebBrowserUrl {
  FlutterAppDelegate* appDelegate = [[FlutterAppDelegate alloc] init];
  FlutterViewController* viewController = OCMClassMock([FlutterViewController class]);
  FlutterEngine* engine = OCMClassMock([FlutterEngine class]);
  FlutterMethodChannel* navigationChannel = OCMClassMock([FlutterMethodChannel class]);
  OCMStub([engine navigationChannel]).andReturn(navigationChannel);
  OCMStub([viewController engine]).andReturn(engine);
  // Set blockArg to a strong local to retain to end of scope.
  id blockArg = [OCMArg invokeBlockWithArgs:@NO, nil];
  OCMStub([engine waitForFirstFrame:3.0 callback:blockArg]);
  appDelegate.rootFlutterViewControllerGetter = ^{
    return viewController;
  };
  NSUserActivity* userActivity =
      [[NSUserActivity alloc] initWithActivityType:NSUserActivityTypeBrowsingWeb];
  userActivity.webpageURL = [NSURL URLWithString:@"http://myApp/custom/route?query=test"];
  BOOL result = [appDelegate
               application:[UIApplication sharedApplication]
      continueUserActivity:userActivity
        restorationHandler:^(NSArray<id<UIUserActivityRestoring>>* __nullable restorableObjects){
        }];
  XCTAssertFalse(result);
  OCMReject([navigationChannel invokeMethod:OCMOCK_ANY arguments:OCMOCK_ANY]);
}

- (void)testUniversalLinkPushRoute {
  id mockBundle = OCMClassMock([NSBundle class]);
  OCMStub([mockBundle mainBundle]).andReturn(mockBundle);
  OCMStub([mockBundle infoDictionary]).andReturn(@{@"FlutterDeepLinkingEnabled" : @YES});

  FlutterAppDelegate* appDelegate = [[FlutterAppDelegate alloc] init];
  FlutterViewController* viewController = OCMClassMock([FlutterViewController class]);
  FlutterEngine* engine = OCMClassMock([FlutterEngine class]);
  FlutterMethodChannel* navigationChannel = OCMClassMock([FlutterMethodChannel class]);
  OCMStub([engine navigationChannel]).andReturn(navigationChannel);
  OCMStub([viewController engine]).andReturn(engine);
  // Set blockArg to a strong local to retain to end of scope.
  id blockArg = [OCMArg invokeBlockWithArgs:@NO, nil];
  OCMStub([engine waitForFirstFrame:3.0 callback:blockArg]);
  appDelegate.rootFlutterViewControllerGetter = ^{
    return viewController;
  };
  NSURL* url = [NSURL URLWithString:@"http://myApp/custom/route?query=test"];
  NSUserActivity* userActivity = [[NSUserActivity alloc] initWithActivityType:@"com.example.test"];
  userActivity.webpageURL = url;
  BOOL result = [appDelegate
               application:[UIApplication sharedApplication]
      continueUserActivity:userActivity
        restorationHandler:^(NSArray<id<UIUserActivityRestoring>>* __nullable restorableObjects){
        }];
  XCTAssertTrue(result);
  OCMVerify([navigationChannel invokeMethod:@"pushRoute" arguments:@"/custom/route?query=test"]);
  [mockBundle stopMocking];
}

@end
