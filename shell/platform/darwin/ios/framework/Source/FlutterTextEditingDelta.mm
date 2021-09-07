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

    BOOL isEqual = [textBeforeChange isEqualToString:textAfterChange];
    BOOL isDeletionGreaterThanOne = (end - start) - (tbend - tbstart) > 1;

    BOOL isDeletingByReplacingWithEmpty = text.length == 0 && tbstart == 0 && tbstart == tbend;

    BOOL isReplacedByShorter = isDeletionGreaterThanOne && (tbend - tbstart < end - start);
    BOOL isReplacedByLonger = tbend - tbstart > end - start;
    BOOL isReplacedBySame = tbend - tbstart == end - start;

    BOOL isInsertingInsideMarkedText = start + tbend > end;
    BOOL isDeletingInsideMarkedText =
        !isReplacedByShorter && !isDeletingByReplacingWithEmpty && start + tbend < end;

    NSString* newMarkedText;
    NSString* originalMarkedText;

    if (isDeletingByReplacingWithEmpty || isDeletingInsideMarkedText || isReplacedByShorter) {
      newMarkedText = [text substringWithRange:NSMakeRange(tbstart, tbend)];
      originalMarkedText = [textBeforeChange substringWithRange:NSMakeRange(range.location, tbend)];
    } else {
      newMarkedText = [text substringWithRange:NSMakeRange(tbstart, end - start)];
      originalMarkedText = [textBeforeChange substringWithRange:range];
    }

    BOOL isOriginalComposingRegionTextChanged = ![originalMarkedText isEqualToString:newMarkedText];

    BOOL isReplaced = isOriginalComposingRegionTextChanged ||
                      (isReplacedByLonger || isReplacedByShorter || isReplacedBySame);

    if (isEqual) {
      [self setDeltas:textBeforeChange
              newText:@""
                 type:@"TextEditingDeltaType.nonTextUpdate"
           deltaStart:-1
             deltaEnd:-1];
    } else if ((isDeletingByReplacingWithEmpty || isDeletingInsideMarkedText) &&
               !isOriginalComposingRegionTextChanged) {  // Deletion.
      NSString* deleted = @"";
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
      [self
           setDeltas:textBeforeChange
             newText:[text substringWithRange:NSMakeRange(end - start, text.length - (end - start))]
                type:@"TextEditingDeltaType.insertion"
          deltaStart:end
            deltaEnd:end];
    } else if (isReplaced) {  // Replacement.
      [self setDeltas:textBeforeChange
              newText:text
                 type:@"TextEditingDeltaType.replacement"
           deltaStart:start
             deltaEnd:end];
    }
  }

  return self;
}

- (instancetype)initWithNonText:(NSString*)text {
  self = [super init];

  if (self) {
    [self setDeltas:text
            newText:@""
               type:@"TextEditingDeltaType.nonTextUpdate"
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
  [copyOfObject setDeltas:_oldText
                  newText:_deltaText
                     type:_deltaType
               deltaStart:_deltaStart
                 deltaEnd:_deltaEnd];
  return copyOfObject;
}

@end
