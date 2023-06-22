// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/NSCursor+IgnoreChange.h"

#include <objc/runtime.h>
#include <atomic>

@implementation NSCursor (FlutterIgnoreCursorChange)

static std::atomic_bool flutterIgnoreCursorChange = false;

+ (BOOL)flutterIgnoreCursorChange {
  return flutterIgnoreCursorChange;
}

+ (void)setFlutterIgnoreCursorChange:(BOOL)ignore {
  flutterIgnoreCursorChange = ignore;
}

- (void)_flutterSetCursorOverride {
  if (flutterIgnoreCursorChange) {
    return;
  }
  [self _flutterSetCursorOverride];
}

+ (void)replaceSelector:(SEL)originalSelector
              fromClass:(Class)originalClass
           withSelector:(SEL)replacementSelector
              fromClass:(Class)replacementClass {
  Method altMethod = class_getInstanceMethod(replacementClass, replacementSelector);

  class_addMethod(originalClass, replacementSelector,
                  class_getMethodImplementation(replacementClass, replacementSelector),
                  method_getTypeEncoding(altMethod));

  method_exchangeImplementations(class_getInstanceMethod(originalClass, originalSelector),
                                 class_getInstanceMethod(originalClass, replacementSelector));
}

+ (void)load {
  [self replaceSelector:@selector(set)
              fromClass:[NSCursor class]
           withSelector:@selector(_flutterSetCursorOverride)
              fromClass:[NSCursor class]];
}

@end
