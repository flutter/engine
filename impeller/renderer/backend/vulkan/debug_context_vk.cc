// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/debug_context_vk.h"
#include "impeller/base/validation.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {
DebugContextVK::DebugContextVK(const vk::Device& logical_device)
    : logical_device_(logical_device) {}

DebugContextVK::~DebugContextVK() = default;

}  // namespace impeller
