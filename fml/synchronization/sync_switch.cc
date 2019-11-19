// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/synchronization/sync_switch.h"

namespace fml {

SyncSwitch::Handlers::Handlers() : true_handler({}), false_handler({}) {}

SyncSwitch::Handlers& SyncSwitch::Handlers::SetTrue(
    const std::function<void()>& handler) {
  true_handler = handler;
  return *this;
}

SyncSwitch::Handlers& SyncSwitch::Handlers::SetFalse(
    const std::function<void()>& handler) {
  false_handler = handler;
  return *this;
}

SyncSwitch::SyncSwitch() : SyncSwitch(false) {}

SyncSwitch::SyncSwitch(bool value) : value_(value) {}

void SyncSwitch::Execute(const SyncSwitch::Handlers& handlers) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (value_) {
    handlers.true_handler();
  } else {
    handlers.false_handler();
  }
}

void SyncSwitch::SetSwitch(bool value) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    value_ = value;
  }
  for (Observer* observer : observers_) {
    observer->OnSetSwitch(value);
  }
}

void SyncSwitch::AddObserver(SyncSwitch::Observer* observer) {
  observers_.push_front(observer);
}

void SyncSwitch::RemoveObserver(SyncSwitch::Observer* observer) {
  observers_.remove(observer);
}

}  // namespace fml
