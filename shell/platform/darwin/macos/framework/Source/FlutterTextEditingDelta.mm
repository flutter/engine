// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterTextEditingDelta.h"

@implementation FlutterTextEditingDelta

- (instancetype)initTextEditingDelta:(NSString*)textBeforeChange
                       replacedRange:(NSRange)range
                         updatedText:(NSString*)text {
  self = [super init];

  if (self) {
    NSInteger start = range.location;
    NSInteger end = range.location + range.length;
    [self setDeltas:textBeforeChange newText:text deltaStart:start deltaEnd:end];
  }

  return self;
}

- (instancetype)initWithNonText:(NSString*)text {
  self = [super init];

  if (self) {
    [self setDeltas:text newText:@"" deltaStart:-1 deltaEnd:-1];
  }

  return self;
}

- (void)setDeltas:(NSString*)oldText
          newText:(NSString*)newTxt
       deltaStart:(NSInteger)newStart
         deltaEnd:(NSInteger)newEnd {
  _oldText = oldText;
  _deltaText = newTxt;
  _deltaStart = newStart;
  _deltaEnd = newEnd;
  //_deltaStart = MAX(0, MIN(oldText.length, newStart));
  //_deltaEnd = MAX(0, MIN(oldText.length, newEnd));
}

+ (instancetype)textEditingDelta:(NSString*)textBeforeChange
                   replacedRange:(NSRange)range
                     updatedText:(NSString*)text {
  return [[FlutterTextEditingDelta alloc] initTextEditingDelta:textBeforeChange
                                                 replacedRange:range
                                                   updatedText:text];
}

+ (instancetype)deltaWithNonText:(NSString*)text {
  return [[FlutterTextEditingDelta alloc] initWithNonText:text];
}

@end
