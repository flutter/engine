// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "TextSemanticsFocusTest.h"

@implementation TextSemanticsFocusTest

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--text-semantics-focus" ];
  [self.application launch];
}

- (void)testAccessibilityFocusOnTextSemanticsProducesCorrectIosViews {
  // Find the initial TextInputSemanticsObject which was sent from the mock framework on first frame.
  XCUIElement* textInputSemanticsObject = [[[self.application textFields] matchingIdentifier:@"flutter textfield"] element];
  XCTAssertTrue([textInputSemanticsObject waitForExistenceWithTimeout:5]);
  XCTAssertEqual([textInputSemanticsObject valueForKey:@"hasKeyboardFocus"], @(NO));
  
  // Since the first mock framework text field isn't focused on, it shouldn't produce a UITextInput
  // in the view hierarchy.
  XCUIElement* delegateTextInput = [[self.application textViews] element];
  XCTAssertFalse([delegateTextInput waitForExistenceWithTimeout:5]);
  
  // Nor should there be a keyboard for text entry.
  XCUIElement* keyboard = [[self.application keyboards] element];
  XCTAssertFalse([keyboard waitForExistenceWithTimeout:5]);

  // The tap location doesn't matter. The mock framework just sends a focused text field on tap.
  [textInputSemanticsObject tap];
  
  // The new TextInputSemanticsObject now has keyboard focus (the only trait accessible through
  // UI tests on a XCUIElement).
  textInputSemanticsObject = [[[self.application textFields] matchingIdentifier:@"focused flutter textfield"] element];
  XCTAssertTrue([textInputSemanticsObject waitForExistenceWithTimeout:5]);
  XCTAssertEqual([textInputSemanticsObject valueForKey:@"hasKeyboardFocus"], @(YES));
  
  // The delegate UITextInput is also inserted on the window and also has keyboard focus to delegate
  // real text entry events to the framework.
  delegateTextInput = [[self.application textViews] element];
  XCTAssertTrue([delegateTextInput waitForExistenceWithTimeout:5]);
  XCTAssertEqual([delegateTextInput valueForKey:@"hasKeyboardFocus"], @(YES));
  
  // Since there is focus, the soft keyboard is visible on the simulator.
  keyboard = [[self.application keyboards] element];
  XCTAssertTrue([keyboard waitForExistenceWithTimeout:5]);
}

@end
