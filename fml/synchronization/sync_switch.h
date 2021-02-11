// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_SYNCHRONIZATION_SYNC_SWITCH_H_
#define FLUTTER_FML_SYNCHRONIZATION_SYNC_SWITCH_H_

#include <forward_list>
#include <functional>
#include <mutex>

#include "flutter/fml/macros.h"

namespace fml {

/// A threadsafe structure that allows you to switch between 2 different
/// execution paths.
///
/// Execution and setting the switch is exclusive, i.e. only one will happen
/// at a time.
class SyncSwitch {
 public:
  /// A controller that sets the value of a |SyncSwitch|.
  class Controller {
   public:
    explicit Controller(std::shared_ptr<SyncSwitch> sync_switch);

    /// Set the value of the |SyncSwitch|.
    ///
    /// This can be called on any thread.
    ///
    /// @param[in]  value  New value for the |SyncSwitch|.
    void SetSwitch(bool value);

   private:
    std::shared_ptr<SyncSwitch> sync_switch_;

    FML_DISALLOW_COPY_AND_ASSIGN(Controller);
  };

  /// Represents the 2 code paths available when calling |SyncSwitch::Execute|.
  struct Handlers {
    /// Sets the handler that will be executed if the |SyncSwitch| is true.
    Handlers& SetIfTrue(const std::function<void()>& handler);

    /// Sets the handler that will be executed if the |SyncSwitch| is false.
    Handlers& SetIfFalse(const std::function<void()>& handler);

    std::function<void()> true_handler = [] {};
    std::function<void()> false_handler = [] {};
  };

  /// Create a |SyncSwitch| with the specified value.
  ///
  /// If value is not specified, it is defaulted to false.
  ///
  /// @param[in]  value      The initial value for this switch.
  explicit SyncSwitch(bool value = false);

  /// Diverge execution between true and false values of the SyncSwitch.
  ///
  /// This can be called on any thread.  Note that attempting to call
  /// |SetSwitch| inside of the handlers will result in a self deadlock.
  ///
  /// @param[in]  handlers  Called for the correct value of the |SyncSwitch|.
  void Execute(const Handlers& handlers);

 private:
  std::mutex mutex_;
  bool value_;

  FML_DISALLOW_COPY_AND_ASSIGN(SyncSwitch);
};

}  // namespace fml

#endif  // FLUTTER_FML_SYNCHRONIZATION_SYNC_SWITCH_H_
