// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextEditingDelta.h"

@implementation FlutterTextEditingDelta

- (instancetype)initTextEditingDelta:(NSMutableString*)textBeforeChange
                     textAfterChange:(NSMutableString*)textAfterChange
                       replacedRange:(NSRange)range
                         updatedText:(NSMutableString*)text {
  self = [super init];

  if (self) {
    // NSRange(start, length).
    // The start indicates the starting position of the range.
    // The length indicates how far the range extends from the start position.
    // NSRange(10, 5) means that we want the range from position 10 to position 15.
    // We start at position 10 and add 5 to get the final position 15.
    NSInteger start = range.location;
    NSInteger end = range.location + range.length;
    NSInteger tbstart = 0;
    NSInteger tbend = text.length;

    NSLog(@"replaceRangeLocal range start: %lu to end: %lu character: %@ tbstart: %lu tbend: %lu",
          start, end, text, tbstart, tbend);
    NSLog(@"Word being edited (before edits): %@", textBeforeChange);

    BOOL isDeletionGreaterThanOne = end - (start + tbend) > 1;

    BOOL isDeletingByReplacingWithEmpty = text.length == 0 && tbstart == 0 && tbstart == tbend;
    BOOL isReplacedByShorter = isDeletionGreaterThanOne && (tbend - tbstart < end - start);
    BOOL isReplacedByLonger = tbend - tbstart > end - start;
    BOOL isReplacedBySame = tbend - tbstart == end - start;
    BOOL isEqual = [textBeforeChange isEqualToString:textAfterChange];

    BOOL isDeletingInsideMarkedText = !isReplacedByShorter && start + tbend < end;
    BOOL isInsertingInsideMarkedText = start + tbend > end;

    NSString* originalMarkedText = [textBeforeChange substringWithRange:range];
    NSString* newMarkedText =
        (isDeletingByReplacingWithEmpty || isDeletingInsideMarkedText || isReplacedByShorter)
            ? @""
            : [text substringWithRange:NSMakeRange(tbstart, end - start)];
    BOOL isOriginalComposingRegionTextChanged = ![originalMarkedText isEqualToString:newMarkedText];

    BOOL isReplaced = isOriginalComposingRegionTextChanged &&
                      (isReplacedByLonger || isReplacedByShorter || isReplacedBySame);

    if (isEqual) {
      NSLog(@"We have no changes, reporting equality");
      NSMutableString* empty = [@"" mutableCopy];
      NSMutableString* type = [@"TextEditingDeltaType.equality" mutableCopy];
      [self setDeltas:[textBeforeChange mutableCopy]
              newText:empty
                 type:type
           deltaStart:-1
             deltaEnd:-1];
    } else if (isDeletingByReplacingWithEmpty || isDeletingInsideMarkedText) {  // Deletion.
      NSMutableString* deleted = [[textBeforeChange
         substringWithRange:NSMakeRange(start + tbend, textBeforeChange.length - (start + tbend))]
         mutableCopy];
      NSLog(@"We have a deletion");
      NSLog(@"We are deletion %@ at start position: %lu and end position: %lu", deleted, start,
            end);
      NSMutableString* type = [@"TextEditingDeltaType.deletion" mutableCopy];
      NSInteger actualStart = start;

      if (!isDeletionGreaterThanOne) {
        actualStart = end - 1;
      }

      [self setDeltas:[textBeforeChange mutableCopy]
              newText:deleted
                 type:type
           deltaStart:actualStart
             deltaEnd:end];
    } else if ((start == end || isInsertingInsideMarkedText) &&
               !isOriginalComposingRegionTextChanged) {  // Insertion.
      NSLog(@"We have an insertion");
      NSLog(@"We are inserting %@ at start position: %lu and end position: %lu",
            [text substringWithRange:NSMakeRange(end - start, text.length - (end - start))], start,
            end);
      NSMutableString* type = [@"TextEditingDeltaType.insertion" mutableCopy];
      NSMutableString* textBeforeInsertion = [textBeforeChange mutableCopy];
      [self setDeltas:textBeforeInsertion
              newText:[[text
                          substringWithRange:NSMakeRange(end - start, text.length - (end - start))]
                          mutableCopy]
                 type:type
           deltaStart:end
             deltaEnd:end];
    } else if (isReplaced) {  // Replacement.
      NSMutableString* replaced = [[textBeforeChange substringWithRange:range] mutableCopy];
      NSLog(@"We have a replacement");
      NSLog(@"We are replacing %@ at start position: %lu and end position: %lu with %@", replaced,
            start, end, text);
      NSMutableString* type = [@"TextEditingDeltaType.replacement" mutableCopy];
      [self setDeltas:[textBeforeChange mutableCopy]
              newText:[text mutableCopy]
                 type:type
           deltaStart:start
             deltaEnd:end];
    }
  }

  return self;
}

- (instancetype)initWithEquality:(NSMutableString*)text {
  self = [super init];

  if (self) {
    NSLog(@"We have no changes, reporting equality");
    NSMutableString* empty = [@"" mutableCopy];
    NSMutableString* type = [@"TextEditingDeltaType.equality" mutableCopy];
    [self setDeltas:[text mutableCopy] newText:empty type:type deltaStart:-1 deltaEnd:-1];
  }

  return self;
}

- (void)setDeltas:(NSMutableString*)oldText
          newText:(NSMutableString*)newTxt
             type:(NSMutableString*)deltaType
       deltaStart:(NSInteger)newStart
         deltaEnd:(NSInteger)newEnd {
  _oldText = oldText;
  _deltaText = newTxt;
  _deltaType = deltaType;
  _deltaStart = newStart;
  _deltaEnd = newEnd;
}

@end
