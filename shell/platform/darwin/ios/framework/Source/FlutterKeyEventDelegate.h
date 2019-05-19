// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_

#import <Foundation/Foundation.h>

@protocol FlutterKeyEventDelegate <NSObject>

- (void)performKeyPress:(int)keyCode withCharacters:(NSString*)characters;

@end

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_
