// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_RENDERER_CONTEXT_MANAGER_H_
#define FLUTTER_SHELL_COMMON_RENDERER_CONTEXT_MANAGER_H_

#include <functional>
#include <memory>
#include <vector>

#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"

namespace flutter {

//------------------------------------------------------------------------------
/// An abstract class represents a renderer context
///
/// The subclass should wrap a "Context" object inside this class. For example,
/// in iOS while using GL rendering surface, the subclass should wrap an
/// |EAGLContext|.
class RendererContext {
 public:
  //------------------------------------------------------------------------------
  /// Implement this to set the context wrapped by this |RendererContext| object
  /// to the current context.
  virtual bool SetCurrent() = 0;

  //------------------------------------------------------------------------------
  /// Implement this to remove the context wrapped by this |RendererContext|
  /// object from current context;
  virtual bool RemoveCurrent() = 0;

  RendererContext();

  virtual ~RendererContext();

  FML_DISALLOW_COPY_AND_ASSIGN(RendererContext);
};

class RendererContextSwitch;

//------------------------------------------------------------------------------
/// Manages the renderer context.
///
/// Use this manager to manage flutter's gl context. All the contexts managed by
/// this are not shared between threads.
///
/// We want to return the exact context state before we set our context, after
/// we are done with our contexts. One way to do it is to cache any current
/// context (if any) in the `SetCurrent` of your `ContextRenderer`
/// implementation. Then in the `RemoveCurrent`, reset the cached context to
/// current using the platform specific gl method.
class RendererContextManager {
 public:
  RendererContextManager(std::shared_ptr<RendererContext> context,
                         std::shared_ptr<RendererContext> resource_context);

  ~RendererContextManager();

  //----------------------------------------------------------------------------
  /// @brief      Make the `context` to be the current context. It can be used
  /// to set a context other than the flutter context or the flutter resource
  /// context. We can manage a context this way only if we know the life cycle
  /// of it.
  ///
  /// @return     A `RendererContextSwitch` which ensures the `context` is the
  /// current context.
  std::unique_ptr<RendererContextSwitch> MakeCurrent(
      std::shared_ptr<RendererContext> context);

  //----------------------------------------------------------------------------
  /// @brief      Make the context Flutter uses on the raster thread for
  /// onscreen rendering to be the current context.
  ///
  /// @return     A `RendererContextSwitch` which ensures the current context is
  /// the `context` that used in the constructor.
  std::unique_ptr<RendererContextSwitch> FlutterMakeCurrent();

  //----------------------------------------------------------------------------
  /// @brief      Make the context Flutter uses for performing texture upload on
  /// the IO thread to be the current context.
  ///
  /// @return     A `RendererContextSwitch` which ensures the current context is
  /// the `resource_context` that used in the constructor.
  std::unique_ptr<RendererContextSwitch> FlutterResourceMakeCurrent();

 private:
  friend RendererContextSwitch;

  std::shared_ptr<RendererContext> context_;
  std::shared_ptr<RendererContext> resource_context_;

  bool PushContext(std::shared_ptr<RendererContext> context);
  bool PopContext();
  void EnsureStackInitialized();

  FML_DISALLOW_COPY_AND_ASSIGN(RendererContextManager);
};

/// The result of a set context operation.
///
/// Used by platforms that don't need the |RendererContextManager|.
class RendererContextResult {
 public:
  //----------------------------------------------------------------------------
  /// Constructs a |RendererContextResult| with a static result.
  ///
  /// Used in embeders that doesn't require renderer context switching. (For
  /// example, metal on iOS)
  ///
  /// @param  static_result a static value that will be returned from
  /// |GetResult|
  RendererContextResult(bool static_result);

  RendererContextResult();
  ~RendererContextResult();

  //----------------------------------------------------------------------------
  // Returns true if the context switching was successful.
  bool GetResult();

 protected:
  bool result_;

  FML_DISALLOW_COPY_AND_ASSIGN(RendererContextResult);
};
//------------------------------------------------------------------------------
/// Switches the renderer context to the a context that is passed in the
/// constructor.
///
/// In destruction, it should reset the current context back to what was
/// before the construction of this switch.
///
class RendererContextSwitch final : public RendererContextResult {
 public:
  //----------------------------------------------------------------------------
  /// Constructs a |RendererContextSwitch|.
  ///
  /// @param  manager A reference to the manager.
  /// @param  context The context that is going to be set as the current
  /// context.
  RendererContextSwitch(RendererContextManager& manager,
                        std::shared_ptr<RendererContext> context);

  ~RendererContextSwitch();

 private:
  RendererContextManager& manager_;

  FML_DISALLOW_COPY_AND_ASSIGN(RendererContextSwitch);
};

}  // namespace flutter

#endif
