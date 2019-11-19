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
class SyncSwitch {
 public:
  /// Represents the 2 code paths available when calling |SyncSwitch::Execute|.
  struct Handlers {
    /// Creates a |Handlers| were all exection paths are noops.
    Handlers();
    Handlers& SetTrue(const std::function<void()>& handler);
    Handlers& SetFalse(const std::function<void()>& handler);
    std::function<void()> true_handler;
    std::function<void()> false_handler;
  };

  /// Create a |SyncSwitch| with the false value.
  SyncSwitch();

  /// Create a |SyncSwitch| with the specified value.
  SyncSwitch(bool value);

  /// Diverge execution between true and false values of the SyncSwitch.
  ///
  /// This can be called on any thread.
  void Execute(const Handlers& handlers);

  /// Set the value of the SyncSwitch.
  ///
  /// This can be called on any thread.
  void SetSwitch(bool value);

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(SyncSwitch);

  std::mutex mutex_;
  bool value_;
};

}  // namespace fml

#endif  // FLUTTER_FML_SYNCHRONIZATION_SYNC_SWITCH_H_
