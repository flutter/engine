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
          (long)start, (long)end, text, (long)tbstart, (long)tbend);
    NSLog(@"Word being edited (before edits): %@", textBeforeChange);
    BOOL isEqual = [textBeforeChange isEqualToString:textAfterChange];
    BOOL isDeletionGreaterThanOne = (end - start) - (tbend - tbstart) > 1;

    BOOL isDeletingByReplacingWithEmpty = text.length == 0 && tbstart == 0 && tbstart == tbend;

    BOOL isReplacedByShorter = isDeletionGreaterThanOne && (tbend - tbstart < end - start);
    BOOL isReplacedByLonger = tbend - tbstart > end - start;
    BOOL isReplacedBySame = tbend - tbstart == end - start;

    BOOL isInsertingInsideMarkedText = start + tbend > end;
    BOOL isDeletingInsideMarkedText =
        !isReplacedByShorter && !isDeletingByReplacingWithEmpty && start + tbend < end;

    NSLog(@"isEqual? %d", isEqual);
    NSLog(@"isDeletionGreaterThanOne? %d", isDeletionGreaterThanOne);
    NSLog(@"isReplacedBySame? %d", isReplacedBySame);
    NSLog(@"isReplacedByShorter? %d", isReplacedByShorter);
    NSLog(@"isReplacedByLonger? %d", isReplacedByLonger);
    NSLog(@"isDeletingByReplacingWithEmpty? %d", isDeletingByReplacingWithEmpty);

    NSLog(@"isInsertedInsideMarkedText? %d", isInsertingInsideMarkedText);
    NSLog(@"isDeletingInsideMarkedText? %d", isDeletingInsideMarkedText);

    NSString* newMarkedText;
    NSString* originalMarkedText;

    if (isDeletingByReplacingWithEmpty || isDeletingInsideMarkedText || isReplacedByShorter) {
      newMarkedText = [text substringWithRange:NSMakeRange(tbstart, tbend)];
      originalMarkedText = [textBeforeChange substringWithRange:NSMakeRange(range.location, tbend)];
    } else {
      newMarkedText = [text substringWithRange:NSMakeRange(tbstart, end - start)];
      originalMarkedText = [textBeforeChange substringWithRange:range];
    }

    NSLog(@"comparing %@ with %@", originalMarkedText, newMarkedText);

    BOOL isOriginalComposingRegionTextChanged = ![originalMarkedText isEqualToString:newMarkedText];

    NSLog(@"the original composing text has changed? %d", isOriginalComposingRegionTextChanged);

    BOOL isReplaced = isOriginalComposingRegionTextChanged ||
                      (isReplacedByLonger || isReplacedByShorter || isReplacedBySame);

    NSLog(@"isReplaced? %d", isReplaced);

    if (isEqual) {
      NSLog(@"We have no changes, reporting equality");
      [self setDeltas:textBeforeChange
              newText:@""
                 type:@"TextEditingDeltaType.equality"
           deltaStart:-1
             deltaEnd:-1];
    } else if ((isDeletingByReplacingWithEmpty || isDeletingInsideMarkedText) &&
               !isOriginalComposingRegionTextChanged) {  // Deletion.
      NSLog(@"We have a deletion");
      NSString* deleted;
      if (isDeletingInsideMarkedText) {
        deleted = [textBeforeChange
            substringWithRange:NSMakeRange(start + tbend, (end - start) - (start + tbend))];
      } else {
        deleted = [textBeforeChange substringWithRange:NSMakeRange(start + tbend, end - start)];
      }

      NSLog(@"We are deletion %@ at start position: %lu and end position: %lu", deleted,
            (long)start, (long)end);
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
            [text substringWithRange:NSMakeRange(end - start, text.length - (end - start))],
            (long)start, (long)end);
      [self
           setDeltas:textBeforeChange
             newText:[text substringWithRange:NSMakeRange(end - start, text.length - (end - start))]
                type:@"TextEditingDeltaType.insertion"
          deltaStart:end
            deltaEnd:end];
    } else if (isReplaced) {  // Replacement.
      NSLog(@"We have a replacement");
      NSString* replaced = [textBeforeChange substringWithRange:range];
      NSLog(@"We are replacing %@ at start position: %lu and end position: %lu with %@", replaced,
            (long)start, (long)end, text);
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
    [self setDeltas:text
            newText:@""
               type:@"TextEditingDeltaType.equality"
         deltaStart:-1
           deltaEnd:-1];
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

- (id)copyWithZone:(NSZone*)zone {
  FlutterTextEditingDelta* copyOfObject = [[FlutterTextEditingDelta alloc] initWithEquality:@""];
  [copyOfObject setDeltas: _oldText
                  newText: _deltaText
                     type: _deltaType
               deltaStart: _deltaStart
                 deltaEnd: _deltaEnd];
  return copyOfObject;
}

@end
