// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/core/allocator.h"

#include "impeller/base/validation.h"
#include "impeller/core/device_buffer.h"
#include "impeller/core/formats.h"
#include "impeller/core/range.h"

namespace impeller {

Allocator::Allocator() = default;

Allocator::~Allocator() = default;

std::shared_ptr<DeviceBuffer> Allocator::CreateBufferWithCopy(
    const uint8_t* buffer,
    size_t length) {
  DeviceBufferDescriptor desc;
  desc.size = length;
  desc.storage_mode = StorageMode::kHostVisible;
  auto new_buffer = CreateBuffer(desc);

  if (!new_buffer) {
    return nullptr;
  }

  auto entire_range = Range{0, length};

  if (!new_buffer->CopyHostBuffer(buffer, entire_range)) {
    return nullptr;
  }

  return new_buffer;
}

std::shared_ptr<DeviceBuffer> Allocator::CreateBufferWithCopy(
    const fml::Mapping& mapping) {
  return CreateBufferWithCopy(mapping.GetMapping(), mapping.GetSize());
}

std::shared_ptr<DeviceBuffer> Allocator::CreateBuffer(
    const DeviceBufferDescriptor& desc) {
  return OnCreateBuffer(desc);
}

std::shared_ptr<Texture> Allocator::CreateTexture(
    const TextureDescriptor& desc) {
  const auto max_size = GetMaxTextureSizeSupported();
  if (desc.size.width > max_size.width || desc.size.height > max_size.height) {
    VALIDATION_LOG << "Requested texture size " << desc.size
                   << " exceeds maximum supported size of " << max_size;
    return nullptr;
  }
  total_count_++;
  if (desc.ignore_cache) {
    return OnCreateTexture(desc);
  }
  if (desc.storage_mode != StorageMode::kHostVisible) {
    for (auto& td : data_to_recycle_) {
      const auto other_desc = td.texture->GetTextureDescriptor();
      if (!td.used_this_frame &&
          desc.size.width == other_desc.size.width &&
          desc.size.height == other_desc.size.height &&
          desc.storage_mode == other_desc.storage_mode &&
          desc.format == other_desc.format && desc.usage == other_desc.usage &&
          desc.sample_count == other_desc.sample_count &&
          desc.type == other_desc.type) {
        hit_count_++;
        td.used_this_frame = true;
        return td.texture;
      }
    }
    auto result = OnCreateTexture(desc);
    data_to_recycle_.push_back({.used_this_frame = true, .texture = result});
    return result;
  }

  return OnCreateTexture(desc);
}

uint16_t Allocator::MinimumBytesPerRow(PixelFormat format) const {
  return BytesPerPixelForPixelFormat(format);
}

}  // namespace impeller
