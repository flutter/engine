// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_choreographer.h"

#include "flutter/fml/native_library.h"

// Only avialalbe on API 24+
typedef void AChoreographer;
// Only avialalbe on API 29+ or API 24+ and architecture is 64-bit
typedef void (*AChoreographer_frameCallback)(int64_t frameTimeNanos,
                                             void* data);
// Only avialalbe on API 24+
typedef AChoreographer* (*AChoreographer_getInstance_FPN)();
typedef void (*AChoreographer_postFrameCallback_FPN)(
    AChoreographer* choreographer,
    AChoreographer_frameCallback callback,
    void* data);
static AChoreographer_getInstance_FPN AChoreographer_getInstance;
static AChoreographer_postFrameCallback_FPN AChoreographer_postFrameCallback;

namespace flutter {

bool AndroidChoreographer::ShouldUseNDKChoreographer() {
  static std::optional<bool> should_use_ndk_choreographer;
  if (should_use_ndk_choreographer) {
    return should_use_ndk_choreographer.value();
  }
  auto libandroid = fml::NativeLibrary::Create("libandroid.so");
  FML_DCHECK(libandroid);
  auto get_instance_fn =
      libandroid->ResolveFunction<AChoreographer_getInstance_FPN>(
          "AChoreographer_getInstance");
  auto post_frame_callback_fn =
      libandroid->ResolveFunction<AChoreographer_postFrameCallback_FPN>(
          "AChoreographer_postFrameCallback64");
#if FML_ARCH_CPU_64_BITS
  if (!post_frame_callback_fn) {
    post_frame_callback_fn =
        libandroid->ResolveFunction<AChoreographer_postFrameCallback_FPN>(
            "AChoreographer_postFrameCallback");
  }
#endif
  if (get_instance_fn && post_frame_callback_fn) {
    AChoreographer_getInstance = get_instance_fn.value();
    AChoreographer_postFrameCallback = post_frame_callback_fn.value();
    should_use_ndk_choreographer = true;
  } else {
    should_use_ndk_choreographer = false;
  }
  return should_use_ndk_choreographer.value();
}

void AndroidChoreographer::PostFrameCallback(OnFrameCallback callback,
                                             void* data) {
  AChoreographer* choreographer = AChoreographer_getInstance();
  AChoreographer_postFrameCallback(choreographer, callback, data);
}

}  // namespace flutter
