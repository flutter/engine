// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/core/shader_types.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

/// @brief Retrieve the [VertInfo] struct data from the provided [command].
template <typename T>
typename T::VertInfo* GetVertInfo(const Command& command,
                                  const RenderPass& render_pass) {
  auto resource = std::find_if(
      render_pass.GetBoundBuffers().begin() + command.buffer_bindings.offset,
      render_pass.GetBoundBuffers().begin() + command.buffer_bindings.offset +
          command.buffer_bindings.length,
      [](const BoundBuffer& data) {
        return data.slot.ext_res_0 == 0u && data.stage == ShaderStage::kVertex;
      });
  if (resource == render_pass.GetBoundBuffers().end()) {
    return nullptr;
  }

  auto data =
      (resource->view.resource.contents + resource->view.resource.range.offset);
  return reinterpret_cast<typename T::VertInfo*>(data);
}

/// @brief Retrieve the [FragInfo] struct data from the provided [command].
template <typename T>
typename T::FragInfo* GetFragInfo(const Command& command,
                                  const RenderPass& render_pass) {
  auto resource = std::find_if(
      render_pass.GetBoundBuffers().begin() + command.buffer_bindings.offset,
      render_pass.GetBoundBuffers().begin() + command.buffer_bindings.offset +
          command.buffer_bindings.length,
      [](const BoundBuffer& data) {
        return data.slot.ext_res_0 == 0u &&
               data.stage == ShaderStage::kFragment;
      });
  if (resource == render_pass.GetBoundBuffers().end()) {
    return nullptr;
  }

  auto data =
      (resource->view.resource.contents + resource->view.resource.range.offset);
  return reinterpret_cast<typename T::FragInfo*>(data);
}

}  // namespace impeller
