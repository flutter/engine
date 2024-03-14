// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_ANDROID_CHOREOGRAPHER_H_
#define FLUTTER_IMPELLER_TOOLKIT_ANDROID_CHOREOGRAPHER_H_

#include "impeller/toolkit/android/proc_table.h"

#include <chrono>
#include <memory>

namespace impeller::android {

//------------------------------------------------------------------------------
/// @brief      This class describes access to the choreographer instance for
///             the current thread. Choreographers are only available on API
///             levels above 24. On levels below 24, an invalid choreographer
///             will be returned.
///
///             Since choreographer need an event loop on the current thread,
///             one will be setup if it doesn't already exist.
///
class Choreographer {
 public:
  static bool IsAvailableOnPlatform();

  static Choreographer& GetInstance();

  ~Choreographer();

  Choreographer(const Choreographer&) = delete;

  Choreographer& operator=(const Choreographer&) = delete;

  bool IsValid() const;

  using FrameClock = std::chrono::steady_clock;
  using FrameTimePoint = std::chrono::time_point<FrameClock>;
  using FrameCallback = std::function<void(FrameTimePoint)>;

  bool PostFrameCallback(FrameCallback callback) const;

 private:
  AChoreographer* instance_ = nullptr;

  explicit Choreographer();
};

}  // namespace impeller::android

#endif  // FLUTTER_IMPELLER_TOOLKIT_ANDROID_CHOREOGRAPHER_H_
