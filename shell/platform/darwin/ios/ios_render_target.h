// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_RENDER_TARGET_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_RENDER_TARGET_H_

#include <cstdint>

#include "flutter/fml/macros.h"

namespace flutter {

class IOSRenderTarget {
 public:
  IOSRenderTarget();

  virtual ~IOSRenderTarget();

  virtual bool IsValid() const = 0;

  virtual bool PresentRenderBuffer() const = 0;

  virtual intptr_t GetFramebuffer() const = 0;

  virtual bool UpdateStorageSizeIfNecessary() = 0;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(IOSRenderTarget);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_RENDER_TARGET_H_
