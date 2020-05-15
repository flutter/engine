// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_GL_CONTEXT_SWITCH_H_
#define FLUTTER_SHELL_COMMON_GL_CONTEXT_SWITCH_H_

#include <functional>
#include <memory>
#include <vector>

#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"

namespace flutter {

//------------------------------------------------------------------------------
/// An abstract class represents a gl context that can be switched by
/// GLContextSwitch
///
/// The subclass should wrap a "Context" object inside this class. For example,
/// in iOS while using GL rendering surface, the subclass should wrap an
/// |EAGLContext|.
class SwitchableGLContext {
 public:
  //------------------------------------------------------------------------------
  /// Implement this to set the context wrapped by this |SwitchableGLContext|
  /// object to the current context.
  virtual bool SetCurrent() = 0;

  //------------------------------------------------------------------------------
  /// Implement this to remove the context wrapped by this |SwitchableGLContext|
  /// object from current context;
  virtual bool RemoveCurrent() = 0;

  SwitchableGLContext();

  virtual ~SwitchableGLContext();

  FML_DISALLOW_COPY_AND_ASSIGN(SwitchableGLContext);
};

class GLContextResult {
 public:
  GLContextResult();
  virtual ~GLContextResult();

  //----------------------------------------------------------------------------
  // Returns true if the context switching was successful.
  bool GetResult();

 protected:
  GLContextResult(bool static_result);
  bool result_;

  FML_DISALLOW_COPY_AND_ASSIGN(GLContextResult);
};

class GLContextDefaultResult : public GLContextResult {
 public:
  //----------------------------------------------------------------------------
  /// Constructs a |GLContextResult| with a static result.
  ///
  /// Used in embeders that doesn't require renderer context switching. (For
  /// example, metal on iOS)
  ///
  /// @param  static_result a static value that will be returned from
  /// |GetResult|
  GLContextDefaultResult(bool static_result);

  ~GLContextDefaultResult() override;

  FML_DISALLOW_COPY_AND_ASSIGN(GLContextDefaultResult);
};
//------------------------------------------------------------------------------
/// Switches the renderer context to the a context that is passed in the
/// constructor.
///
/// In destruction, it should reset the current context back to what was
/// before the construction of this switch.
///
class GLContextSwitch final : public GLContextResult {
 public:
  //----------------------------------------------------------------------------
  /// Constructs a |GLContextSwitch|.
  ///
  /// @param  context The context that is going to be set as the current
  /// context.
  GLContextSwitch(std::unique_ptr<SwitchableGLContext> context);

  ~GLContextSwitch() override;

 private:
  std::unique_ptr<SwitchableGLContext> context_;

  FML_DISALLOW_COPY_AND_ASSIGN(GLContextSwitch);
};

}  // namespace flutter

#endif
