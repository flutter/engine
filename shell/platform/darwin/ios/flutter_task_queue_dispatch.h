// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FLUTTER_TASK_QUEUE_DISPATCH_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FLUTTER_TASK_QUEUE_DISPATCH_H_

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"

@protocol FlutterTaskQueueDispatch <FlutterTaskQueue>
- (void)dispatch:(dispatch_block_t)block;
@end

#endif
