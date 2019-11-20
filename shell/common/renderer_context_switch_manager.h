// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_GL_CONTEXT_SWITCH_MANAGER_H_
#define FLUTTER_SHELL_COMMON_GL_CONTEXT_SWITCH_MANAGER_H_

#include <memory>
#include "flutter/fml/macros.h"

namespace flutter {

//------------------------------------------------------------------------------
/// Manages `RendererContextSwitch`.
///
/// Should be subclassed for platforms that uses GL and requires context
/// switching. Always use `MakeCurrent` and `ResourceMakeCurrent` in the
/// `RendererContextSwitchManager` to set gl contexts.
///
class RendererContextSwitchManager {
 public:
  //------------------------------------------------------------------------------
  /// Switches the gl context to the flutter's contexts.
  ///
  /// Should be subclassed for each platform embedder that uses GL.
  /// In construction, it should set the current context to a flutter's context
  /// In destruction, it should rest the current context.
  ///
  class RendererContextSwitch {
   public:
    RendererContextSwitch();

    virtual ~RendererContextSwitch();

    virtual bool GetSwitchResult() = 0;

    FML_DISALLOW_COPY_AND_ASSIGN(RendererContextSwitch);
  };

  RendererContextSwitchManager();
  ~RendererContextSwitchManager();

  //----------------------------------------------------------------------------
  /// @brief      Make the flutter's context as current context.
  ///
  /// @return     A `RendererContextSwitch` with `GetSwitchResult` returning
  /// true if the setting process is succesful.
  virtual std::unique_ptr<RendererContextSwitch> MakeCurrent() = 0;

  //----------------------------------------------------------------------------
  /// @brief      Make the flutter's resources context as current context.
  ///
  /// @return     A `RendererContextSwitch` with `GetSwitchResult` returning
  /// true if the setting process is succesful.
  virtual std::unique_ptr<RendererContextSwitch> ResourceMakeCurrent() = 0;

  //------------------------------------------------------------------------------
  /// A representation of a `RendererContextSwitch` that doesn't require actual
  /// context switching.
  ///
  class RendererContextSwitchPureResult final : public RendererContextSwitch {
   public:
    // Constructor that creates an `RendererContextSwitchPureResult`.
    // The `GetSwitchResult` will return the same value as `switch_result`.

    //----------------------------------------------------------------------------
    /// @brief      Constructs a  `RendererContextSwitchPureResult`.
    ///
    /// @param[in]  switch_result       the switch result that will be returned
    /// in `GetSwitchResult()`
    ///
    RendererContextSwitchPureResult(bool switch_result);

    ~RendererContextSwitchPureResult();

    bool GetSwitchResult() override;

   private:
    bool switch_result_;

    FML_DISALLOW_COPY_AND_ASSIGN(RendererContextSwitchPureResult);
  };

  FML_DISALLOW_COPY_AND_ASSIGN(RendererContextSwitchManager);
};

}  // namespace flutter

#endif
