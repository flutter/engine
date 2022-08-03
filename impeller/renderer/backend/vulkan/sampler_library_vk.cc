// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/sampler_library_vk.h"

#include "impeller/renderer/backend/vulkan/sampler_vk.h"

#include <memory>

namespace impeller {

SamplerLibraryVK::SamplerLibraryVK(vk::Device device) : device_(device) {}

SamplerLibraryVK::~SamplerLibraryVK() = default;

std::shared_ptr<const Sampler> SamplerLibraryVK::GetSampler(
    SamplerDescriptor descriptor) {
  return std::make_shared<SamplerVK>(descriptor);
}

}  // namespace impeller
