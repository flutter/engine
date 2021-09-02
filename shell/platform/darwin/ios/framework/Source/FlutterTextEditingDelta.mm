// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextEditingDelta.h"

@implementation FlutterTextEditingDelta

- (instancetype)initTextEditingDelta:(NSString*)textBeforeChange
                     textAfterChange:(NSString*)textAfterChange
                       replacedRange:(NSRange)range
                         updatedText:(NSString*)text {
  self = [super init];

  if (self) {
    NSInteger start = range.location;
    NSInteger end = range.location + range.length;
    NSInteger tbstart = 0;
    NSInteger tbend = text.length;

    NSLog(@"replaceRangeLocal range start: %lu to end: %lu character: %@ tbstart: %lu tbend: %lu",
          start, end, text, tbstart, tbend);
    NSLog(@"Word being edited (before edits): %@", textBeforeChange);
    BOOL isEqual = [textBeforeChange isEqualToString:textAfterChange];

    BOOL isDeletionGreaterThanOne = end - (start + tbend) > 1;
    BOOL isDeletingByReplacingWithEmpty = text.length == 0 && tbstart == 0 && tbstart == tbend;

    BOOL isReplacedByShorter = isDeletionGreaterThanOne && (tbend - tbstart < end - start);
    BOOL isReplacedByLonger = tbend - tbstart > end - start;
    BOOL isReplacedBySame = tbend - tbstart == end - start;

    BOOL isInsertingInsideMarkedText = start + tbend > end;
    BOOL isDeletingInsideMarkedText = !isReplacedByShorter && start + tbend < end;

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
      [self setDeltas:textBeforeChange
              newText:@""
                 type:@"TextEditingDeltaType.equality"
           deltaStart:-1
             deltaEnd:-1];
    } else if (isDeletingByReplacingWithEmpty || isDeletingInsideMarkedText) {  // Deletion.
      NSString* deleted = [textBeforeChange
          substringWithRange:NSMakeRange(start + tbend, textBeforeChange.length - (start + tbend))];
      NSLog(@"We have a deletion");
      NSLog(@"We are deletion %@ at start position: %lu and end position: %lu", deleted, start,
            end);
      NSInteger actualStart = start;

      if (!isDeletionGreaterThanOne) {
        actualStart = end - 1;
      }

      [self setDeltas:textBeforeChange
              newText:deleted
                 type:@"TextEditingDeltaType.deletion"
           deltaStart:actualStart
             deltaEnd:end];
    } else if ((start == end || isInsertingInsideMarkedText) &&
               !isOriginalComposingRegionTextChanged) {  // Insertion.
      NSLog(@"We have an insertion");
      NSLog(@"We are inserting %@ at start position: %lu and end position: %lu",
            [text substringWithRange:NSMakeRange(end - start, text.length - (end - start))], start,
            end);
      [self
           setDeltas:textBeforeChange
             newText:[text substringWithRange:NSMakeRange(end - start, text.length - (end - start))]
                type:@"TextEditingDeltaType.insertion"
          deltaStart:end
            deltaEnd:end];
    } else if (isReplaced) {  // Replacement.
      NSString* replaced = [textBeforeChange substringWithRange:range];
      NSLog(@"We have a replacement");
      NSLog(@"We are replacing %@ at start position: %lu and end position: %lu with %@", replaced,
            start, end, text);
      [self setDeltas:textBeforeChange
              newText:text
                 type:@"TextEditingDeltaType.replacement"
           deltaStart:start
             deltaEnd:end];
    }
  }

  return self;
}

- (instancetype)initWithEquality:(NSString*)text {
  self = [super init];

  if (self) {
    NSLog(@"We have no changes, reporting equality");
    NSString* empty = @"";
    NSString* type = @"TextEditingDeltaType.equality";
    [self setDeltas:text newText:empty type:type deltaStart:-1 deltaEnd:-1];
  }

  return self;
}

- (void)setDeltas:(NSString*)oldText
          newText:(NSString*)newTxt
             type:(NSString*)deltaType
       deltaStart:(NSInteger)newStart
         deltaEnd:(NSInteger)newEnd {
  _oldText = oldText;
  _deltaText = newTxt;
  _deltaType = deltaType;
  _deltaStart = newStart;
  _deltaEnd = newEnd;
}

@end
