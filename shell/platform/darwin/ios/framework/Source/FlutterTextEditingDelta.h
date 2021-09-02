// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTEDITINGDELTA_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTEDITINGDELTA_H_

#import <UIKit/UIKit.h>

@interface FlutterTextEditingDelta : NSObject

@property(nonatomic, readonly) NSMutableString* oldText;
@property(nonatomic, readonly) NSMutableString* deltaText;
@property(nonatomic, readonly) NSMutableString* deltaType;
@property(nonatomic, readonly) NSInteger deltaStart;
@property(nonatomic, readonly) NSInteger deltaEnd;

- (instancetype)initTextEditingDelta:(NSMutableString*)textBeforeChange
                     textAfterChange:(NSMutableString*)textAfterChange
                       replacedRange:(NSRange)range
                         updatedText:(NSMutableString*)text;

- (instancetype)initWithEquality:(NSMutableString*)text;

- (void)setDeltas:(NSMutableString*)oldText
          newText:(NSMutableString*)newTxt
             type:(NSMutableString*)deltaType
       deltaStart:(NSInteger)newStart
         deltaEnd:(NSInteger)newEnd;

@end
#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTEDITINGDELTA_H_
