// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/dithering_utils.h"

namespace impeller {

constexpr int kImgSize =
    8;  // if changed, also change value in sk_dither_shader

/// Generate an 8x8 lookup table for the ordered dither used by gradient
/// shaders.
std::shared_ptr<Texture> GenerateDitherLUT(
    Allocator& allocator,
    PixelFormat pixel_format,
    std::shared_ptr<BlitPass>& blit_pass) {
  uint8_t data[64];
  for (int x = 0; x < kImgSize; ++x) {
    for (int y = 0; y < kImgSize; ++y) {
      // The computation of 'm' and 'value' is lifted from CPU backend.
      unsigned int m = (y & 1) << 5 | (x & 1) << 4 | (y & 2) << 2 |
                       (x & 2) << 1 | (y & 4) >> 1 | (x & 4) >> 2;
      float value = float(m) * 1.0 / 64.0 - 63.0 / 128.0;
      // Bias by 0.5 to be in 0..1, mul by 255 and round to nearest int to make
      // byte.
      data[y * 8 + x] = (uint8_t)((value + 0.5) * 255.f + 0.5f);
    }
  }
  auto mapping = fml::NonOwnedMapping(data, 64);
  auto buffer = allocator.CreateBufferWithCopy(mapping);

  TextureDescriptor descriptor;
  descriptor.format = pixel_format;
  descriptor.size = ISize{8, 8};
  descriptor.storage_mode = StorageMode::kDevicePrivate;

  auto texture = allocator.CreateTexture(descriptor);
  if (!blit_pass->AddCopy(DeviceBuffer::AsBufferView(std::move(buffer)),
                          texture)) {
    return nullptr;
  }
  return texture;
}

}  // namespace impeller
