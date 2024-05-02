// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/texture_mtl.h"
#include <memory>

#include "impeller/base/validation.h"
#include "impeller/core/texture_descriptor.h"

namespace impeller {

std::shared_ptr<Texture> WrapperMTL(TextureDescriptor desc,
                                    const void* mtl_texture,
                                    std::function<void()> deletion_proc) {
  return TextureMTL::Wrapper(desc, (__bridge id<MTLTexture>)mtl_texture,
                             std::move(deletion_proc));
}

TextureMTL::TextureMTL(TextureDescriptor p_desc,
                       const AcquireTextureProc& aquire_proc,
                       bool wrapped,
                       bool drawable)
    : Texture(p_desc), aquire_proc_(aquire_proc), is_drawable_(drawable) {
  const auto& desc = GetTextureDescriptor();

  if (!desc.IsValid() || !aquire_proc) {
    return;
  }

  if (desc.size != GetSize()) {
    VALIDATION_LOG << "The texture and its descriptor disagree about its size.";
    return;
  }

  is_wrapped_ = wrapped;
  is_valid_ = true;
}

std::shared_ptr<TextureMTL> TextureMTL::Wrapper(
    TextureDescriptor desc,
    id<MTLTexture> texture,
    std::function<void()> deletion_proc) {
  if (deletion_proc) {
    return std::shared_ptr<TextureMTL>(
        new TextureMTL(
            desc, [texture]() { return texture; }, true),
        [deletion_proc = std::move(deletion_proc)](TextureMTL* t) {
          deletion_proc();
          delete t;
        });
  }
  return std::shared_ptr<TextureMTL>(
      new TextureMTL(desc, [texture]() { return texture; }, true));
}

std::shared_ptr<TextureMTL> TextureMTL::Create(TextureDescriptor desc,
                                               id<MTLTexture> texture) {
  return std::make_shared<TextureMTL>(desc, [texture]() { return texture; });
}

TextureMTL::~TextureMTL() = default;

void TextureMTL::SetLabel(std::string_view label) {
  if (is_drawable_) {
    return;
  }
  [aquire_proc_() setLabel:@(label.data())];
}

ISize TextureMTL::GetSize() const {
  if (is_drawable_) {
    return GetTextureDescriptor().size;
  }
  const auto& texture = aquire_proc_();
  return {static_cast<ISize::Type>(texture.width),
          static_cast<ISize::Type>(texture.height)};
}

id<MTLTexture> TextureMTL::GetMTLTexture() const {
  return aquire_proc_();
}

bool TextureMTL::IsValid() const {
  return is_valid_;
}

bool TextureMTL::IsWrapped() const {
  return is_wrapped_;
}

bool TextureMTL::IsDrawable() const {
  return is_drawable_;
}

bool TextureMTL::GenerateMipmap(id<MTLBlitCommandEncoder> encoder) {
  if (is_drawable_) {
    return false;
  }

  auto texture = aquire_proc_();
  if (!texture) {
    return false;
  }

  [encoder generateMipmapsForTexture:texture];
  mipmap_generated_ = true;

  return true;
}

}  // namespace impeller
