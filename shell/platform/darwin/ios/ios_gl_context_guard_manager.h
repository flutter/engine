// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ios_gl_guard_manager_h
#define ios_gl_guard_manager_h

#define GLES_SILENCE_DEPRECATION

#import <OpenGLES/EAGL.h>
#include <map>
#include "flutter/flow/gl_context_guard_manager.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"

namespace flutter {

// The iOS implementation of `GLContextGuardManager`
//
// On `GLGuard`'s construction, it pushes the current EAGLContext to a stack and
// sets the flutter's gl context as current.
// On `GLGuard`'s desstruction, it pops a EAGLContext from the stack and set it to current.
class IOSGLContextGuardManager final : public GLContextGuardManager {
 public:

  class IOSGLContextAutoRelease final : public GLContextMakeCurrentResult {

    public:
     IOSGLContextAutoRelease(IOSGLContextGuardManager& manager, fml::scoped_nsobject<EAGLContext> context) : manager_(manager) {
       bool result = manager_.PushContext(context);
       has_pushed_context_ = true;
       make_current_result_ = result;
     };

    bool GetMakeCurrentResult() {return make_current_result_;}

    ~IOSGLContextAutoRelease() {
      if (!has_pushed_context_) {
        return;
      }
      manager_.PopContext();
    }

   private:
    IOSGLContextGuardManager& manager_;
    bool make_current_result_;
    bool has_pushed_context_;
  };

  IOSGLContextGuardManager();

  ~IOSGLContextGuardManager() = default;

  ///=====================================================

  IOSGLContextAutoRelease MakeCurrent();
  IOSGLContextAutoRelease ResourceMakeCurrent();
  bool PushContext(fml::scoped_nsobject<EAGLContext> context);
  void PopContext();
  fml::scoped_nsobject<EAGLContext> context_;

 private:
  fml::scoped_nsobject<EAGLContext> resource_context_;

  fml::scoped_nsobject<NSMutableArray> stored_;

  FML_DISALLOW_COPY_AND_ASSIGN(IOSGLContextGuardManager);
};

}

#endif
