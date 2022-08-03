// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <optional>

#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/pipeline_descriptor.h"

namespace impeller::renderpass {

std::optional<vk::UniqueRenderPass> CreateRenderPass(vk::Device device,
                                                     SampleCount sample_count,
                                                     std::string_view label);

}
