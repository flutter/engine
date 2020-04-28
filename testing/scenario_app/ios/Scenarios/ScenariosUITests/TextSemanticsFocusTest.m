// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "TextSemanticsFocusTest.h"

FLUTTER_ASSERT_ARC

@interface XCUIElement (ftr_waitForNonExistence)
-(BOOL)ftr_waitForNonExistenceWithTimeout:(NSTimeInterval)duration;
@end

@implementation XCUIElement (ftr_waitForNonExistence)
-(BOOL)ftr_waitForNonExistenceWithTimeout:(NSTimeInterval)duration {
  NSLog(@"Waiting %fs for non-existance of:%@", duration, [self debugDescription]);
  NSTimeInterval delta = 0.5;
  while (duration > 0.0) {
    if (!self.exists) {
      return YES;
    }
    usleep(delta * 1000000);
  }
  NSLog(@"timeout waiting for:%@", self);
  return NO;
}
@end

@implementation TextSemanticsFocusTest

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--text-semantics-focus" ];
  [self.application launch];
}

- (void)testAccessibilityFocusOnTextSemanticsProducesCorrectIosViews {
  NSTimeInterval timeout = 10.0;
  // Find the initial TextInputSemanticsObject which was sent from the mock framework on first
  // frame.
  XCUIElement* textInputSemanticsObject =
      [[[self.application textFields] matchingIdentifier:@"flutter textfield"] element];
  XCTAssertTrue([textInputSemanticsObject waitForExistenceWithTimeout:timeout]);
  XCTAssertEqualObjects([textInputSemanticsObject valueForKey:@"hasKeyboardFocus"], @(NO));

  // Since the first mock framework text field isn't focused on, it shouldn't produce a UITextInput
  // in the view hierarchy.
  XCUIElement* delegateTextInput = [[self.application textViews] element];
  XCTAssertTrue([delegateTextInput ftr_waitForNonExistenceWithTimeout:timeout]);

  // Nor should there be a keyboard for text entry.
  XCUIElement* keyboard = [[self.application keyboards] element];
  XCTAssertTrue([keyboard ftr_waitForNonExistenceWithTimeout:timeout]);

  // The tap location doesn't matter. The mock framework just sends a focused text field on tap.
  [textInputSemanticsObject tap];

  // The new TextInputSemanticsObject now has keyboard focus (the only trait accessible through
  // UI tests on a XCUIElement).
  textInputSemanticsObject =
      [[[self.application textFields] matchingIdentifier:@"focused flutter textfield"] element];
  XCTAssertTrue([textInputSemanticsObject waitForExistenceWithTimeout:timeout]);
  XCTAssertEqualObjects([textInputSemanticsObject valueForKey:@"hasKeyboardFocus"], @(YES));

  // The delegate UITextInput is also inserted on the window but we make only the
  // TextInputSemanticsObject visible and not the FlutterTextInputView to avoid confusing
  // accessibility, it shouldn't be visible to the UI test either.
  delegateTextInput = [[self.application textViews] element];
  XCTAssertTrue([delegateTextInput ftr_waitForNonExistenceWithTimeout:timeout]);

  // But since there is focus, the soft keyboard is visible on the simulator.
  keyboard = [[self.application keyboards] element];
  XCTAssertTrue([keyboard waitForExistenceWithTimeout:timeout]);
}

@end
