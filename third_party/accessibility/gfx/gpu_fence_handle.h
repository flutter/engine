// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GPU_FENCE_HANDLE_H_
#define UI_GFX_GPU_FENCE_HANDLE_H_

#include "ax_build/build_config.h"
#include "ui/gfx/gfx_export.h"

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
#include "base/file_descriptor_posix.h"
#endif

namespace gfx {

enum class GpuFenceHandleType {
  // A null handle for transport. It cannot be used for making a waitable fence
  // object.
  kEmpty,

  // A file descriptor for a native fence object as used by the
  // EGL_ANDROID_native_fence_sync extension.
  kAndroidNativeFenceSync,

  kLast = kAndroidNativeFenceSync
};

struct GFX_EXPORT GpuFenceHandle {
  GpuFenceHandle();
  GpuFenceHandle(const GpuFenceHandle& other);
  GpuFenceHandle& operator=(const GpuFenceHandle& other);
  ~GpuFenceHandle();

  bool is_null() const { return type == GpuFenceHandleType::kEmpty; }

  GpuFenceHandleType type;
#if defined(OS_POSIX) || defined(OS_FUCHSIA)
  base::FileDescriptor native_fd;
#endif
};

// Returns an instance of |handle| which can be sent over IPC. This duplicates
// the file-handles as appropriate, so that the IPC code take ownership of them,
// without invalidating |handle| itself.
GFX_EXPORT GpuFenceHandle CloneHandleForIPC(const GpuFenceHandle& handle);

}  // namespace gfx

#endif  // UI_GFX_GPU_FENCE_HANDLE_H_
