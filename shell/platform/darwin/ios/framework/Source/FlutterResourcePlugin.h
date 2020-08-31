// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERRESOURCEPLUGIN_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERRESOURCEPLUGIN_H_

#include "flutter/shell/platform/darwin/common/framework/Headers/FlutterChannels.h"

/*!
  A plugin for responding to system channel requests system resources over
  'system/resource'.
 */
@interface FlutterResourcePlugin : NSObject

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result;

@end

#endif
