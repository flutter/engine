// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GL_CONTEXT_GUARD_MANAGER_H
#define GL_CONTEXT_GUARD_MANAGER_H

#include "flutter/fml/macros.h"

namespace flutter {

// Manages `GLContextSwitch`.
//
// Should be subclassed for each platform embedder that uses GL, and requires to
// protect flutter's gl context from other 3rd party librarys, plugins and
// packages.
class GLContextSwitchManager {
 public:
  // A `GLGuard` protects the flutter's gl context to be used by other 3rd party
  // librarys, plugins and packages. On construction, it should set flutter's gl
  // context to the current context. On destruction, it should restore the gl
  // context before the construction of this object.
  class GLContextSwitch {
   public:
    GLContextSwitch() = default;

    virtual ~GLContextSwitch() {}

    virtual bool GetSwitchResult() = 0;

    FML_DISALLOW_COPY_AND_ASSIGN(GLContextSwitch);
  };

  GLContextSwitchManager() = default;
  ~GLContextSwitchManager() = default;

  virtual std::unique_ptr<GLContextSwitch> MakeCurrent() = 0;
  virtual std::unique_ptr<GLContextSwitch> ResourceMakeCurrent() = 0;

  class EmbedderGLContextSwitch final : public GLContextSwitch {
   public:
    EmbedderGLContextSwitch(bool switch_result):switch_result_(switch_result){}

    ~EmbedderGLContextSwitch() = default;

    bool GetSwitchResult() override {
      return switch_result_;
    }
   private:
    bool switch_result_;
  };

  FML_DISALLOW_COPY_AND_ASSIGN(GLContextSwitchManager);
};

}  // namespace flutter

#endif
