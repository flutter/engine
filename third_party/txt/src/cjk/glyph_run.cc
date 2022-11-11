#include "glyph_run.h"
#include "cjk_utils.h"

namespace txt {

GlyphRun::GlyphRun(const SkFont& sk_font,
                   const TextStyle& txt_style,
                   size_t start,
                   size_t end)
    : font(sk_font), style(txt_style), start(start), end(end), metrics(&style) {
  sk_font.getMetrics(&metrics.font_metrics);
}

void PlaceholderGlyphRun::get_break(size_t& from_position,
                                    double& width) const {
  from_position = 0;
  width = placeholder.width;
}

void PlaceholderGlyphRun::get_x_range(size_t from_position,
                                      size_t count,
                                      double& left,
                                      double& width) const {
  left = 0;
  width = placeholder.width;
}

sk_sp<SkTextBlob> PlaceholderGlyphRun::build_sk_text_run(SkTextBlobBuilder&,
                                                         size_t,
                                                         size_t,
                                                         double& left,
                                                         double& width) const {
  left = 0;
  width = placeholder.width;
  return nullptr;
}

IdeographicGlyphRun::IdeographicGlyphRun(const SkFont& font,
                                         const txt::TextStyle& style,
                                         size_t start,
                                         size_t end,
                                         SkScalar advance,
                                         std::unique_ptr<SkGlyphID[]> glyph_ids)
    : GlyphRun(font, style, start, end),
      ideographic_advance_(advance),
      glyph_ids_(std::move(glyph_ids)) {
  size_t len = glyph_count();
  glyph_positions_ = std::make_unique<SkScalar[]>(len << 1);
  SkScalar* pos = glyph_positions_.get();
  SkScalar x_offset = 0;
  for (size_t i = 0; i < len; ++i) {
    pos[i << 1] = x_offset;
    x_offset += ideographic_advance_;
  }
}

size_t IdeographicGlyphRun::glyph_count() const {
  // No clusters in this run, one code point corresponding to one glyph.
  return end - start;
}

double IdeographicGlyphRun::max_word_width() const {
  return ideographic_advance_;
}

double IdeographicGlyphRun::get_width(size_t from_position,
                                      size_t to_position) const {
  return (to_position - from_position) * ideographic_advance_;
}

void IdeographicGlyphRun::get_break(size_t& from_position,
                                    double& width) const {
  size_t cnt = floor(width / ideographic_advance_);
  from_position += cnt;
  width = cnt * ideographic_advance_;
}

double IdeographicGlyphRun::get_x_position(size_t i,
                                           size_t from_position) const {
  return (i - from_position) * ideographic_advance_;
}

double IdeographicGlyphRun::get_y_position(size_t i) const {
  return 0;
}

void IdeographicGlyphRun::get_x_range(size_t from_position,
                                      size_t count,
                                      double& left,
                                      double& width) const {
  left = from_position * ideographic_advance_;
  width = count * ideographic_advance_;
}

sk_sp<SkTextBlob> IdeographicGlyphRun::build_sk_text_run(
    SkTextBlobBuilder& builder,
    size_t from_position,
    size_t count,
    double& left,
    double& width) const {
  SkTextBlobBuilder::RunBuffer buffer;
  {
    CJK_MEASURE_TIME("alloc_run");
    buffer = builder.allocRunPos(font, count);
  }

  constexpr size_t u16_size = sizeof(uint16_t);
  {
    CJK_MEASURE_TIME("copy_memory");
    memcpy(
        buffer.glyphs,
        reinterpret_cast<uint8_t*>(glyph_ids_.get()) + from_position * u16_size,
        count * 2);
    constexpr size_t pos_size = sizeof(SkScalar) * 2;
    void* src = reinterpret_cast<uint8_t*>(glyph_positions_.get()) +
                from_position * pos_size;
    memcpy(buffer.pos, src, count * pos_size);
  }

  left = from_position * ideographic_advance_;
  width = count * ideographic_advance_;

  CJK_MEASURE_TIME("make_blob");
  return builder.make();
}

}  // namespace txt
