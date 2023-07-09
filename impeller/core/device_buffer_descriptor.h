// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef>

#include "impeller/core/formats.h"

namespace impeller {

using BufferUsageMask = uint64_t;

enum class BufferUsage : TextureUsageMask {
  kShaderRead = 0,
  kTransferSrc = 1 << 0,
  kTransferDst = 1 << 1,
};

struct DeviceBufferDescriptor {
  StorageMode storage_mode = StorageMode::kDeviceTransient;
  size_t size = 0u;
  BufferUsageMask buffer_usage =
      static_cast<BufferUsageMask>(BufferUsage::kShaderRead) |
      static_cast<BufferUsageMask>(BufferUsage::kTransferSrc) |
      static_cast<BufferUsageMask>(BufferUsage::kTransferDst);
};

}  // namespace impeller
