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

// The iOS implementation of `GLContextSwitchManager`
//
// On `GLGuard`'s construction, it pushes the current EAGLContext to a stack and
// sets the flutter's gl context as current.
// On `GLGuard`'s desstruction, it pops a EAGLContext from the stack and set it to current.
class IOSGLContextSwitchManager final : public GLContextSwitchManager {
 public:
  class IOSGLContextSwitch final : public GLContextSwitch {
    public:
     IOSGLContextSwitch(IOSGLContextSwitchManager& manager, fml::scoped_nsobject<EAGLContext> context) : manager_(manager) {
       bool result = manager_.PushContext(context);
       has_pushed_context_ = true;
       switch_result_ = result;
     };

    bool GetSwitchResult() override {return switch_result_;}

    ~IOSGLContextSwitch() {
      if (!has_pushed_context_) {
        return;
      }
      manager_.PopContext();
    }

   private:
    IOSGLContextSwitchManager& manager_;
    bool switch_result_;
    bool has_pushed_context_;

    FML_DISALLOW_COPY_AND_ASSIGN(IOSGLContextSwitch);
  };

  IOSGLContextSwitchManager();

  ~IOSGLContextSwitchManager() = default;

  ///=====================================================

  std::unique_ptr<GLContextSwitch> MakeCurrent() override;
  std::unique_ptr<GLContextSwitch> ResourceMakeCurrent() override;
  bool PushContext(fml::scoped_nsobject<EAGLContext> context);
  void PopContext();
  fml::scoped_nsobject<EAGLContext> context_;

 private:
  fml::scoped_nsobject<EAGLContext> resource_context_;

  fml::scoped_nsobject<NSMutableArray> stored_;

  FML_DISALLOW_COPY_AND_ASSIGN(IOSGLContextSwitchManager);
};

}

#endif
