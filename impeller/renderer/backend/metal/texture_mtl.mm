// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/texture_mtl.h"
#include <memory>

#include "fml/synchronization/count_down_latch.h"
#include "fml/trace_event.h"
#include "impeller/base/validation.h"
#include "impeller/core/texture_descriptor.h"

namespace impeller {

// std::shared_ptr<Texture> WrapperMTL(TextureDescriptor desc,
//                                     const void* mtl_texture,
//                                     std::function<void()> deletion_proc,) {
//   return TextureMTL::Wrapper(desc, (__bridge id<MTLTexture>)mtl_texture,
//                              std::move(deletion_proc));
// }

TextureMTL::TextureMTL(TextureDescriptor p_desc,
                       id<MTLTexture> texture,
                       bool wrapped,
                       CAMetalLayer* layer)
    : Texture(p_desc), texture_(texture), layer_(layer) {
  const auto& desc = GetTextureDescriptor();

  if (!desc.IsValid() || (!texture_ && !layer_)) {
    return;
  }

  if (desc.size != GetSize() && !layer_) {
    VALIDATION_LOG << "The texture and its descriptor disagree about its size.";
    return;
  }

  if (layer_) {
    latch_ = std::make_shared<fml::CountDownLatch>(1u);
  }

  is_wrapped_ = wrapped;
  is_valid_ = true;
}

std::shared_ptr<TextureMTL> TextureMTL::Wrapper(
    TextureDescriptor desc,
    CAMetalLayer* layer,
    std::function<void()> deletion_proc) {
  if (deletion_proc) {
    return std::shared_ptr<TextureMTL>(
        new TextureMTL(desc, nullptr, true, layer),
        [deletion_proc = std::move(deletion_proc)](TextureMTL* t) {
          deletion_proc();
          delete t;
        });
  }
  return std::shared_ptr<TextureMTL>(
      new TextureMTL(desc, nullptr, true, layer));
}

TextureMTL::~TextureMTL() = default;

void TextureMTL::SetLabel(std::string_view label) {
  [texture_ setLabel:@(label.data())];
}

// |Texture|
bool TextureMTL::OnSetContents(std::shared_ptr<const fml::Mapping> mapping,
                               size_t slice) {
  // Metal has no threading restrictions. So we can pass this data along to the
  // client rendering API immediately.
  return OnSetContents(mapping->GetMapping(), mapping->GetSize(), slice);
}

// |Texture|
bool TextureMTL::OnSetContents(const uint8_t* contents,
                               size_t length,
                               size_t slice) {
  if (!IsValid() || !contents || is_wrapped_) {
    return false;
  }

  const auto& desc = GetTextureDescriptor();

  // Out of bounds access.
  if (length != desc.GetByteSizeOfBaseMipLevel()) {
    return false;
  }

  const auto region =
      MTLRegionMake2D(0u, 0u, desc.size.width, desc.size.height);
  [texture_ replaceRegion:region                            //
              mipmapLevel:0u                                //
                    slice:slice                             //
                withBytes:contents                          //
              bytesPerRow:desc.GetBytesPerRow()             //
            bytesPerImage:desc.GetByteSizeOfBaseMipLevel()  //
  ];

  return true;
}

ISize TextureMTL::GetSize() const {
  if (is_wrapped_) {
    return {static_cast<ISize::Type>(layer_.drawableSize.width),
            static_cast<ISize::Type>(layer_.drawableSize.height)};
  }
  return {static_cast<ISize::Type>(texture_.width),
          static_cast<ISize::Type>(texture_.height)};
}

id<MTLTexture> TextureMTL::GetMTLTexture() const {
  if (is_wrapped_) {
    TRACE_EVENT0("texture", "TextureMTL::WaitForNextDrawable");
    if (!texture_) {
      drawable_ = [layer_ nextDrawable];
      if (!drawable_) {
        latch_->CountDown();
        return nil;
      }
      texture_ = drawable_.texture;
      latch_->CountDown();
    }
  }
  return texture_;
}

bool TextureMTL::IsValid() const {
  return is_valid_;
}

bool TextureMTL::IsWrapped() const {
  return is_wrapped_;
}

bool TextureMTL::GenerateMipmap(id<MTLBlitCommandEncoder> encoder) {
  if (!texture_) {
    return false;
  }

  [encoder generateMipmapsForTexture:texture_];
  mipmap_generated_ = true;

  return true;
}

}  // namespace impeller
