// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextEditingDelta.h"

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterEngine.h"

@interface FlutterTextEditingDeltaTest : XCTestCase
@end

@implementation FlutterTextEditingDeltaTest

- (NSRange)clampSelection:(NSRange)range forText:(NSString*)text {
  int start = MIN(MAX(range.location, 0), text.length);
  int length = MIN(range.length, text.length - start);
  return NSMakeRange(start, length);
}

#pragma mark - TextEditingDelta Insertion tests
- (void)testInsertionTextEditingDeltaAtEndOfComposing {
  // Here we are simulating inserting an "o" at the end of "hell".
  NSString* oldText = @"hell";
  NSString* textAfterChange = @"hello";
  NSString* replacementText = textAfterChange;
  NSRange range = NSMakeRange(0, 4);

  FlutterTextEditingDelta* delta = [[FlutterTextEditingDelta alloc]
      initTextEditingDelta:oldText
           textAfterChange:textAfterChange
             replacedRange:[self clampSelection:range forText:oldText]
               updatedText:replacementText];

  XCTAssertEqual(delta.oldText, oldText);
  XCTAssertEqual(delta.deltaText, @"o");
  XCTAssertEqual(delta.deltaType, @"TextEditingDeltaType.insertion");
  XCTAssertEqual(delta.deltaStart, 4);
  XCTAssertEqual(delta.deltaEnd, 4);
}

- (void)testInsertionTextEditingDeltaInsideOfComposing {
  // Here we are simulating inserting an "l" after the "l" in "helo".
  NSString* oldText = @"helo";
  NSString* textAfterChange = @"hello";
  NSString* replacementText = @"l";
  NSRange range = NSMakeRange(3, 0);

  FlutterTextEditingDelta* delta = [[FlutterTextEditingDelta alloc]
      initTextEditingDelta:oldText
           textAfterChange:textAfterChange
             replacedRange:[self clampSelection:range forText:oldText]
               updatedText:replacementText];

  XCTAssertEqual(delta.oldText, oldText);
  XCTAssertEqual(delta.deltaText, replacementText);
  XCTAssertEqual(delta.deltaType, @"TextEditingDeltaType.insertion");
  XCTAssertEqual(delta.deltaStart, 3);
  XCTAssertEqual(delta.deltaEnd, 3);
}

#pragma mark - TextEditingDelta Deletion tests

- (void)testDeletionTextEditingDeltaAtEndOfComposing {
  // Here we are simulating deleting an "o" at the end of "hello".
  NSString* oldText = @"hello";
  NSString* textAfterChange = @"hell";
  NSString* replacementText = textAfterChange;
  NSRange range = NSMakeRange(0, 5);

  FlutterTextEditingDelta* delta = [[FlutterTextEditingDelta alloc]
      initTextEditingDelta:oldText
           textAfterChange:textAfterChange
             replacedRange:[self clampSelection:range forText:oldText]
               updatedText:replacementText];

  XCTAssertEqual(delta.oldText, oldText);
  XCTAssertEqual(delta.deltaText, @"o");
  XCTAssertEqual(delta.deltaType, @"TextEditingDeltaType.deletion");
  XCTAssertEqual(delta.deltaStart, 4);
  XCTAssertEqual(delta.deltaEnd, 5);
}

- (void)testDeletionTextEditingDeltaInsideOfComposing {
  // Here we are simulating deleting an "e" in the world "hello".
  NSString* oldText = @"hello";
  NSString* textAfterChange = @"hllo";
  NSString* replacementText = @"";
  NSRange range = NSMakeRange(1, 1);

  FlutterTextEditingDelta* delta = [[FlutterTextEditingDelta alloc]
      initTextEditingDelta:oldText
           textAfterChange:textAfterChange
             replacedRange:[self clampSelection:range forText:oldText]
               updatedText:replacementText];

  XCTAssertEqual(delta.oldText, oldText);
  XCTAssertEqual(delta.deltaText, @"e");
  XCTAssertEqual(delta.deltaType, @"TextEditingDeltaType.deletion");
  XCTAssertEqual(delta.deltaStart, 1);
  XCTAssertEqual(delta.deltaEnd, 2);
}

- (void)testDeletionTextEditingDeltaForSelection {
  // Here we are simulating deleting "llo" in the word "hello".
  NSString* oldText = @"hello";
  NSString* textAfterChange = @"he";
  NSString* replacementText = @"";
  NSRange range = NSMakeRange(2, 3);

  FlutterTextEditingDelta* delta = [[FlutterTextEditingDelta alloc]
      initTextEditingDelta:oldText
           textAfterChange:textAfterChange
             replacedRange:[self clampSelection:range forText:oldText]
               updatedText:replacementText];

  XCTAssertEqual(delta.oldText, oldText);
  XCTAssertEqual(delta.deltaText, @"llo");
  XCTAssertEqual(delta.deltaType, @"TextEditingDeltaType.deletion");
  XCTAssertEqual(delta.deltaStart, 2);
  XCTAssertEqual(delta.deltaEnd, 5);
}

#pragma mark - TextEditingDelta Replacement tests

- (void)testReplacementSameTextEditingDelta {
  // Here we are simulating a replacement of a range of text that could for example happen
  // when the word "worfd" is autocorrected to the word "world".
  NSString* oldText = @"worfd";
  NSString* textAfterChange = @"world";
  NSString* replacementText = textAfterChange;
  NSRange range = NSMakeRange(0, 5);

  FlutterTextEditingDelta* delta = [[FlutterTextEditingDelta alloc]
      initTextEditingDelta:oldText
           textAfterChange:textAfterChange
             replacedRange:[self clampSelection:range forText:oldText]
               updatedText:replacementText];

  XCTAssertEqual(delta.oldText, oldText);
  XCTAssertEqual(delta.deltaText, @"world");
  XCTAssertEqual(delta.deltaType, @"TextEditingDeltaType.replacement");
  XCTAssertEqual(delta.deltaStart, 0);
  XCTAssertEqual(delta.deltaEnd, 5);
}

- (void)testReplacementLongerTextEditingDelta {
  // Here we are simulating a replacement of a range of text that could for example happen
  // when the word "wolkin" is autocorrected to "walking".
  NSString* oldText = @"wolkin";
  NSString* textAfterChange = @"walking";
  NSString* replacementText = textAfterChange;
  NSRange range = NSMakeRange(0, 6);

  FlutterTextEditingDelta* delta = [[FlutterTextEditingDelta alloc]
      initTextEditingDelta:oldText
           textAfterChange:textAfterChange
             replacedRange:[self clampSelection:range forText:oldText]
               updatedText:replacementText];

  XCTAssertEqual(delta.oldText, oldText);
  XCTAssertEqual(delta.deltaText, @"walking");
  XCTAssertEqual(delta.deltaType, @"TextEditingDeltaType.replacement");
  XCTAssertEqual(delta.deltaStart, 0);
  XCTAssertEqual(delta.deltaEnd, 6);
}

- (void)testReplacementShorterTextEditingDelta {
  // Here we are simulating a replacement of a range of text that could for example happen
  // when the word "world" is replaced with a single character "h".
  NSString* oldText = @"world";
  NSString* textAfterChange = @"h";
  NSString* replacementText = textAfterChange;
  NSRange range = NSMakeRange(0, 5);

  FlutterTextEditingDelta* delta = [[FlutterTextEditingDelta alloc]
      initTextEditingDelta:oldText
           textAfterChange:textAfterChange
             replacedRange:[self clampSelection:range forText:oldText]
               updatedText:replacementText];

  XCTAssertEqual(delta.oldText, oldText);
  XCTAssertEqual(delta.deltaText, @"h");
  XCTAssertEqual(delta.deltaType, @"TextEditingDeltaType.replacement");
  XCTAssertEqual(delta.deltaStart, 0);
  XCTAssertEqual(delta.deltaEnd, 5);
}

@end
