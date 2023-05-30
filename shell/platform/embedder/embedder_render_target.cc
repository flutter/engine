// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_render_target.h"

#include <optional>
#include <utility>

#include "flutter/fml/logging.h"

#include "impeller/aiks/aiks_context.h"
#include "impeller/renderer/render_target.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace flutter {

EmbedderRenderTarget::EmbedderRenderTarget(
    FlutterBackingStore backing_store,
    sk_sp<SkSurface> render_surface,
    std::shared_ptr<impeller::AiksContext> aiks_context,
    std::optional<impeller::RenderTarget> impeller_target,
    fml::closure on_release)
    : backing_store_(backing_store),
      render_surface_(std::move(render_surface)),
      aiks_context_(std::move(aiks_context)),
      impeller_target_(std::move(impeller_target)),
      on_release_(std::move(on_release)) {
  // TODO(38468): The optimization to elide backing store updates between frames
  // has not been implemented yet.
  backing_store_.did_update = true;

  if (impeller_target_) {
    FML_DCHECK(aiks_context_);
    FML_DCHECK(!render_surface_);
  } else {
    FML_CHECK(render_surface_);
  }
}

EmbedderRenderTarget::~EmbedderRenderTarget() {
  if (on_release_) {
    on_release_();
  }
}

const FlutterBackingStore* EmbedderRenderTarget::GetBackingStore() const {
  return &backing_store_;
}

sk_sp<SkSurface> EmbedderRenderTarget::GetRenderSurface() const {
  return render_surface_;
}

std::optional<impeller::RenderTarget>
EmbedderRenderTarget::GetImpellerRenderTarget() const {
  return impeller_target_;
}

std::shared_ptr<impeller::AiksContext> EmbedderRenderTarget::GetAiksContext()
    const {
  return aiks_context_;
}

SkISize EmbedderRenderTarget::GetRenderTargetSize() const {
  if (impeller_target_.has_value()) {
    auto size = impeller_target_->GetRenderTargetSize();
    return SkISize::Make(size.width, size.height);
  }
  return SkISize::Make(render_surface_->width(), render_surface_->height());
}

}  // namespace flutter
