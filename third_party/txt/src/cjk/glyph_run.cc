#include "glyph_run.h"
#include "cjk_utils.h"

namespace txt {

GlyphRun::GlyphRun(const SkFont& sk_font,
                   const TextStyle& txt_style,
                   size_t start,
                   size_t end,
                   bool is_hard_break)
    : font(sk_font),
      style(txt_style),
      start(start),
      end(end),
      is_hard_break(is_hard_break),
      metrics(&style) {
  if (!is_hard_break) {
    sk_font.getMetrics(&metrics.font_metrics);
  }
}

void PlaceholderGlyphRun::get_break(size_t& from_position,
                                    double& width,
                                    const std::vector<uint16_t>& text,
                                    minikin::WordBreaker& breaker,
                                    bool is_head) const {
  from_position = 0;
  width = placeholder.width;
}

////////////////////////////////////////////////////////////////////////////////

MonoWidthGlyphRun::MonoWidthGlyphRun(const SkFont& font,
                                     const txt::TextStyle& style,
                                     size_t start,
                                     size_t end,
                                     SkScalar advance,
                                     std::unique_ptr<SkGlyphID[]> glyph_ids)
    : GlyphRun(font, style, start, end),
      advance_(advance),
      ids_(std::move(glyph_ids)) {
  size_t len = glyph_count();
  positions_ = std::make_unique<SkScalar[]>(len << 1);
  SkScalar* pos = positions_.get();
  SkScalar x_offset = 0;
  for (size_t i = 0; i < len; ++i) {
    pos[i << 1] = x_offset;
    x_offset += advance_;
  }
}

size_t MonoWidthGlyphRun::glyph_count() const {
  // No clusters in this run, one code unit corresponding to one glyph.
  return end - start;
}

double MonoWidthGlyphRun::max_word_width() const {
  return advance_;
}

double MonoWidthGlyphRun::get_rest_width(size_t from_position) const {
  return (glyph_count() - from_position) * advance_;
}

void MonoWidthGlyphRun::get_break(size_t& from_position,
                                  double& width,
                                  const std::vector<uint16_t>& text,
                                  minikin::WordBreaker& breaker,
                                  bool is_head) const {
  // TODO(nano): UAX #14
  size_t cnt = floor(width / advance_);
  from_position += cnt;
  width = cnt * advance_;
}

double MonoWidthGlyphRun::get_x_start(size_t position) const {
  return position * advance_;
}

double MonoWidthGlyphRun::get_x_end(size_t position) const {
  return (position + 1) * advance_;
}

sk_sp<SkTextBlob> make_sk_text_run(SkTextBlobBuilder& builder,
                                   size_t from_position,
                                   size_t count,
                                   const SkFont& font,
                                   SkGlyphID* ids,
                                   SkScalar* positions) {
  if (count <= 0) {
    return nullptr;
  }
  SkTextBlobBuilder::RunBuffer buffer;
  {
    CJK_MEASURE_TIME("alloc_run");
    buffer = builder.allocRunPos(font, count);
  }
  {
    CJK_MEASURE_TIME("copy_memory");
    memcpy(buffer.glyphs, &ids[from_position], count * sizeof(uint16_t));
    memcpy(buffer.pos, &positions[from_position << 1],
           count * sizeof(SkScalar) * 2);
  }
  CJK_MEASURE_TIME("make_blob");
  return builder.make();
}

sk_sp<SkTextBlob> MonoWidthGlyphRun::build_sk_text_run(
    SkTextBlobBuilder& builder,
    size_t from_position,
    size_t count) const {
  return make_sk_text_run(builder, from_position, count, font, ids_.get(),
                          positions_.get());
}

////////////////////////////////////////////////////////////////////////////////

ComplexGlyphRun::ComplexGlyphRun(const SkFont& font,
                                 const txt::TextStyle& style,
                                 size_t start,
                                 size_t end,
                                 size_t glyph_count,
                                 std::unique_ptr<SkGlyphID[]> glyph_ids,
                                 std::unique_ptr<SkScalar[]> positions,
                                 std::unique_ptr<SkScalar[]> advances,
                                 std::unique_ptr<size_t[]> code_unit_glyph_idx,
                                 bool is_space_standalone,
                                 bool is_space)
    : GlyphRun(font, style, start, end),
      is_space_(is_space),
      is_space_standalone_(is_space_standalone),
      glyph_count_(glyph_count),
      ids_(std::move(glyph_ids)),
      positions_(std::move(positions)),
      advances_(std::move(advances)),
      code_unit_glyph_idx_(std::move(code_unit_glyph_idx)) {
  const size_t end_idx = end - start - 1;
  total_advance_ = get_x_start(end_idx) + get_advance(end_idx);
}

double ComplexGlyphRun::get_advance(size_t position) const {
  const size_t cluster = code_unit_glyph_idx_[position];
  int i = (int)position - 1;
  while (i >= 0 && code_unit_glyph_idx_[i] == cluster) {
    --i;
  }
  return advances_[i + 1];
}

double ComplexGlyphRun::max_word_width() const {
  if (is_space_standalone_) {
    return total_advance_;
  }
  // TODO(nano)
  return 0;
}

double ComplexGlyphRun::get_rest_width(size_t from_position) const {
  return total_advance_ - get_x_start(from_position);
}

void ComplexGlyphRun::get_break(size_t& from_position,
                                double& width,
                                const std::vector<uint16_t>& text,
                                minikin::WordBreaker& breaker,
                                bool is_head) const {
  const double x_start = get_x_start(from_position);
  const size_t end_position = end - start;
  // Special case: the end of the run happens to be the end of the line
  if (total_advance_ - x_start <= width) {
    from_position = end_position;
    width = total_advance_ - x_start;
    return;
  }

  // If the spaces were handled separately, that shows us this run contains only
  // one word, in general, we just break at the current position, however, when
  // the cursor at the start of the line, it means the width of the word is
  // wider than the line, we still need to calculate how many glyphs in the run
  // to take out to lay out the current line.
  if (is_space_standalone_ && !is_head) {
    width = 0;
    return;
  }

  // A heuristic approach to guess an approximate position to break the text.
  //
  // Typically, we should use the average glyph width to calculate the position,
  // however, for those fonts have multilingual support (especially for CJK
  // ideographic scripts), it is much wider than what we expect.
  //
  // In most cases, the x-height can be treated as the pivot glyph size in the
  // font for those non-mono width scripts.
  const double pivot_width = abs(metrics.font_metrics.fXHeight);
  size_t approx_cnt = width / pivot_width + 1;
  size_t to_position = from_position + approx_cnt;
  if (to_position >= end_position) {
    to_position = end_position - 1;
  }

  // Determine where to break the run not take the Unicode break rules (UAX #14)
  // into account.
  //
  // Find the first position where the glyphs advance exceed the target width.
  double x_end = get_x_end(to_position);
  while (x_end - x_start < width) {
    size_t cnt = (width - (x_end - x_start)) / pivot_width + 1;
    to_position += cnt;
    if (to_position >= end_position) {
      to_position = end_position - 1;
      break;
    }
    x_end = get_x_end(to_position);
  }
  // Backward to the position where the glyphs advance just fit the target
  // width. It's OK with clusters, it will be skipped since the code units in
  // the same cluster have the same `x_end`.
  while (to_position > from_position && x_end - x_start > width) {
    x_end = get_x_end(--to_position);
  }

  if (!is_space_standalone_) {
    const size_t text_pos = start + to_position + 1;
    const ssize_t prev_text_pos = (ssize_t)(start + from_position) - 1;
    // When the cursor is not at the beginning of the line, and the preceding
    // codepoint of the `from_position` is a space and current character is not
    // a space (when it is, we need to consume the following spaces), it shows
    // us we are breaking at the middle of a word, that is illegal, just break
    // at the `from_position`.
    if (!is_head && prev_text_pos >= 0 &&
        is_space_separator(text[prev_text_pos]) && text_pos < end &&
        !is_space_separator(text[text_pos])) {
      width = 0;
      return;
    }
    // The code unit at `to_position + 1` is a space separator, consume the
    // succeeding spaces, since it will not participate in line-width
    // calculation.
    size_t i = text_pos;
    while (i < end && is_space_separator(text[i])) {
      ++i;
    }
    if (i != text_pos) {
      from_position = i - start;
      // Consume the preceding spaces, we should ignore its width when calculate
      // the line-width just like the succeeding spaces.
      ssize_t non_space_pos = look_back_spaces(text, text_pos - start);
      x_end = non_space_pos < 0 ? 0 : get_x_end(non_space_pos);
      width = x_end - x_start;
      return;
    }
  } else if (to_position <= from_position) {
    width = 0;
    return;
  }

  size_t start_boundary = start + from_position;
  size_t end_boundary = end - 1;
  if (!is_space_standalone_) {
    end_boundary = to_position + start + 1;
    while (end_boundary < end && !is_space_separator(text[end_boundary])) {
      ++end_boundary;
    }
    start_boundary = to_position + start;
    size_t n = start + from_position;
    while (start_boundary > n && !is_space_separator(text[start_boundary])) {
      --start_boundary;
    }
  }

  const uint16_t* p_text = text.data() + start_boundary;
  breaker.setText(p_text, end_boundary - start_boundary);

  size_t break_position = from_position;
  int32_t pos;
  while (1) {
    pos = breaker.next() + from_position;
    if (pos < 0 || pos > (int32_t)to_position) {
      break;
    }
    break_position = pos;
  }
  size_t p = (size_t)pos;
  if (p == to_position + 1 ||
      (p > to_position && break_position == from_position)) {
    // If `break_position` remains unchanged, that means there are no candidates
    // before the position we've 'guessed' before, so let it be the guessed
    // position.
    break_position = to_position + 1;
  }
  // Consume the preceding spaces, we should ignore its width when calculate
  // line-width.
  ssize_t non_space_pos = look_back_spaces(text, break_position);
  x_end = non_space_pos < 0 ? 0 : get_x_end(non_space_pos);
  from_position = break_position;
  width = x_end - x_start;

  breaker.finish();
}

ssize_t ComplexGlyphRun::look_back_spaces(const std::vector<uint16_t>& text,
                                          ssize_t position) const {
  if (is_space_standalone_) {
    return position - 1;
  }
  const ssize_t s_start = start;
  ssize_t i = s_start + position - 1;
  while (i >= s_start && is_space_separator(text[i])) {
    --i;
  }
  return i - s_start;
}

double ComplexGlyphRun::get_x_start(size_t position) const {
  return positions_[code_unit_glyph_idx_[position] << 1];
}

double ComplexGlyphRun::get_x_end(size_t position) const {
  return positions_[code_unit_glyph_idx_[position] << 1] +
         get_advance(position);
}

sk_sp<SkTextBlob> ComplexGlyphRun::build_sk_text_run(SkTextBlobBuilder& builder,
                                                     size_t from_position,
                                                     size_t count) const {
  if (count <= 0) {
    return nullptr;
  }
  size_t start_idx = code_unit_glyph_idx_[from_position];
  size_t end_idx = code_unit_glyph_idx_[from_position + count - 1];
  return make_sk_text_run(builder, start_idx, end_idx - start_idx + 1, font,
                          ids_.get(), positions_.get());
}

}  // namespace txt
