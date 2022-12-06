// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_render_target.h"

#include <utility>

#include "flutter/fml/logging.h"

namespace flutter {

EmbedderRenderTarget::EmbedderRenderTarget(FlutterBackingStore backing_store,
                                           AcquireSurfaceCallback acquire_surface,
                                           SkISize size,
                                           fml::closure on_release)
    : backing_store_(backing_store),
      acquire_surface_(std::move(acquire_surface)),
      size_(size),
      on_release_(std::move(on_release)) {
  // TODO(38468): The optimization to elide backing store updates between frames
  // has not been implemented yet.
  backing_store_.did_update = true;
  FML_DCHECK(acquire_surface_);
}

EmbedderRenderTarget::~EmbedderRenderTarget() {
  if (on_release_) {
    on_release_();
  }
}

const FlutterBackingStore* EmbedderRenderTarget::GetBackingStore() const {
  return &backing_store_;
}

sk_sp<SkSurface> EmbedderRenderTarget::AcquireRenderSurface() const {
  return acquire_surface_(backing_store_, size_);
}

}  // namespace flutter
