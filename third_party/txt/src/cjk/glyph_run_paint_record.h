#ifndef FLUTTER_TXT_GLYPH_RUN_PAINT_RECORD_H
#define FLUTTER_TXT_GLYPH_RUN_PAINT_RECORD_H

#include <include/core/SkCanvas.h>
#include "glyph_run.h"
#include "txt/paint_record.h"

namespace txt {

class GlyphRunPaintRecord : public PaintRecord {
 public:
  ~GlyphRunPaintRecord() = default;

  GlyphRunPaintRecord(const GlyphRun* glyph_run,
                      size_t from_position,
                      size_t count,
                      SkPoint offset,
                      size_t line,
                      double x_start,
                      double x_end,
                      bool is_ghost,
                      sk_sp<SkTextBlob> text = nullptr);

  GlyphRunPaintRecord(GlyphRunPaintRecord&& other);

  void paint_glyphs_with_shadow(SkCanvas* canvas,
                                const SkPoint& base_offset,
                                const SkPaint& paint) const;
  void paint_glyphs(SkCanvas* canvas,
                    const SkPoint& offset,
                    const SkPaint& paint) const;
  void paint_shadow(SkCanvas* canvas, const SkPoint& offset) const;
  void paint_background(SkCanvas* canvas, const SkPoint& base_offset) const;
  void paint_decorations(SkCanvas* canvas, const SkPoint& offset) const;

 private:
  const GlyphRun* glyph_run_;
  const size_t from_position_;
  const size_t count_;

  FML_DISALLOW_COPY_AND_ASSIGN(GlyphRunPaintRecord);
};

}  // namespace txt

#endif  // FLUTTER_TXT_GLYPH_RUN_PAINT_RECORD_H
