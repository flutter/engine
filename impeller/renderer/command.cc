// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/command.h"

#include <utility>

#include "impeller/base/validation.h"
#include "impeller/core/formats.h"

namespace impeller {

bool Command::BindVertices(VertexBuffer buffer) {
  if (buffer.index_type == IndexType::kUnknown) {
    VALIDATION_LOG << "Cannot bind vertex buffer with an unknown index type.";
    return false;
  }

  vertex_buffer = std::move(buffer);
  return true;
}

}  // namespace impeller
