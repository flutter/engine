// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_H_

#include <memory>
#include "flutter/shell/common/surface.h"
#include "lib/ftl/macros.h"
#include "third_party/skia/include/core/SkSize.h"

namespace shell {

class AndroidSurface {
 public:
  virtual bool IsValid() const = 0;

  virtual std::unique_ptr<Surface> CreateGPUSurface() = 0;

  virtual SkISize OnScreenSurfaceSize() const = 0;

  virtual bool OnScreenSurfaceResize(const SkISize& size) const = 0;

  // TODO(chinmaygarde): This is OpenGL ES specific and makes no sense for
  // vulkan. After resource loading is refactored, this will be removed.
  virtual bool ResourceContextMakeCurrent() = 0;
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_H_
