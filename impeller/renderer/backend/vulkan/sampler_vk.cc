// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/sampler_vk.h"
#include "impeller/renderer/sampler.h"

namespace impeller {

SamplerVK::~SamplerVK() = default;

SamplerVK::SamplerVK(SamplerDescriptor desc) : Sampler(desc) {}

bool SamplerVK::IsValid() const {
  return true;
}

}  // namespace impeller
