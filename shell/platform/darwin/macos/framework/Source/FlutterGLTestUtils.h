// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

namespace flutter::testing {

// Returns an NSOpenGLContext with a 24-bit color + 8-bit alpha pixel
// format.
NSOpenGLContext* CreateTestOpenGLContext();

}  // namespace flutter::testing
