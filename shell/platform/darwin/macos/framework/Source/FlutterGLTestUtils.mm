// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterGLTestUtils.h"

#import <OpenGL/gl.h>

namespace flutter::testing {

NSOpenGLContext* CreateTestOpenGLContext() {
  NSOpenGLPixelFormatAttribute attributes[] = {
      NSOpenGLPFAColorSize, 24, NSOpenGLPFAAlphaSize, 8, 0,
  };
  NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
  return [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
}

}  // flutter::testing
