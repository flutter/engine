// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/glyph_path_cache.h"

#include <CoreText/CoreText.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

Path ToPath(CGPathRef path) {
  PathBuilder builder;
  PathBuilder* builder_ref = &builder;
  CGPathApplyWithBlock(path, ^(const CGPathElement* element) {
    switch (element->type) {
      case kCGPathElementMoveToPoint: {
        CGPoint point = element->points[0];
        builder_ref->MoveTo(
            {static_cast<Scalar>(point.x), static_cast<Scalar>(point.y)});
        break;
      }
      case kCGPathElementAddLineToPoint: {
        CGPoint point = element->points[0];
        builder_ref->LineTo(
            {static_cast<Scalar>(point.x), static_cast<Scalar>(point.y)});
        break;
      }
      case kCGPathElementAddCurveToPoint: {
        CGPoint control = element->points[0];
        CGPoint p1 = element->points[1];
        CGPoint p2 = element->points[2];
        builder_ref->CubicCurveTo(
            {static_cast<Scalar>(control.x), static_cast<Scalar>(control.y)},
            {static_cast<Scalar>(p1.x), static_cast<Scalar>(p2.y)},
            {static_cast<Scalar>(p1.x), static_cast<Scalar>(p2.y)});
        break;
      }
      case kCGPathElementAddQuadCurveToPoint: {
        CGPoint control = element->points[0];
        CGPoint point = element->points[1];
        builder_ref->QuadraticCurveTo(
            {
                static_cast<Scalar>(control.x),
                static_cast<Scalar>(control.y),
            },
            {static_cast<Scalar>(point.x), static_cast<Scalar>(point.y)});
        break;
      }
      case kCGPathElementCloseSubpath: {
        builder_ref->Close();
        break;
      }
    }
  });
  return builder.TakePath(FillType::kOdd);
}

void GlyphCacheContext::Reset() {
  dirty_ = true;
  paths_.clear();
  return;
  // std::vector<FontGlyphPair> pairs_to_remove;
  // for (auto& data : paths_) {
  //   if (!data.second.used) {
  //     pairs_to_remove.push_back(data.first);
  //   } else {
  //     data.second.used = false;
  //   }
  // }
  // for (auto& key : pairs_to_remove) {
  //   paths_.erase(key);
  // }
}

std::optional<const std::vector<Point>>
GlyphCacheContext::FindFontGlyphVertices(const FontGlyphPair& pair) const {
  auto found = paths_.find(pair);
  if (found == paths_.end()) {
    return std::nullopt;
  }
  return found->second.vertices;
}

bool GlyphCacheContext::Prepare(
    const ContentContext& renderer,
    const std::shared_ptr<GlyphPathCache> path_cache) {
  if (!dirty_) {
    return true;
  }
  TRACE_EVENT0("impeller", __FUNCTION__);

  std::unordered_map<Font, CTFontRef, Font::Hash, Font::Equal> fonts;

  for (auto& frame : path_cache->GetTextFrames()) {
    for (const auto& run : frame.GetRuns()) {
      auto font = run.GetFont();
      for (const auto& glyph_position : run.GetGlyphPositions()) {
        FontGlyphPair pair = {font, glyph_position.glyph};
        auto found = paths_.find(pair);
        if (found == paths_.end()) {
          paths_[pair] = {.used = true};
        } else {
          found->second.used = true;
        }
        fonts[font] = nullptr;
      }
    }
  }

  for (auto& font_data : fonts) {
    CTFontRef font =
        static_cast<CTFontRef>(font_data.first.GetTypeface()->GetCTFont());
    CTFontRef sized_font = CTFontCreateWithFontDescriptor(
        CTFontCopyFontDescriptor(font), font_data.first.GetMetrics().point_size,
        nullptr);
    font_data.second = sized_font;
  }

  auto tessellator = renderer.GetTessellator();

  for (auto& data : paths_) {
    if (data.second.vertices.size() > 0) {
      continue;
    }

    auto sized_font = fonts.find(data.first.font);
    if (sized_font == fonts.end()) {
      continue;
    }
    CGAffineTransform transform = CGAffineTransformMakeScale(1, -1);
    CGPathRef cg_path = CTFontCreatePathForGlyph(
        sized_font->second, data.first.glyph.index, &transform);
    if (cg_path == nullptr) {
      continue;
    }
    auto path = ToPath(cg_path);
    CFRelease(cg_path);

    auto result = tessellator->Tessellate(
        path.GetFillType(), path.CreatePolyline(1.0),
        [buffer = &data.second.vertices](
            const float* vertices, size_t vertices_count,
            const uint16_t* indices, size_t indices_count) {
          std::vector<Point> points;
          for (auto i = 0u; i < vertices_count; i += 2) {
            points.emplace_back(vertices[i], vertices[i + 1]);
          }
          for (auto i = 0u; i < indices_count; i++) {
            auto idx = indices[i];
            buffer->push_back(points[idx]);
          }
          return true;
        });
    if (result != Tessellator::Result::kSuccess) {
      return false;
    }
  }
  dirty_ = false;
  return true;
}

void GlyphPathCache::AddTextFrame(const TextFrame& frame) {
  frames_.emplace_back(frame);
}

// if (font_name[0] == '.') {
//   // This must be a system font. CT will complain and return a times new
//   // roman font if we do not use this API.
//   font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, font_size,
//                                        nullptr);
// } else {
//   auto cf_font_name = CFStringCreateWithCString(
//       nullptr, data.first.font.GetPostscriptName().c_str(),
//       kCFStringEncodingUTF8);
//   font = CTFontCreateWithName(cf_font_name, font_size, nullptr);
// }

}  // namespace impeller
