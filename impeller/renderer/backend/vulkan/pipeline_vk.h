// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/pipeline.h"

namespace impeller {

struct PipelineVKCreateInfo {
  vk::UniquePipeline pipeline;
  vk::UniqueRenderPass render_pass;
};

class PipelineVK final : public Pipeline,
                         public BackendCast<PipelineVK, Pipeline> {
 public:
  PipelineVK(std::weak_ptr<PipelineLibrary> library,
             PipelineDescriptor desc,
             std::unique_ptr<PipelineVKCreateInfo> create_info);

  // |Pipeline|
  ~PipelineVK() override = default;

 private:
  friend class PipelineLibraryVK;

  // |Pipeline|
  bool IsValid() const override;

  bool is_valid_ = false;
  vk::UniquePipeline pipeline_;
  vk::UniqueRenderPass render_pass_;

  FML_DISALLOW_COPY_AND_ASSIGN(PipelineVK);
};

}  // namespace impeller
