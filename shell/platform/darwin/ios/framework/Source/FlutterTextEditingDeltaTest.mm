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

@end
