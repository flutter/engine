// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_

#import <Foundation/Foundation.h>

#define KEYCODE_DELETE_BACKWARD 0x33

@protocol FlutterKeyEventDelegate <NSObject>

- (void)dispatchKeyEvent:(NSString*)type keyCode:(int)keyCode characters:(NSString*)characters;

@end

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_
