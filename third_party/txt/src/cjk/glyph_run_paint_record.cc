#include "glyph_run_paint_record.h"
#include <include/core/SkBlurTypes.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkPath.h>
#include <include/core/SkPathEffect.h>
#include <include/effects/SkDashPathEffect.h>
#include <include/effects/SkDiscretePathEffect.h>

namespace txt {

constexpr float kDoubleDecorationSpacing = 3.0f;

GlyphRunPaintRecord::GlyphRunPaintRecord(const txt::GlyphRun* glyph_run,
                                         size_t from_position,
                                         size_t count,
                                         SkPoint offset,
                                         size_t line,
                                         double x_start,
                                         double x_end,
                                         bool is_ghost,
                                         sk_sp<SkTextBlob> text)
    : PaintRecord(glyph_run->style,
                  offset,
                  text,
                  glyph_run->metrics.font_metrics,
                  line,
                  x_start,
                  x_end,
                  is_ghost),
      glyph_run_(glyph_run),
      from_position_(from_position),
      count_(count) {}

GlyphRunPaintRecord::GlyphRunPaintRecord(txt::GlyphRunPaintRecord&& other)
    : PaintRecord(std::move(other)),
      glyph_run_(other.glyph_run_),
      from_position_(other.from_position_),
      count_(other.count_) {}

void GlyphRunPaintRecord::paint_glyphs_with_shadow(SkCanvas* canvas,
                                                   const SkPoint& base_offset,
                                                   const SkPaint& paint) const {
  SkPoint offset = base_offset + this->offset();
  paint_shadow(canvas, offset);
  paint_glyphs(canvas, offset, paint);
}

void GlyphRunPaintRecord::paint_glyphs(SkCanvas* canvas,
                                       const SkPoint& offset,
                                       const SkPaint& paint) const {
  if (text() != nullptr) {
    canvas->drawTextBlob(text(), offset.x(), offset.y(), paint);
  } else {
    // Draw glyphs directly
    auto pos = reinterpret_cast<const SkPoint*>(glyph_run_->positions() +
                                                from_position_ * 2);
    canvas->drawGlyphs(count_, glyph_run_->glyphs() + from_position_, pos,
                       offset, glyph_run_->font, paint);
  }
}

void GlyphRunPaintRecord::paint_shadow(SkCanvas* canvas,
                                       const SkPoint& offset) const {
  if (style().text_shadows.empty()) {
    return;
  }
  for (auto& text_shadow : style().text_shadows) {
    if (!text_shadow.hasShadow()) {
      continue;
    }

    SkPaint paint;
    paint.setColor(text_shadow.color);
    if (text_shadow.blur_sigma > 0.5) {
      paint.setMaskFilter(SkMaskFilter::MakeBlur(
          kNormal_SkBlurStyle, text_shadow.blur_sigma, false));
    }
    paint_glyphs(canvas, offset + text_shadow.offset, paint);
  }
}

void GlyphRunPaintRecord::paint_background(SkCanvas* canvas,
                                           const SkPoint& base_offset) const {
  if (!style().has_background) {
    return;
  }
  const SkFontMetrics& metrics = this->metrics();
  SkRect rect =
      SkRect::MakeLTRB(x_start(), metrics.fAscent, x_end(), metrics.fDescent);
  rect.offset(base_offset + offset());
  canvas->drawRect(rect, style().background);
}

void compute_wavy_decoration(SkPath& path,
                             double x,
                             double y,
                             double width,
                             double thickness) {
  int wave_count = 0;
  double x_start = 0;
  // One full wavelength is 4 * thickness.
  double quarter = thickness;
  path.moveTo(x, y);
  double remaining = width;
  while (x_start + (quarter * 2) < width) {
    path.rQuadTo(quarter, wave_count % 2 == 0 ? -quarter : quarter, quarter * 2,
                 0);
    x_start += quarter * 2;
    remaining = width - x_start;
    ++wave_count;
  }

  double x1 = remaining / 2;
  double y1 = remaining / 2 * (wave_count % 2 == 0 ? -1 : 1);
  double x2 = remaining;
  double y2 = (remaining - remaining * remaining / (quarter * 2)) *
              (wave_count % 2 == 0 ? -1 : 1);
  path.rQuadTo(x1, y1, x2, y2);
}

void GlyphRunPaintRecord::paint_decorations(SkCanvas* canvas,
                                            const SkPoint& base_offset) const {
  if (style().decoration == TextDecoration::kNone)
    return;

  if (isGhost())
    return;

  const SkFontMetrics& metrics = this->metrics();
  SkPaint paint;
  paint.setStyle(SkPaint::kStroke_Style);
  if (style().decoration_color == SK_ColorTRANSPARENT) {
    paint.setColor(style().color);
  } else {
    paint.setColor(style().decoration_color);
  }
  paint.setAntiAlias(true);

  // This is set to 2 for the double line style
  int decoration_count = 1;

  // Filled when drawing wavy decorations.
  SkPath path;

  double width = GetRunWidth();

  SkScalar underline_thickness;
  if ((metrics.fFlags &
       SkFontMetrics::FontMetricsFlags::kUnderlineThicknessIsValid_Flag) &&
      metrics.fUnderlineThickness > 0) {
    underline_thickness = metrics.fUnderlineThickness;
  } else {
    // Backup value if the fUnderlineThickness metric is not available:
    // Divide by 14pt as it is the default size.
    underline_thickness = style().font_size / 14.0f;
  }
  paint.setStrokeWidth(underline_thickness *
                       style().decoration_thickness_multiplier);

  SkPoint record_offset = base_offset + offset();
  SkScalar x = record_offset.x() + x_start();
  SkScalar y = record_offset.y();

  // Setup the decorations.
  switch (style().decoration_style) {
    case TextDecorationStyle::kSolid: {
      break;
    }
    case TextDecorationStyle::kDouble: {
      decoration_count = 2;
      break;
    }
    // Note: the intervals are scaled by the thickness of the line, so it is
    // possible to change spacing by changing the decoration_thickness
    // property of TextStyle.
    case TextDecorationStyle::kDotted: {
      // Divide by 14pt as it is the default size.
      const float scale = style().font_size / 14.0f;
      const SkScalar intervals[] = {1.0f * scale, 1.5f * scale, 1.0f * scale,
                                    1.5f * scale};
      size_t count = sizeof(intervals) / sizeof(intervals[0]);
      paint.setPathEffect(SkPathEffect::MakeCompose(
          SkDashPathEffect::Make(intervals, count, 0.0f),
          SkDiscretePathEffect::Make(0, 0)));
      break;
    }
    // Note: the intervals are scaled by the thickness of the line, so it is
    // possible to change spacing by changing the decoration_thickness
    // property of TextStyle.
    case TextDecorationStyle::kDashed: {
      // Divide by 14pt as it is the default size.
      const float scale = style().font_size / 14.0f;
      const SkScalar intervals[] = {4.0f * scale, 2.0f * scale, 4.0f * scale,
                                    2.0f * scale};
      size_t count = sizeof(intervals) / sizeof(intervals[0]);
      paint.setPathEffect(SkPathEffect::MakeCompose(
          SkDashPathEffect::Make(intervals, count, 0.0f),
          SkDiscretePathEffect::Make(0, 0)));
      break;
    }
    case TextDecorationStyle::kWavy: {
      compute_wavy_decoration(
          path, x, y, width,
          underline_thickness * style().decoration_thickness_multiplier);
      break;
    }
  }

  // Draw the decorations.
  // Use a for loop for "kDouble" decoration style
  for (int i = 0; i < decoration_count; i++) {
    double y_offset = i * underline_thickness * kDoubleDecorationSpacing;
    double y_offset_original = y_offset;
    // Underline
    if (style().decoration & TextDecoration::kUnderline) {
      y_offset +=
          (metrics.fFlags &
           SkFontMetrics::FontMetricsFlags::kUnderlinePositionIsValid_Flag)
              ? metrics.fUnderlinePosition
              : underline_thickness;
      if (style().decoration_style != TextDecorationStyle::kWavy) {
        canvas->drawLine(x, y + y_offset, x + width, y + y_offset, paint);
      } else {
        SkPath offsetPath = path;
        offsetPath.offset(0, y_offset);
        canvas->drawPath(offsetPath, paint);
      }
      y_offset = y_offset_original;
    }
    // Overline
    if (style().decoration & TextDecoration::kOverline) {
      // We subtract fAscent here because for double overlines, we want the
      // second line to be above, not below the first.
      y_offset -= metrics.fAscent;
      if (style().decoration_style != TextDecorationStyle::kWavy) {
        canvas->drawLine(x, y - y_offset, x + width, y - y_offset, paint);
      } else {
        SkPath offsetPath = path;
        offsetPath.offset(0, -y_offset);
        canvas->drawPath(offsetPath, paint);
      }
      y_offset = y_offset_original;
    }
    // Strikethrough
    if (style().decoration & TextDecoration::kLineThrough) {
      if (metrics.fFlags &
          SkFontMetrics::FontMetricsFlags::kStrikeoutThicknessIsValid_Flag)
        paint.setStrokeWidth(metrics.fStrikeoutThickness *
                             style().decoration_thickness_multiplier);
      // Make sure the double line is "centered" vertically.
      y_offset += (decoration_count - 1.0) * underline_thickness *
                  kDoubleDecorationSpacing / -2.0;
      y_offset +=
          (metrics.fFlags &
           SkFontMetrics::FontMetricsFlags::kStrikeoutPositionIsValid_Flag)
              ? metrics.fStrikeoutPosition
              // Backup value if the strikeout position metric is not
              // available:
              : metrics.fXHeight / -2.0;
      if (style().decoration_style != TextDecorationStyle::kWavy) {
        canvas->drawLine(x, y + y_offset, x + width, y + y_offset, paint);
      } else {
        SkPath offsetPath = path;
        offsetPath.offset(0, y_offset);
        canvas->drawPath(offsetPath, paint);
      }
      y_offset = y_offset_original;
    }
  }
}

}  // namespace txt
