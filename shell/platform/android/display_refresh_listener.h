// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_DISPLAY_REFRESH_LISTENER_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_DISPLAY_REFRESH_LISTENER_H_

#include <cstdint>

#include "flutter/fml/macros.h"

namespace flutter {

/// A means to recieve updates when the display refresh rate updates on Android.
///
/// This class only works on API30+. On older versions, the
/// Delegate::OnDisplayRefreshUpdated will never be called. Instead, the
/// FlutterJNI#refreshRateFPS should be used for lower API versions. That field
/// has the disadvantage being out of sync with the current refresh rate, as it
/// is typically only updated on initialization.
///
/// This class must be constructed on the Platform task runner. Callbacks on the
/// delegate are safe to recieve on any task runner.
class DisplayRefreshListener {
 public:
  class Delegate {
   public:
    /// This method may be called on any task runner.
    virtual void OnDisplayRefreshUpdated(int64_t vsync_period_nanos) = 0;
  };

  /// This class must be constructed on the Platform task runner.
  explicit DisplayRefreshListener(Delegate& delegate);
  ~DisplayRefreshListener();

 private:
  Delegate& delegate_;

  FML_DISALLOW_COPY_AND_ASSIGN(DisplayRefreshListener);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_DISPLAY_REFRESH_LISTENER_H_
