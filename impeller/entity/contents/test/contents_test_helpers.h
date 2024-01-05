// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_TEST_CONTENTS_TEST_HELPERS_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_TEST_CONTENTS_TEST_HELPERS_H_

#include "impeller/core/shader_types.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

/// @brief Retrieve the [VertInfo] struct data from the provided [command].
template <typename T>
typename T::VertInfo* GetVertInfo(
    const BoundCommand& command,
    const std::vector<BoundBuffer>& bound_buffers) {
  const auto& end =
      bound_buffers.begin() + command.buffer_offset + command.buffer_length;
  auto resource = std::find_if(bound_buffers.begin() + command.buffer_offset,
                               end, [](const BoundBuffer& data) {
                                 return data.slot.ext_res_0 == 0u &&
                                        data.stage == ShaderStage::kVertex;
                               });
  if (resource == end) {
    return nullptr;
  }
  auto data =
      (resource->view.resource.contents + resource->view.resource.range.offset);
  return reinterpret_cast<typename T::VertInfo*>(data);
}

/// @brief Retrieve the [FragInfo] struct data from the provided [command].
template <typename T>
typename T::FragInfo* GetFragInfo(
    const BoundCommand& command,
    const std::vector<BoundBuffer>& bound_buffers) {
  const auto& end =
      bound_buffers.begin() + command.buffer_offset + command.buffer_length;
  auto resource = std::find_if(bound_buffers.begin() + command.buffer_offset,
                               end, [](const BoundBuffer& data) {
                                 return data.slot.ext_res_0 == 0u &&
                                        data.stage == ShaderStage::kFragment;
                               });
  if (resource == end) {
    return nullptr;
  }

  auto data =
      (resource->view.resource.contents + resource->view.resource.range.offset);
  return reinterpret_cast<typename T::FragInfo*>(data);
}

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_TEST_CONTENTS_TEST_HELPERS_H_
