// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/tessellation_cache.h"

#define IMPELLER_ENABLE_TESSELLATION_CACHE 1

namespace impeller {

Path::Polyline TessellationCache::GetOrCreatePolyline(
    const impeller::Path& path,
    Scalar scale) {
#if IMPELLER_ENABLE_TESSELLATION_CACHE == 1
  auto path_identifier = path.GetPathIdentifier();
  if (!path_identifier) {
    // We can't match path without a valid identifier.
    return path.CreatePolyline(scale);
  }
  PolylineKey key{*path_identifier, scale};
  auto result = polyline_cache_.Get(key);
  if (result) {
    return *result;
  }
  auto polyline = path.CreatePolyline(scale);
  polyline_cache_.Set(key, polyline);
  return polyline;
#else
  return path.CreatePolyline(scale);
#endif
}

Tessellator::Result TessellationCache::Tessellate(
    const Tessellator& tesselator,
    FillType fill_type,
    const Path::Polyline& polyline,
    const Tessellator::BuilderCallback& callback) {
#if IMPELLER_ENABLE_TESSELLATION_CACHE == 1
  auto path_identifier = polyline.original_path_identifier;
  if (!path_identifier) {
    return tesselator.Tessellate(fill_type, polyline, callback);
  }

  TessellatorKey key{*path_identifier, fill_type};
  auto result = tessellator_cache_.Get(key);
  if (result) {
    if (callback(result->vertices.data(), result->vertices.size(),
                 result->indices.data(), result->indices.size())) {
      return Tessellator::Result::kSuccess;
    } else {
      return Tessellator::Result::kInputError;
    }
  }

  return tesselator.Tessellate(
      fill_type, polyline,
      [&](const float* vertices, size_t vertices_size, const uint16_t* indices,
          size_t indices_size) {
        TessellatorData data;
        data.vertices.assign(vertices, vertices + vertices_size);
        data.indices.assign(indices, indices + indices_size);
        tessellator_cache_.Set(key, data);
        return callback(vertices, vertices_size, indices, indices_size);
      });
#else
  return tesselator.Tessellate(fill_type, polyline, callback);
#endif
}

}  // namespace impeller
