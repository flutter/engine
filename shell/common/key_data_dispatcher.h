// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef KEY_DATA_DISPATCHER_H_
#define KEY_DATA_DISPATCHER_H_

#include "flutter/runtime/runtime_controller.h"
#include "flutter/shell/common/animator.h"

namespace flutter {

class KeyDataDispatcher;

//------------------------------------------------------------------------------
/// The `Engine` key data dispatcher that forwards the packet received from
/// `PlatformView::DispatchKeyDataPacket` on the platform thread, to
/// `Window::DispatchKeyDataPacket` on the UI thread.
///
/// This class is used to filter the packets so the Flutter framework on the UI
/// thread will receive packets with some desired properties. See
/// `SmoothKeyDataDispatcher` for an example which filters irregularly
/// delivered packets, and dispatches them in sync with the VSYNC signal.
///
/// This object will be owned by the engine because it relies on the engine's
/// `Animator` (which owns `VsyncWaiter`) and `RuntomeController` to do the
/// filtering. This object is currently designed to be only called from the UI
/// thread (no thread safety is guaranteed).
///
/// The `PlatformView` decides which subclass of `KeyDataDispatcher` is
/// constructed by sending a `KeyDataDispatcherMaker` to the engine's
/// constructor in `Shell::CreateShellOnPlatformThread`. This is needed because:
///   (1) Different platforms (e.g., Android, iOS) have different dispatchers
///       so the decision has to be made per `PlatformView`.
///   (2) The `PlatformView` can only be accessed from the PlatformThread while
///       this class (as owned by engine) can only be accessed in the UI thread.
///       Hence `PlatformView` creates a `KeyDataDispatchMaker` on the
///       platform thread, and sends it to the UI thread for the final
///       construction of the `KeyDataDispatcher`.
class KeyDataDispatcher {
 public:
  /// The interface for Engine to implement.
  class Delegate {
   public:
    /// Actually dispatch the packet using Engine's `animator_` and
    /// `runtime_controller_`.
    virtual void DoDispatchKeyPacket(std::unique_ptr<KeyDataPacket> packet) = 0;
  };

  //----------------------------------------------------------------------------
  /// @brief      Signal that `PlatformView` has a packet to be dispatched.
  ///
  /// @param[in]  packet             The `KeyDataPacket` to be dispatched.
  virtual void DispatchKeyPacket(std::unique_ptr<KeyDataPacket> packet) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Default destructor.
  virtual ~KeyDataDispatcher();
};

//------------------------------------------------------------------------------
/// The default dispatcher that forwards the packet without any modification.
///
class DefaultKeyDataDispatcher : public KeyDataDispatcher {
 public:
  DefaultKeyDataDispatcher(Delegate& delegate) : delegate_(delegate) {}

  // |KeyDataDispatcer|
  void DispatchKeyPacket(std::unique_ptr<KeyDataPacket> packet) override;

  virtual ~DefaultKeyDataDispatcher();

 protected:
  Delegate& delegate_;

  FML_DISALLOW_COPY_AND_ASSIGN(DefaultKeyDataDispatcher);
};

//--------------------------------------------------------------------------
/// @brief      Signature for constructing KeyDataDispatcher.
///
/// @param[in]  delegate      the `Flutter::Engine`
///
using KeyDataDispatcherMaker =
    std::function<std::unique_ptr<KeyDataDispatcher>(
        KeyDataDispatcher::Delegate&)>;

}  // namespace flutter

#endif  // KEY_DATA_DISPATCHER_H_
