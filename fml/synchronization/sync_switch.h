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

class SyncSwitch {
 public:
  class Observer {
   public:
    virtual void OnSetSwitch(bool value) = 0;
  };

  struct Handlers {
    Handlers();
    Handlers& SetTrue(const std::function<void()>& handler);
    Handlers& SetFalse(const std::function<void()>& handler);
    std::function<void()> true_handler;
    std::function<void()> false_handler;
  };

  SyncSwitch();

  SyncSwitch(bool value);

  void Execute(const Handlers& handlers);

  void SetSwitch(bool value);

  /// Run on same thread as |SetSwitch|.
  void AddObserver(Observer* observer);

  /// Run on same thread as |SetSwitch|.
  void RemoveObserver(Observer* observer);

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(SyncSwitch);

  std::mutex mutex_;
  std::condition_variable condition_variable_;
  bool value_;
  std::forward_list<Observer*> observers_;
};

}  // namespace fml

#endif  // FLUTTER_FML_SYNCHRONIZATION_SYNC_SWITCH_H_
