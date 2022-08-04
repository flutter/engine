// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/texture_vk.h"
#include "impeller/renderer/formats.h"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {
TextureVK::TextureVK(TextureDescriptor desc,
                     vk::Image image,
                     vk::UniqueImageView image_view,
                     vk::Format image_format,
                     vk::Extent2D extent)
    : Texture(desc),
      image_(image),
      image_view_(std::move(image_view)),
      image_format_(image_format),
      extent_(extent) {}

TextureVK::~TextureVK() = default;

void TextureVK::SetLabel(std::string_view label) {
  FML_UNREACHABLE();
}

bool TextureVK::OnSetContents(const uint8_t* contents,
                              size_t length,
                              size_t slice) {
  FML_UNREACHABLE();
}

bool TextureVK::OnSetContents(std::shared_ptr<const fml::Mapping> mapping,
                              size_t slice) {
  FML_UNREACHABLE();
}

bool TextureVK::IsValid() const {
  FML_UNREACHABLE();
}

ISize TextureVK::GetSize() const {
  FML_UNREACHABLE();
}

//

}  // namespace impeller
