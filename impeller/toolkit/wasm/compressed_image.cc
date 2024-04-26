// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define STB_IMAGE_IMPLEMENTATION

#include "impeller/toolkit/wasm/compressed_image.h"

#include <stb_image.h>

#include "impeller/base/validation.h"

namespace impeller::wasm {

DecompressedImage CreateDecompressedTextureMapping(
    const fml::Mapping& mapping) {
  int x = 0;
  int y = 0;
  int channels = 0;

  stbi_uc* decoded = ::stbi_load_from_memory(mapping.GetMapping(),  //
                                             mapping.GetSize(),     //
                                             &x,                    //
                                             &y,                    //
                                             &channels,             //
                                             STBI_rgb_alpha         //
  );

  if (decoded == nullptr || x < 0 || y < 0) {
    FML_LOG(ERROR) << "Could not decompress image";
    return {};
  }

  auto decompressed = std::make_shared<fml::NonOwnedMapping>(
      decoded,                                               //
      x * y * 4,                                             //
      [decoded](auto, auto) { ::stbi_image_free(decoded); }  //
  );

  return {
      .mapping = std::move(decompressed),
      .size = ISize::MakeWH(x, y),
  };
}

std::shared_ptr<Texture> CreateTextureFromCompressedImageData(
    const Context& context,
    const fml::Mapping& mapping) {
  if (!context.IsValid()) {
    return nullptr;
  }

  auto decompressed = CreateDecompressedTextureMapping(mapping);
  if (!decompressed.mapping) {
    return nullptr;
  }

  auto texture_descriptor = TextureDescriptor{};
  texture_descriptor.storage_mode = StorageMode::kHostVisible;
  texture_descriptor.format = PixelFormat::kR8G8B8A8UNormInt;
  texture_descriptor.size = decompressed.size;
  texture_descriptor.mip_count = 1u;

  auto texture =
      context.GetResourceAllocator()->CreateTexture(texture_descriptor);
  if (!texture) {
    VALIDATION_LOG << "Could not allocate texture for fixture.";
    return nullptr;
  }

  auto uploaded = texture->SetContents(decompressed.mapping);
  if (!uploaded) {
    VALIDATION_LOG << "Could not upload texture to device memory for fixture.";
    return nullptr;
  }
  return texture;
}

}  // namespace impeller::wasm
