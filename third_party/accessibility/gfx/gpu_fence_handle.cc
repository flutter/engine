// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/gpu_fence_handle.h"

#include "base/notreached.h"

#if defined(OS_POSIX)
#include <unistd.h>

#include "base/posix/eintr_wrapper.h"
#endif

namespace gfx {

GpuFenceHandle::GpuFenceHandle() : type(GpuFenceHandleType::kEmpty) {}

GpuFenceHandle::GpuFenceHandle(const GpuFenceHandle& other) = default;

GpuFenceHandle& GpuFenceHandle::operator=(const GpuFenceHandle& other) =
    default;

GpuFenceHandle::~GpuFenceHandle() {}

GpuFenceHandle CloneHandleForIPC(const GpuFenceHandle& source_handle) {
  switch (source_handle.type) {
    case GpuFenceHandleType::kEmpty:
      NOTREACHED();
      return source_handle;
    case GpuFenceHandleType::kAndroidNativeFenceSync: {
      gfx::GpuFenceHandle handle;
#if defined(OS_POSIX)
      handle.type = GpuFenceHandleType::kAndroidNativeFenceSync;
      int duped_handle = HANDLE_EINTR(dup(source_handle.native_fd.fd));
      if (duped_handle < 0)
        return GpuFenceHandle();
      handle.native_fd = base::FileDescriptor(duped_handle, true);
#endif
      return handle;
    }
  }
  NOTREACHED();
  return gfx::GpuFenceHandle();
}

}  // namespace gfx
