
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/scene/importer/vertices_builder.h"

#include <cstring>
#include <limits>

#include "impeller/scene/importer/conversions.h"
#include "impeller/scene/importer/scene_flatbuffers.h"

namespace impeller {
namespace scene {
namespace importer {

VerticesBuilder::VerticesBuilder() = default;

std::map<VerticesBuilder::Attribute, VerticesBuilder::AttributeProperties>
    VerticesBuilder::kAttributes = {
        {VerticesBuilder::Attribute::kPosition,
         {.offset_bytes = offsetof(Vertex, position),
          .size_bytes = sizeof(Vertex::position),
          .component_count = 3}},
        {VerticesBuilder::Attribute::kNormal,
         {.offset_bytes = offsetof(Vertex, normal),
          .size_bytes = sizeof(Vertex::normal),
          .component_count = 3}},
        {VerticesBuilder::Attribute::kTangent,
         {.offset_bytes = offsetof(Vertex, tangent),
          .size_bytes = sizeof(Vertex::tangent),
          .component_count = 4}},
        {VerticesBuilder::Attribute::kTextureCoords,
         {.offset_bytes = offsetof(Vertex, texture_coords),
          .size_bytes = sizeof(Vertex::texture_coords),
          .component_count = 2}},
        {VerticesBuilder::Attribute::kColor,
         {.offset_bytes = offsetof(Vertex, color),
          .size_bytes = sizeof(Vertex::color),
          .component_count = 4}}};

void VerticesBuilder::WriteFBVertices(std::vector<fb::Vertex>& vertices) const {
  vertices.resize(0);
  for (auto& v : vertices_) {
    vertices.push_back(fb::Vertex(
        ToFBVec3(v.position), ToFBVec3(v.normal), ToFBVec4(v.tangent),
        ToFBVec2(v.texture_coords), ToFBColor(v.color)));
  }
}

template <typename SourceType, size_t Divisor>
void WriteScalars(void* destination,
                  const void* source,
                  size_t component_count) {
  for (size_t i = 0; i < component_count; i++) {
    const SourceType* s = reinterpret_cast<const SourceType*>(source) + i;
    Scalar v = static_cast<Scalar>(*s) / Divisor;
    Scalar* dest = reinterpret_cast<Scalar*>(destination) + i;
    *dest = v;
  }
}

static std::map<
    VerticesBuilder::ComponentType,
    std::function<
        void(void* destination, const void* source, size_t component_count)>>
    kAttributeWriters = {
        {VerticesBuilder::ComponentType::kSignedByte,
         WriteScalars<int8_t, std::numeric_limits<int8_t>::max()>},
        {VerticesBuilder::ComponentType::kUnsignedByte,
         WriteScalars<uint8_t, std::numeric_limits<uint8_t>::max()>},
        {VerticesBuilder::ComponentType::kSignedShort,
         WriteScalars<int16_t, std::numeric_limits<int16_t>::max()>},
        {VerticesBuilder::ComponentType::kUnsignedShort,
         WriteScalars<uint16_t, std::numeric_limits<uint16_t>::max()>},
        {VerticesBuilder::ComponentType::kSignedInt,
         WriteScalars<int32_t, std::numeric_limits<int32_t>::max()>},
        {VerticesBuilder::ComponentType::kUnsignedInt,
         WriteScalars<uint32_t, std::numeric_limits<uint32_t>::max()>},
        {VerticesBuilder::ComponentType::kFloat, WriteScalars<float, 1>},
};

void VerticesBuilder::SetAttributeFromBuffer(Attribute attribute,
                                             ComponentType component_type,
                                             const void* buffer_start,
                                             size_t stride_bytes,
                                             size_t count) {
  if (count > vertices_.size()) {
    vertices_.resize(count, Vertex());
  }

  const auto& properties = kAttributes[attribute];
  const auto& writer = kAttributeWriters[component_type];
  for (size_t i = 0; i < count; i++) {
    const char* source =
        reinterpret_cast<const char*>(buffer_start) + stride_bytes * i;
    char* destination =
        reinterpret_cast<char*>(&vertices_.data()[i]) + properties.offset_bytes;

    writer(destination, source, properties.component_count);
  }
}

}  // namespace importer
}  // namespace scene
}  // namespace impeller
