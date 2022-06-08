// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/picture.h"
#include <memory>

#include "impeller/base/validation.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

std::shared_ptr<Image> Picture::RenderToImage(AiksContext& context) {
  auto coverage = pass->GetElementsCoverage();
  if (!coverage.has_value() || coverage->IsEmpty()) {
    return nullptr;
  }

  // This texture isn't host visible, but we might want to add host visible
  // features to Image someday.
  auto target = RenderTarget::CreateOffscreen(
      *context.GetContext(),
      ISize(coverage->GetRight(), coverage->GetBottom()));
  if (!target.IsValid()) {
    VALIDATION_LOG << "Could not create valid RenderTarget.";
    return nullptr;
  }

  if (!context.Render(*this, target)) {
    VALIDATION_LOG << "Could not render Picture to Texture.";
    return nullptr;
  }

  auto texture = target.GetRenderTargetTexture();
  if (!texture) {
    VALIDATION_LOG << "RenderTarget has no target texture.";
    return nullptr;
  }
  return std::make_shared<Image>(std::move(texture));
}

}  // namespace impeller
