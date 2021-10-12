// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FLUTTERTEXTEDITINGDELTA_H_
#define SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FLUTTERTEXTEDITINGDELTA_H_

#import <Cocoa/Cocoa.h>

@interface FlutterTextEditingDelta : NSObject

@property(nonatomic, readonly) NSString* oldText;
@property(nonatomic, readonly) NSString* deltaText;
@property(nonatomic, readonly) NSInteger deltaStart;
@property(nonatomic, readonly) NSInteger deltaEnd;

+ (instancetype)textEditingDelta:(NSString*)textBeforeChange
                   replacedRange:(NSRange)range
                     updatedText:(NSString*)text;

+ (instancetype)deltaWithNonText:(NSString*)text;

@end
#endif  // SHELL_PLATFORM_MACOS_FRAMEWORK_SOURCE_FLUTTERTEXTEDITINGDELTA_H_
