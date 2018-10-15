// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_FLUTTERPLATFORMVIEWS_INTERNAL_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_FLUTTERPLATFORMVIEWS_INTERNAL_H_

#include "flutter/shell/common/shell.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterBinaryMessenger.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterPlatformViews.h"

@interface FlutterPlatformViewsController : NSObject

- (instancetype)init:(NSObject<FlutterBinaryMessenger>*)withMessenger;

- (void)registerViewFactory:(NSObject<FlutterPlatformViewFactory>*)factory
                     withId:(NSString*)factoryId;

@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_FLUTTERPLATFORMVIEWS_INTERNAL_H_
