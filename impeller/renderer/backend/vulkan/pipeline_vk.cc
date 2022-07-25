// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/pipeline_vk.h"

namespace impeller {

PipelineVK::PipelineVK(std::weak_ptr<PipelineLibrary> library,
                       PipelineDescriptor desc,
                       std::unique_ptr<PipelineVKCreateInfo> create_info)
    : Pipeline(std::move(library), std::move(desc)),
      pipeline_(std::move(create_info->pipeline)),
      render_pass_(std::move(create_info->render_pass)) {
  is_valid_ = pipeline_ && render_pass_;
}

bool PipelineVK::IsValid() const {
  return is_valid_;
}

}  // namespace impeller
