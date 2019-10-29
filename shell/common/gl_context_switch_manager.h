// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_GL_CONTEXT_SWITCH_MANAGER_H_
#define FLUTTER_SHELL_COMMON_GL_CONTEXT_SWITCH_MANAGER_H_

#include <map>
#include "flutter/fml/macros.h"

namespace flutter {

// Manages `GLContextSwitch`.
//
// Should be subclassed for platforms that uses GL and requires context
// switching. Always use `MakeCurrent` and `ResourceMakeCurrent` in the
// `GLContextSwitchManager` to set gl contexts.
class GLContextSwitchManager {
 public:
  // Switches the gl context to the flutter's contexts.
  //
  // Should be subclassed for each platform embedder that uses GL.
  // In construction, it should set the current context to a flutter's context
  // In destruction, it should rest the current context.
  class GLContextSwitch {
   public:
    GLContextSwitch() = default;

    virtual ~GLContextSwitch() {}

    virtual bool GetSwitchResult() = 0;

    FML_DISALLOW_COPY_AND_ASSIGN(GLContextSwitch);
  };

  GLContextSwitchManager() = default;
  ~GLContextSwitchManager() = default;

  // Make the flutter's context as current context and returns a
  // `GLContextSwitch`.
  virtual std::unique_ptr<GLContextSwitch> MakeCurrent() = 0;

  // Make the flutter's resource context as current context and returns a
  // `GLContextSwitch`.
  virtual std::unique_ptr<GLContextSwitch> ResourceMakeCurrent() = 0;

  // A representation of a `GLContextSwitch` that doesn't require actual context
  // switching.
  class GLContextSwitchPureResult final : public GLContextSwitch {
   public:
    // Constructor that creates an `GLContextSwitchPureResult`.
    // The `GetSwitchResult` will return the same value as `switch_result`.
    GLContextSwitchPureResult(bool switch_result)
        : switch_result_(switch_result) {}

    ~GLContextSwitchPureResult() = default;

    bool GetSwitchResult() override { return switch_result_; }

   private:
    bool switch_result_;

    FML_DISALLOW_COPY_AND_ASSIGN(GLContextSwitchPureResult);
  };

  FML_DISALLOW_COPY_AND_ASSIGN(GLContextSwitchManager);
};

}  // namespace flutter

#endif
