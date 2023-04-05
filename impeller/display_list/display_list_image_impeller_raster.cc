// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/display_list_image_impeller_raster.h"

#include "flutter/fml/trace_event.h"
#include "flutter/impeller/core/allocator.h"
#include "flutter/impeller/core/texture.h"
#include "flutter/impeller/renderer/command_buffer.h"
#include "impeller/aiks/aiks_context.h"
#include "impeller/entity/contents/filters/filter_contents.h"

namespace impeller {

static std::optional<impeller::PixelFormat> ToPixelFormat(SkColorType type) {
  switch (type) {
    case kRGBA_8888_SkColorType:
      return impeller::PixelFormat::kR8G8B8A8UNormInt;
    case kRGBA_F16_SkColorType:
      return impeller::PixelFormat::kR16G16B16A16Float;
    case kBGR_101010x_XR_SkColorType:
      return impeller::PixelFormat::kB10G10R10XR;
    default:
      return std::nullopt;
  }
  return std::nullopt;
}

sk_sp<DlImageImpellerRaster> DlImageImpellerRaster::Make(
    std::weak_ptr<Context> context,
    std::shared_ptr<SkBitmap> bitmap,
    OwningContext owning_context) {
  if (!bitmap) {
    return nullptr;
  }
  return sk_sp<DlImageImpellerRaster>(new DlImageImpellerRaster(
      std::move(context), std::move(bitmap), owning_context));
}

DlImageImpellerRaster::DlImageImpellerRaster(std::weak_ptr<Context> context,
                                             std::shared_ptr<SkBitmap> bitmap,
                                             OwningContext owning_context)
    : context_(std::move(context)),
      bitmap_(std::move(bitmap)),
      owning_context_(owning_context) {}

// static
std::shared_ptr<Texture> DlImageImpellerRaster::UploadTextureToShared(
    std::shared_ptr<Context> context,
    std::shared_ptr<SkBitmap> bitmap,
    bool create_mips) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!context || !bitmap) {
    return nullptr;
  }
  const auto image_info = bitmap->info();
  const auto pixel_format = ToPixelFormat(image_info.colorType());
  if (!pixel_format) {
    FML_DLOG(ERROR) << "Pixel format unsupported by Impeller.";
    return nullptr;
  }

  impeller::TextureDescriptor texture_descriptor;
  texture_descriptor.storage_mode = impeller::StorageMode::kHostVisible;
  texture_descriptor.format = pixel_format.value();
  texture_descriptor.size = {image_info.width(), image_info.height()};
  texture_descriptor.mip_count =
      create_mips ? texture_descriptor.size.MipCount() : 1;

  auto texture =
      context->GetResourceAllocator()->CreateTexture(texture_descriptor);
  if (!texture) {
    FML_DLOG(ERROR) << "Could not create Impeller texture.";
    return nullptr;
  }

  auto mapping = std::make_shared<fml::NonOwnedMapping>(
      reinterpret_cast<const uint8_t*>(bitmap->getAddr(0, 0)),  // data
      texture_descriptor.GetByteSizeOfBaseMipLevel(),           // size
      [bitmap](auto, auto) mutable { bitmap.reset(); }          // proc
  );

  if (!texture->SetContents(mapping)) {
    FML_DLOG(ERROR) << "Could not copy contents into Impeller texture.";
    return nullptr;
  }

  texture->SetLabel(impeller::SPrintF("ui.Image(%p)", texture.get()).c_str());

  if (texture_descriptor.mip_count > 1u && create_mips) {
    auto command_buffer = context->CreateCommandBuffer();
    if (!command_buffer) {
      FML_DLOG(ERROR)
          << "Could not create command buffer for mipmap generation.";
      return nullptr;
    }
    command_buffer->SetLabel("Mipmap Command Buffer");

    auto blit_pass = command_buffer->CreateBlitPass();
    if (!blit_pass) {
      FML_DLOG(ERROR) << "Could not create blit pass for mipmap generation.";
      return nullptr;
    }
    blit_pass->SetLabel("Mipmap Blit Pass");
    blit_pass->GenerateMipmap(texture);

    blit_pass->EncodeCommands(context->GetResourceAllocator());
    if (!command_buffer->SubmitCommands()) {
      FML_DLOG(ERROR) << "Failed to submit blit pass command buffer.";
      return nullptr;
    }
  }
  return texture;
}

// |DlImage|
DlImageImpellerRaster::~DlImageImpellerRaster() = default;

// |DlImage|
sk_sp<SkImage> DlImageImpellerRaster::skia_image() const {
  return nullptr;
};

// |DlImage|
std::shared_ptr<impeller::Texture> DlImageImpellerRaster::impeller_texture()
    const {
  if (!texture_) {
    auto context = context_.lock();
    if (!context) {
      return nullptr;
    }
    texture_ = UploadTextureToShared(context, bitmap_,
                                     /*create_mips=*/false);
  }
  return texture_;
}

// |DlImage|
bool DlImageImpellerRaster::isOpaque() const {
  // Impeller doesn't currently implement opaque alpha types.
  return false;
}

// |DlImage|
bool DlImageImpellerRaster::isTextureBacked() const {
  return true;
}

// |DlImage|
SkISize DlImageImpellerRaster::dimensions() const {
  const auto size = texture_ ? texture_->GetSize() : ISize{};
  return SkISize::Make(size.width, size.height);
}

// |DlImage|
size_t DlImageImpellerRaster::GetApproximateByteSize() const {
  auto size = sizeof(*this);
  if (bitmap_) {
    size += bitmap_->info().computeMinByteSize();
  } else if (texture_) {
    size += texture_->GetTextureDescriptor().GetByteSizeOfBaseMipLevel();
  }
  return size;
}

}  // namespace impeller
