// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_

#import <Foundation/Foundation.h>

// The iOS platform doesn't provide us with the device keycode, so the
// usb keycode (as defined in chromium) is used here.
#define KEYCODE_DELETE_BACKWARD 0x7002A

@protocol FlutterKeyEventDelegate <NSObject>

- (void)dispatchKeyEvent:(NSString*)type keyCode:(int)keyCode characters:(NSString*)characters;

@end

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FlutterKeyEventDelegate_H_
