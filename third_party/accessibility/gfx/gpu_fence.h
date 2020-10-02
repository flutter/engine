// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GL_GPU_FENCE_H_
#define UI_GL_GPU_FENCE_H_

#include "base/macros.h"
#include "ax_build/build_config.h"
#include "ui/gfx/gfx_export.h"
#include "ui/gfx/gpu_fence_handle.h"

extern "C" typedef struct _ClientGpuFence* ClientGpuFence;

namespace base {
class TimeTicks;
}  // namespace base

namespace gfx {

// GpuFence objects own a GpuFenceHandle and release the resources in it when
// going out of scope as appropriate.
class GFX_EXPORT GpuFence {
 public:
  // Constructor takes ownership of the source handle's resources.
  explicit GpuFence(const GpuFenceHandle& handle);
  GpuFence() = delete;
  ~GpuFence();

  // This handle is an unowned view of the resources owned by this class for
  // use with CloneHandleForIPC. Don't pass this to a consuming method such as
  // GpuFence(handle) or to IPC, that would cause duplicate resource release.
  GpuFenceHandle GetGpuFenceHandle() const;

  // Casts for use with the GLES interface.
  ClientGpuFence AsClientGpuFence();
  static GpuFence* FromClientGpuFence(ClientGpuFence gpu_fence);

  // Wait for the GpuFence to become ready.
  void Wait();

  enum FenceStatus { kSignaled, kNotSignaled, kInvalid };
  static FenceStatus GetStatusChangeTime(int fd, base::TimeTicks* time);

  base::TimeTicks GetMaxTimestamp() const;

 private:
  gfx::GpuFenceHandleType type_;
#if defined(OS_POSIX)
  base::ScopedFD owned_fd_;
#endif

  DISALLOW_COPY_AND_ASSIGN(GpuFence);
};

}  // namespace gfx

#endif  // UI_GL_GPU_FENCE_H_
