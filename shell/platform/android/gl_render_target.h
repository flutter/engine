// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_GL_RENDER_TARGET_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_GL_RENDER_TARGET_H_

#include "flutter/shell/common/platform_view.h"
#include "lib/ftl/macros.h"
#include "third_party/skia/include/core/SkSize.h"

namespace shell {

class GLRenderTarget {
 public:
  GLRenderTarget(GrContext* context, const SkISize& size);

  ~GLRenderTarget();

  bool IsValid() const;

  bool Resize(const SkISize& size);

  intptr_t GetFBO() const;

  bool RenderToWindowFBO();

 private:
  GrContext* context_;
  sk_sp<SkSurface> offscreen_buffer_;
  sk_sp<SkSurface> onscreen_buffer_;
  bool valid_ = false;

  FTL_DISALLOW_COPY_AND_ASSIGN(GLRenderTarget);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_GL_RENDER_TARGET_H_
