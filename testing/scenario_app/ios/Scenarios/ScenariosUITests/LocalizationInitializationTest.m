// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "LocalizationInitialization.h"

FLUTTER_ASSERT_ARC

@interface XCUIElement (ftr_waitForNonExistence)
/// Keeps waiting until the element doesn't exist.  Returns NO if the timeout is
/// reached before it doesn't exist.
- (BOOL)ftr_waitForNonExistenceWithTimeout:(NSTimeInterval)timeout;
/// Waits the full duration to ensure something doesn't exist for that duration.
/// Returns NO if at some point the element exists during the duration.
- (BOOL)ftr_waitForNonExistenceForDuration:(NSTimeInterval)duration;
@end

@implementation XCUIElement (ftr_waitForNonExistence)
- (BOOL)ftr_waitForNonExistenceWithTimeout:(NSTimeInterval)timeout {
  NSTimeInterval delta = 0.5;
  while (timeout > 0.0) {
    if (!self.exists) {
      return YES;
    }
    usleep(delta * 1000000);
    timeout -= delta;
  }
  return NO;
}

- (BOOL)ftr_waitForNonExistenceForDuration:(NSTimeInterval)duration {
  return ![self waitForExistenceWithTimeout:duration];
}

@end

@implementation LocalizationInitializationTest

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--locale-initialization" ];
  [self.application launch];
}

- (void)testNoLocalePrepend {
  NSTimeInterval timeout = 10.0;

  XCUIElement* textInputSemanticsObject =
      [[[self.application textFields] matchingIdentifier:@"[en]"] element];

  // The locales recieved by dart:ui are exposed onBeginFrame via semantics label.
  // There should only be one locale, as we have removed the locale prepend on iOS.
  XCTAssertTrue([textInputSemanticsObject waitForExistenceWithTimeout:timeout]);
  XCTAssertEqualObjects([textInputSemanticsObject valueForKey:@"hasKeyboardFocus"], @(NO));
}

@end
