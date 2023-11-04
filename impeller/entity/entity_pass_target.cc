// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/entity_pass_target.h"

#include "impeller/base/validation.h"
#include "impeller/core/formats.h"
#include "impeller/core/texture.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

EntityPassTarget::EntityPassTarget(const RenderTarget& render_target)
    : target_(render_target) {}

std::shared_ptr<Texture> EntityPassTarget::Flip(
    RenderTargetAllocator& allocator) {
  auto color0 = target_.GetColorAttachments().find(0)->second;
  if (!color0.resolve_texture) {
    VALIDATION_LOG << "EntityPassTarget Flip should never be called for a "
                      "non-MSAA target.";
    // ...because there is never a circumstance where doing so would be
    // necessary. Unlike MSAA passes, non-MSAA passes can be trivially loaded
    // with `LoadAction::kLoad`.
    return color0.texture;
  }

  if (!secondary_color_texture_) {
    // The second texture is allocated lazily to avoid unused allocations.
    TextureDescriptor new_descriptor =
        color0.resolve_texture->GetTextureDescriptor();
    secondary_color_texture_ = allocator.CreateTexture(new_descriptor);

    if (!secondary_color_texture_) {
      return nullptr;
    }
  }

  std::swap(color0.resolve_texture, secondary_color_texture_);

  target_.SetColorAttachment(color0, 0);

  // Return the previous backdrop texture, which is safe to read in the next
  // render pass that attaches `target_`.
  return secondary_color_texture_;
}

const RenderTarget& EntityPassTarget::GetRenderTarget() const {
  return target_;
}

bool EntityPassTarget::IsValid() const {
  return target_.IsValid();
}

}  // namespace impeller
