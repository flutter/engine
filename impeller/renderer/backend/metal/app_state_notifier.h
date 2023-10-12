// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>

namespace impeller {

class AppStateNotifier {
 public:
  struct AppStateNotifierImpl;

  AppStateNotifier();

  /// Callback is executed on the platform thread.
  void SetAppDidBecomeActiveCallback(std::function<void()> callback) {
    app_did_become_active_callback_ = callback;
  }

  void OnAppBecameActive();

 private:
  static void DeleteImpl(AppStateNotifierImpl* impl);
  std::function<void()> app_did_become_active_callback_;
  std::unique_ptr<AppStateNotifierImpl, decltype(&AppStateNotifier::DeleteImpl)>
      impl_;
};

}  // namespace impeller
