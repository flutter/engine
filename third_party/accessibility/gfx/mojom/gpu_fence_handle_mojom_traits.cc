// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/mojom/gpu_fence_handle_mojom_traits.h"

#include "ax_build/build_config.h"
#include "mojo/public/cpp/system/platform_handle.h"

namespace mojo {

mojo::PlatformHandle StructTraits<
    gfx::mojom::GpuFenceHandleDataView,
    gfx::GpuFenceHandle>::native_fd(const gfx::GpuFenceHandle& handle) {
#if defined(OS_POSIX)
  if (handle.type != gfx::GpuFenceHandleType::kAndroidNativeFenceSync)
    return mojo::PlatformHandle();
  return mojo::PlatformHandle(base::ScopedFD(handle.native_fd.fd));
#else
  return mojo::PlatformHandle();
#endif
}

bool StructTraits<gfx::mojom::GpuFenceHandleDataView, gfx::GpuFenceHandle>::
    Read(gfx::mojom::GpuFenceHandleDataView data, gfx::GpuFenceHandle* out) {
  if (!data.ReadType(&out->type))
    return false;

  if (out->type == gfx::GpuFenceHandleType::kAndroidNativeFenceSync) {
#if defined(OS_POSIX)
    constexpr bool auto_close = true;
    out->native_fd =
        base::FileDescriptor(data.TakeNativeFd().ReleaseFD(), auto_close);
    return true;
#else
    NOTREACHED();
    return false;
#endif
  }
  return true;
}

}  // namespace mojo
