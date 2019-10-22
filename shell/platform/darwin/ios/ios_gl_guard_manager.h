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

class IOSGLGuardManager {
 public:
  class IOSGLGuard {
   public:
    IOSGLGuard(IOSGLGuardManager& gl_guard_manager, int64_t view_id)
        : gl_guard_manager_(gl_guard_manager), view_id_(view_id) {
      gl_guard_manager_.SaveFlutterContext();
      gl_guard_manager_.RestorePlatformViewContext(view_id);
    };

    ~IOSGLGuard() {
      gl_guard_manager_.SavePlatformViewContext(view_id_);
      gl_guard_manager_.RestoreFlutterContext();
    }

   private:
    IOSGLGuardManager& gl_guard_manager_;
    int64_t view_id_;
  };

  IOSGLGuard SpwanGuard(int64_t view_id) { return IOSGLGuard(*this, view_id); }

 private:
  fml::scoped_nsobject<EAGLContext> flutter_gl_context_;
  std::map<int64_t, fml::scoped_nsobject<EAGLContext>> platform_view_gl_contexts_;

  FML_DISALLOW_COPY_AND_ASSIGN(IOSGLGuardManager);

  void RestorePlatformViewContext(int64_t view_id) {
    fml::scoped_nsobject<EAGLContext> context = platform_view_gl_contexts_[view_id];
    RestoreContext(context);
  }

  void SavePlatformViewContext(int64_t view_id) {
    fml::scoped_nsobject<EAGLContext> context = platform_view_gl_contexts_[view_id];
    SaveContext(context);
  }

  void RestoreFlutterContext() { return RestoreContext(flutter_gl_context_); }

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
