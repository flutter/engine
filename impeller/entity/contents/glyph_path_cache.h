// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "flutter/fml/macros.h"
#include "impeller/core/texture.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/rect.h"
#include "impeller/renderer/pipeline.h"
#include "impeller/typographer/font_glyph_pair.h"
#include "impeller/typographer/text_frame.h"

namespace impeller {

struct CacheData {
  std::vector<Point> vertices;
  bool used;
};

class GlyphCacheContext {
 public:
  GlyphCacheContext() = default;

  ~GlyphCacheContext() = default;

 private:
  std::unordered_map<FontGlyphPair,
                     CacheData,
                     FontGlyphPair::Hash,
                     FontGlyphPair::Equal>
      paths_;
  bool dirty_ = true;
}

class GlyphPathCache {
 public:
  GlyphPathCache() = default;

  ~GlyphPathCache() = default;

  void AddTextFrame(const TextFrame& frame);

  void AddFontGlyphPair(FontGlyphPair pair);

  std::optional<const std::vector<Point>> FindFontGlyphVertices(
      const FontGlyphPair& pair) const;

  bool Prepare(const ContentContext& renderer);

  void Reset();

 private:
  std::unordered_map<FontGlyphPair,
                     CacheData,
                     FontGlyphPair::Hash,
                     FontGlyphPair::Equal>
      paths_;
  bool dirty_ = true;

  FML_DISALLOW_COPY_AND_ASSIGN(GlyphPathCache);
};

}  // namespace impeller
