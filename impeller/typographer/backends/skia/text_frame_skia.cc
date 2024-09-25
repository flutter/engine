// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/skia/text_frame_skia.h"

#include <vector>

#include "flutter/fml/logging.h"
#include "impeller/typographer/backends/skia/typeface_skia.h"
#include "impeller/typographer/font.h"
#include "impeller/typographer/glyph.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkFontMetrics.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/src/core/SkTextBlobPriv.h"  // nogncheck

namespace impeller {

static Font ToFont(const SkTextBlobRunIterator& run, AxisAlignment alignment) {
  auto& font = run.font();
  auto typeface = std::make_shared<TypefaceSkia>(font.refTypeface());

  SkFontMetrics sk_metrics;
  font.getMetrics(&sk_metrics);

  Font::Metrics metrics;
  metrics.point_size = font.getSize();
  metrics.embolden = font.isEmbolden();
  metrics.skewX = font.getSkewX();
  metrics.scaleX = font.getScaleX();

  return Font{std::move(typeface), metrics, alignment};
}

static Rect ToRect(const SkRect& rect) {
  return Rect::MakeLTRB(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}

std::shared_ptr<TextFrame> MakeTextFrameFromTextBlobSkia(
    const sk_sp<SkTextBlob>& blob,
    bool contains_color_glyphs) {
  Glyph::Type type =
      contains_color_glyphs ? Glyph::Type::kBitmap : Glyph::Type::kPath;
  std::vector<TextRun> runs;

  for (SkTextBlobRunIterator run(blob.get()); !run.done(); run.next()) {
    AxisAlignment alignment = AxisAlignment::kNone;
    if (run.font().isSubpixel()) {
      // If baseline snap or color glyphs, then no axis alignment
      if (run.font().isBaselineSnap() || contains_color_glyphs) {
        alignment = AxisAlignment::kNone;
      } else {
        alignment = AxisAlignment::kX;
      }
    }

    const auto glyph_count = run.glyphCount();
    const auto* glyphs = run.glyphs();
    switch (run.positioning()) {
      case SkTextBlobRunIterator::kFull_Positioning: {
        std::vector<TextRun::GlyphPosition> positions;
        positions.reserve(glyph_count);
        for (auto i = 0u; i < glyph_count; i++) {
          // kFull_Positioning has two scalars per glyph.
          const SkPoint* glyph_points = run.points();
          const SkPoint* point = glyph_points + i;
          positions.emplace_back(
              TextRun::GlyphPosition{Glyph{glyphs[i], type}, Point{
                                                                 point->x(),
                                                                 point->y(),
                                                             }});
        }
        TextRun text_run(ToFont(run, alignment), positions);
        runs.emplace_back(text_run);
        break;
      }
      default:
        FML_DLOG(ERROR) << "Unimplemented.";
        continue;
    }
  }
  return std::make_shared<TextFrame>(runs, ToRect(blob->bounds()),
                                     contains_color_glyphs);
}

}  // namespace impeller
