// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_H_

#include <functional>

#include "flutter/flow/studio.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"

namespace flutter {

class EmbedderStudio {
 public:
  EmbedderStudio() {}

  virtual ~EmbedderStudio() = default;

  virtual bool IsValid() const = 0;

  virtual std::unique_ptr<Studio> CreateGPUStudio() = 0;

  virtual std::unique_ptr<EmbedderSurface> CreateSurface() = 0;

  virtual sk_sp<GrDirectContext> CreateResourceContext() const = 0;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderStudio);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_H_
