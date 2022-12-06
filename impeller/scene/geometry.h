// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "impeller/geometry/vector.h"
#include "impeller/renderer/allocator.h"
#include "impeller/renderer/device_buffer.h"
#include "impeller/renderer/vertex_buffer.h"
#include "impeller/scene/importer/scene_flatbuffers.h"

namespace impeller {
namespace scene {

class CuboidGeometry;
class VertexBufferGeometry;

class Geometry {
 public:
  static std::shared_ptr<CuboidGeometry> MakeCuboid(Vector3 size);

  static std::shared_ptr<VertexBufferGeometry> MakeVertexBuffer(
      VertexBuffer vertex_buffer);

  static std::shared_ptr<VertexBufferGeometry> MakeFromFBMesh(
      const fb::StaticMesh& mesh,
      Allocator& allocator);

  virtual VertexBuffer GetVertexBuffer(Allocator& allocator) const = 0;
};

class CuboidGeometry final : public Geometry {
 public:
  void SetSize(Vector3 size);

  VertexBuffer GetVertexBuffer(Allocator& allocator) const override;

 private:
  Vector3 size_;
};

class VertexBufferGeometry final : public Geometry {
 public:
  void SetVertexBuffer(VertexBuffer vertex_buffer);

  VertexBuffer GetVertexBuffer(Allocator& allocator) const override;

 private:
  VertexBuffer vertex_buffer_;
};

}  // namespace scene
}  // namespace impeller
