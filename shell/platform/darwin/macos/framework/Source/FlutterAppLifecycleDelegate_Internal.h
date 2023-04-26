// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERAPPLIFECYCLEDELEGATE_INTERNAL_H_
#define FLUTTER_FLUTTERAPPLIFECYCLEDELEGATE_INTERNAL_H_

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterAppLifecycleDelegate.h"

#include "flutter/shell/platform/common/application_lifecycle.h"

@interface FlutterAppLifecycleRegistrar ()

/**
 * Check whether there is at least one plugin responds to the selector.
 */
- (BOOL)hasDelegateThatRespondsToSelector:(SEL)selector;

@end

#endif  // FLUTTER_FLUTTERAPPLIFECYCLEDELEGATE_INTERNAL_H_
