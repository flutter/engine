// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "impeller/renderer/vertex_buffer.h"

namespace impeller {

class Tessellator;
class Path;
class HostBuffer;

/**
 * @brief Populate VertexBuffer with solid fill vertices created by tessellating
 * an input path.
 *
 * @param tessellator The tessellator
 * @param path        The path to be tessellated
 * @param buffer      The transient buffer
 * @param out_buffer  The populated vertex buffer
 * @return true       Tessellation was successful.
 * @return false      Tessellation failed.
 */
bool CreateSolidFillVertices(std::shared_ptr<Tessellator> tessellator,
                             const Path& path,
                             HostBuffer& buffer,
                             VertexBuffer* out_buffer);

}  // namespace impeller
