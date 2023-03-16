// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_H_

#include <functional>

#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"

namespace flutter {

class EmbedderStudio {
 public:
  using Builder = std::function<std::unique_ptr<EmbedderSurface>(
      std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)>;

  EmbedderStudio(Builder builder) : builder_(std::move(builder)) {}

  virtual ~EmbedderStudio() = default;

  virtual std::unique_ptr<EmbedderSurface> CreateSurface(
      std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder) {
    return builder_(external_view_embedder);
  }

  virtual sk_sp<GrDirectContext> CreateResourceContext() const {
    // TODO(dkwingsmt)
    return nullptr;
  }

 private:
  Builder builder_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_H_
