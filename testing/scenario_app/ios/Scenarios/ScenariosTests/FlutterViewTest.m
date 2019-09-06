// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/Flutter.h>
#import <XCTest/XCTest.h>
#import "AppDelegate.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"

@interface FlutterViewGLContextTest : XCTestCase
@property(nonatomic, strong) FlutterViewController* flutterViewController;
@end

@implementation FlutterViewGLContextTest

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

// Ideally we want to also test that the IOSurface backed by a FlutterView is
// destroyed as well. Unfortunately due to test harness limitations we can't
// check that without adding test-only APIs to get access to internal objects.
//
// TODO: Test for the IOSurface destruction as well.
- (void)testFlutterViewDestroyed {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"testGL" project:nil];
  [engine runWithEntrypoint:nil];
  self.flutterViewController = [[FlutterViewController alloc] initWithEngine:engine
                                                                     nibName:nil
                                                                      bundle:nil];

  AppDelegate* appDelegate = (AppDelegate*)UIApplication.sharedApplication.delegate;
  UIViewController* rootVC = appDelegate.window.rootViewController;
  [rootVC presentViewController:self.flutterViewController animated:NO completion:nil];

  __weak id flutterView = [self.flutterViewController performSelector:NSSelectorFromString(@"flutterView")];
  XCTAssertNotNil(flutterView);
    
  [self.flutterViewController dismissViewControllerAnimated:NO completion:^{
      __weak id flutterView = [self.flutterViewController performSelector:NSSelectorFromString(@"flutterView")];
      XCTAssertNil(flutterView);
  }];
}

@end

#pragma clang diagnostic pop

