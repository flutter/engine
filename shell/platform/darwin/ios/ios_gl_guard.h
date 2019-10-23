// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ios_gl_guard_manager_h
#define ios_gl_guard_manager_h

#define GLES_SILENCE_DEPRECATION

#import <OpenGLES/EAGL.h>
#include <map>
#include "flutter/fml/platform/darwin/scoped_nsobject.h"

namespace flutter {

class IOSGLGuard {
 public:
  IOSGLGuard(){
    SavePlatformViewContext();
    RestoreFlutterContext();
  };

  ~IOSGLGuard() {
    SaveFlutterContext();
    RestorePlatformViewContext();
  }

 private:
  fml::scoped_nsobject<EAGLContext> flutter_gl_context_;
  fml::scoped_nsobject<EAGLContext> platform_view_gl_context_;

  FML_DISALLOW_COPY_AND_ASSIGN(IOSGLGuard);

  void RestorePlatformViewContext() {
    RestoreContext(platform_view_gl_context_);
  }

  void SavePlatformViewContext() {
    SaveContext(platform_view_gl_context_);
  }

  void RestoreFlutterContext() { RestoreContext(flutter_gl_context_); }

  void SaveFlutterContext() { SaveContext(flutter_gl_context_); }

  void SaveContext(fml::scoped_nsobject<EAGLContext> store_at) {
    store_at = fml::scoped_nsobject<EAGLContext>([[EAGLContext currentContext] retain]);
  }

  void RestoreContext(fml::scoped_nsobject<EAGLContext> stored_context) {
    [EAGLContext setCurrentContext:stored_context.get()];
  }
};

}

#endif /* ios_gl_guard_manager_h */
