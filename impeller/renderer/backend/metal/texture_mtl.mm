// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/texture_mtl.h"

#include <QuartzCore/CAMetalLayer.h>

#include "fml/synchronization/count_down_latch.h"
#include "fml/trace_event.h"
#include "impeller/base/validation.h"
#include "impeller/core/texture_descriptor.h"

namespace impeller {

TextureMTL::TextureMTL(const TextureDescriptor& desc) : Texture(desc) {}

/// @brief an implementation of TextureMTL that is backed by a real Metal
/// texture.
class BetterNameTextureMTL final : public TextureMTL {
 public:
  BetterNameTextureMTL(TextureDescriptor p_desc,
                       id<MTLTexture> texture,
                       bool wrapped)
      : TextureMTL(p_desc), texture_(texture) {
    const auto& desc = GetTextureDescriptor();

    if (!desc.IsValid() || !texture_) {
      return;
    }

    if (desc.size != GetSize()) {
      VALIDATION_LOG
          << "The texture and its descriptor disagree about its size.";
      return;
    }

    is_wrapped_ = wrapped;
    is_valid_ = true;
  }

  // |Texture|
  ~BetterNameTextureMTL() override = default;

 private:
  id<MTLTexture> texture_ = nullptr;
  bool is_valid_ = false;
  bool is_wrapped_ = false;

  // |TextureMTL|
  id<MTLTexture> GetMTLTexture() const override { return texture_; }

  // |TextureMTL|
  bool IsWrapped() const override { return is_wrapped_; }

  // |TextureMTL|
  bool IsDrawableBacked() const override { return false; }

  // |TextureMTL|
  id<CAMetalDrawable> WaitForNextDrawable() const override { return nil; }

  bool GenerateMipmap(id<MTLBlitCommandEncoder> encoder) override {
    if (!texture_) {
      return false;
    }

    [encoder generateMipmapsForTexture:texture_];
    mipmap_generated_ = true;

    return true;
  }

  // |Texture|
  void SetLabel(std::string_view label) override {
    [texture_ setLabel:@(label.data())];
  }
  // |Texture|
  bool OnSetContents(const uint8_t* contents,
                     size_t length,
                     size_t slice) override {
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

  // |Texture|
  bool OnSetContents(std::shared_ptr<const fml::Mapping> mapping,
                     size_t slice) override {
    // Metal has no threading restrictions. So we can pass this data along to
    // the client rendering API immediately.
    return OnSetContents(mapping->GetMapping(), mapping->GetSize(), slice);
  }

  // |Texture|
  bool IsValid() const override { return is_valid_; }

  // |Texture|
  ISize GetSize() const override {
    return {static_cast<ISize::Type>(texture_.width),
            static_cast<ISize::Type>(texture_.height)};
  }

  BetterNameTextureMTL(const BetterNameTextureMTL&) = delete;

  BetterNameTextureMTL& operator=(const BetterNameTextureMTL&) = delete;
};

/// @brief an implementation of TextureMTL that is backed by a CAMetalLayer.
///
/// When asked for a Metal texture, this texture class will reference and cache
/// the next drawable of the provided metal layer. This class is intended to be
/// used to allow referencing the onscreen Metal drawable without blocking on
/// drawable acquisition.
class DrawableTextureMTL final : public TextureMTL {
 public:
  DrawableTextureMTL(const TextureDescriptor& p_desc, CAMetalLayer* layer)
      : TextureMTL(p_desc),
        layer_(layer),
        drawable_latch_(std::make_shared<fml::CountDownLatch>(1u)) {
    const auto& desc = GetTextureDescriptor();

    if (!desc.IsValid() || !layer) {
      return;
    }

    if (ISize{static_cast<ISize::Type>(layer.drawableSize.width),
              static_cast<ISize::Type>(layer.drawableSize.height)} !=
        GetSize()) {
      VALIDATION_LOG
          << "The texture and its descriptor disagree about its size.";
      return;
    }

    is_valid_ = true;
  }

  // |Texture|
  ~DrawableTextureMTL() override = default;

 private:
  CAMetalLayer* layer_;
  bool is_valid_ = false;

  mutable bool acquired_drawable_ = false;
  mutable id<CAMetalDrawable> drawable_ = nullptr;
  mutable id<MTLTexture> texture_ = nullptr;
  mutable std::shared_ptr<fml::CountDownLatch> drawable_latch_;

  id<CAMetalDrawable> WaitForNextDrawable() const override {
    drawable_latch_->Wait();
    return drawable_;
  }

  // |TextureMTL|
  id<MTLTexture> GetMTLTexture() const override {
    // If the drawable has already been acquired, then return the cached Metal
    // texture. If next drawable returned a nil texture, this value will be nil.
    if (acquired_drawable_) {
      return texture_;
    }

    id<CAMetalDrawable> current_drawable = nil;
    {
      TRACE_EVENT0("impeller", "WaitForNextDrawable");
      current_drawable = [layer_ nextDrawable];
    }
    acquired_drawable_ = true;
    drawable_latch_->CountDown();

    if (!current_drawable) {
      VALIDATION_LOG << "Could not acquire current drawable.";
      return nullptr;
    }
    drawable_ = current_drawable;
    texture_ = drawable_.texture;
    return texture_;
  }

  // |TextureMTL|
  bool IsWrapped() const override { return false; }

  // |TextureMTL|
  bool IsDrawableBacked() const override { return true; }

  bool GenerateMipmap(id<MTLBlitCommandEncoder> encoder) override {
    VALIDATION_LOG << "Cannot generate mipmaps for a drawable-backed texture.";
    return false;
  }

  // |Texture|
  void SetLabel(std::string_view label) override {
    // NOOP
  }

  // |Texture|
  bool OnSetContents(const uint8_t* contents,
                     size_t length,
                     size_t slice) override {
    VALIDATION_LOG << "Cannot set contents for a drawable-backed texture.";
    return false;
  }

  // |Texture|
  bool OnSetContents(std::shared_ptr<const fml::Mapping> mapping,
                     size_t slice) override {
    VALIDATION_LOG << "Cannot set contents for a drawable-backed texture.";
    return false;
  }

  // |Texture|
  bool IsValid() const override { return is_valid_; }

  // |Texture|
  ISize GetSize() const override {
    return {static_cast<ISize::Type>(layer_.drawableSize.width),
            static_cast<ISize::Type>(layer_.drawableSize.height)};
  }

  DrawableTextureMTL(const DrawableTextureMTL&) = delete;

  DrawableTextureMTL& operator=(const DrawableTextureMTL&) = delete;
};

// (__bridge id<MTLTexture>)mtl_texture,

std::shared_ptr<TextureMTL> TextureMTL::Wrapper(
    TextureDescriptor desc,
    id<MTLTexture> texture,
    std::function<void()> deletion_proc) {
  if (deletion_proc) {
    return std::shared_ptr<TextureMTL>(
        new BetterNameTextureMTL(desc, texture, true),
        [deletion_proc = std::move(deletion_proc)](TextureMTL* t) {
          deletion_proc();
          delete t;
        });
  }
  return std::make_shared<BetterNameTextureMTL>(desc, texture, true);
}

std::shared_ptr<TextureMTL> TextureMTL::BetterName(TextureDescriptor desc,
                                                   id<MTLTexture> texture) {
  return std::make_shared<BetterNameTextureMTL>(desc, texture, false);
}

}  // namespace impeller
