// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/core/texture.h"

#include "impeller/base/validation.h"

namespace impeller {

Texture::Texture(TextureDescriptor desc) : desc_(desc) {}

Texture::~Texture() = default;

bool Texture::SetContents(const uint8_t* contents,
                          size_t length,
                          std::optional<IRect> region,
                          size_t slice,
                          bool is_opaque) {
  if (!IsSliceValid(slice)) {
    VALIDATION_LOG << "Invalid slice for texture.";
    return false;
  }
  auto& desc = GetTextureDescriptor();
  if (!OnSetContents(contents, length,
                     region.value_or(IRect::MakeLTRB(0, 0, desc.size.width,
                                                     desc.size.height)),
                     slice)) {
    return false;
  }
  coordinate_system_ = TextureCoordinateSystem::kUploadFromHost;
  is_opaque_ = is_opaque;
  return true;
}

bool Texture::SetContents(const BufferView& buffer_view,
                          std::optional<IRect> region,
                          size_t slice,
                          bool is_opaque) {
  if (!IsSliceValid(slice)) {
    VALIDATION_LOG << "Invalid slice for texture.";
    return false;
  }
  if (!buffer_view) {
    return false;
  }
  auto& desc = GetTextureDescriptor();
  if (!OnSetContents(buffer_view,
                     region.value_or(IRect::MakeLTRB(0, 0, desc.size.width,
                                                     desc.size.height)),
                     slice)) {
    return false;
  }
  coordinate_system_ = TextureCoordinateSystem::kUploadFromHost;
  is_opaque_ = is_opaque;
  return true;
}

bool Texture::IsOpaque() const {
  return is_opaque_;
}

size_t Texture::GetMipCount() const {
  return GetTextureDescriptor().mip_count;
}

const TextureDescriptor& Texture::GetTextureDescriptor() const {
  return desc_;
}

bool Texture::IsSliceValid(size_t slice) const {
  switch (desc_.type) {
    case TextureType::kTexture2D:
    case TextureType::kTexture2DMultisample:
    case TextureType::kTextureExternalOES:
      return slice == 0;
    case TextureType::kTextureCube:
      return slice <= 5;
  }
  FML_UNREACHABLE();
}

void Texture::SetCoordinateSystem(TextureCoordinateSystem coordinate_system) {
  coordinate_system_ = coordinate_system;
}

TextureCoordinateSystem Texture::GetCoordinateSystem() const {
  return coordinate_system_;
}

Scalar Texture::GetYCoordScale() const {
  return 1.0;
}

bool Texture::NeedsMipmapGeneration() const {
  return !mipmap_generated_ && desc_.mip_count > 1;
}

}  // namespace impeller
