// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/picture.h"
#include <memory>

#include "impeller/entity/entity.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

std::shared_ptr<Image> Picture::RenderToImage(AiksContext& renderer) {
  auto coverage = pass->GetElementsCoverage();
  auto target = RenderTarget::CreateOffscreen(
      *renderer.context_, ISize(coverage->GetRight(), coverage->GetBottom()));

  if (!renderer.Render(*this, target)) {
    return nullptr;
  }

  return std::make_shared<Image>(target.GetRenderTargetTexture());
}

}  // namespace impeller
