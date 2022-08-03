// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/render_pass_vk.h"
#include "fml/logging.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {
RenderPassVK::RenderPassVK(std::shared_ptr<ContextVK> context,
                           RenderTarget target)
    : RenderPass(target), context_(context) {}

RenderPassVK::~RenderPassVK() {}

bool RenderPassVK::IsValid() const {
  FML_UNREACHABLE();
}

void RenderPassVK::OnSetLabel(std::string label) {
  // context_->SetDebugName(render_pass_->debugReportObjectType, label);
}

bool RenderPassVK::EncodeCommands(
    const std::shared_ptr<Allocator>& transients_allocator) const {
  FML_UNREACHABLE();
}

}  // namespace impeller
