//
//  FlutterViewControllerTest.m
//  ScenariosUITests
//
//  Created by Aaron Clarke on 7/24/19.
//  Copyright © 2019 flutter. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Flutter/Flutter.h>
#import "AppDelegate.h"

@interface FlutterViewControllerTest : XCTestCase
@property (nonatomic, strong) FlutterViewController* flutterViewController;
@end

@implementation FlutterViewControllerTest

- (void)setUp {
  self.continueAfterFailure = NO;
}

- (void)tearDown {
  if (self.flutterViewController) {
    [self.flutterViewController removeFromParentViewController];
  }
}

- (void)testFirstFrameCallback {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"test" project:nil];
  [engine runWithEntrypoint:nil];
  self.flutterViewController = [[FlutterViewController alloc] initWithEngine:engine
                                                                     nibName:nil
                                                                      bundle:nil];
  __block BOOL shouldKeepRunning = YES;
  [self.flutterViewController setFlutterViewDidRenderCallback:^{
    shouldKeepRunning = NO;
  }];
  AppDelegate* appDelegate = (AppDelegate*)UIApplication.sharedApplication.delegate;
  UIViewController* rootVC = appDelegate.window.rootViewController;
  [rootVC presentViewController:self.flutterViewController animated:NO completion:nil];
  NSRunLoop *theRL = [NSRunLoop currentRunLoop];
  int countDownMs = 2000;
  while (shouldKeepRunning && countDownMs > 0) {
   [theRL runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    countDownMs -= 100;
  }
  XCTAssertGreaterThan(countDownMs, 0);
}

@end
