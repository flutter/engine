// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_ANDROID_CHOREOGRAPHER_H_
#define FLUTTER_IMPELLER_TOOLKIT_ANDROID_CHOREOGRAPHER_H_

#include "impeller/toolkit/android/proc_table.h"

#include <chrono>
#include <memory>

namespace impeller::android {

enum class ChoreographerSupportStatus {
  // Unavailable, API level < 24.
  kUnsupported,
  // Available with postFrameCallback64 or postFrameCallback.
  kSupported,
  // Available with postVsyncCallback.
  kSupportedVsync,
};

// TODO(moffatman) document
struct ChoreographerVsyncTimings {
  std::chrono::time_point<std::chrono::steady_clock> start;
  std::chrono::time_point<std::chrono::steady_clock> target;
  int64_t id;
};

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
  static ChoreographerSupportStatus GetPlatformSupport();

  //----------------------------------------------------------------------------
  /// @brief      Create or get the thread local instance of a choreographer. A
  ///             message loop will be setup on the calling thread if none
  ///             exists.
  ///
  /// @warning    Choreographers are only available on API levels 24 and above.
  ///             Below this level, this will return an invalid instance.
  ///             Availability can also be checked via the
  ///             `IsAvailableOnPlatform` call.
  ///
  /// @return     The thread local choreographer instance. If none can be setup,
  ///             an invalid object reference will be returned. See `IsValid`.
  ///
  static Choreographer& GetInstance();

  ~Choreographer();

  Choreographer(const Choreographer&) = delete;

  Choreographer& operator=(const Choreographer&) = delete;

  bool IsValid() const;

  //----------------------------------------------------------------------------
  /// A monotonic system clock.
  ///
  using FrameClock = std::chrono::steady_clock;

  //----------------------------------------------------------------------------
  /// A timepoint on a monotonic system clock.
  ///
  using FrameTimePoint = std::chrono::time_point<FrameClock>;
  using FrameCallback = std::function<void(FrameTimePoint)>;
  // TODO: Need a new struct type here
  using VsyncCallback = std::function<void(ChoreographerVsyncTimings)>;

  //----------------------------------------------------------------------------
  /// @brief      Posts a frame callback. The time that the frame is being
  ///             rendered will be available in the callback as an argument.
  ///             Multiple frame callbacks within the same frame interval will
  ///             receive the same argument.
  ///
  /// @param[in]  callback  The callback
  ///
  /// @return     `true` if the frame callback could be posted. This may return
  ///             `false` if choreographers are not available on the platform.
  ///             See `IsAvailableOnPlatform`.
  ///
  bool PostFrameCallback(FrameCallback callback) const;

  // TODO(moffatman): document
  bool PostVsyncCallback(VsyncCallback callback, size_t latency) const;

 private:
  AChoreographer* instance_ = nullptr;

  explicit Choreographer();
};

}  // namespace impeller::android

#endif  // FLUTTER_IMPELLER_TOOLKIT_ANDROID_CHOREOGRAPHER_H_
