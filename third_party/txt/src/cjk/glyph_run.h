#ifndef FLUTTER_TXT_CJK_GLYPH_RUN_H
#define FLUTTER_TXT_CJK_GLYPH_RUN_H

#include <include/core/SkFont.h>
#include <include/core/SkTextBlob.h>
#include <txt/placeholder_run.h>
#include "flutter/fml/macros.h"
#include "minikin/WordBreaker.h"
#include "script_run.h"
#include "txt/run_metrics.h"
#include "txt/text_style.h"

namespace txt {

// Class to represent a run of glyphs in a font.
//
// The various `position` parameter the methods of this class takes is the
// index in the text relative to the start of this run.
class GlyphRun {
 public:
  const SkFont font;
  const TextStyle& style;
  const size_t start;  // start index in text, inclusive
  const size_t end;    // end index in text, exclusive
  const bool is_hard_break;
  RunMetrics metrics;

  GlyphRun(const SkFont& sk_font,
           const TextStyle& txt_style,
           size_t start,
           size_t end,
           bool is_hard_break = false);

  virtual ~GlyphRun() = default;

  virtual inline bool is_placeholder() const { return false; }
  virtual inline bool is_space() const { return false; }

  virtual size_t glyph_count() const { return 0; }
  virtual const uint16_t* glyphs() const { return nullptr; }
  virtual const SkScalar* positions() const { return nullptr; }

  virtual double max_word_width() const { return 0; }

  // Get the total advance of the glyphs from [from_position] (inclusive) to the
  // end of the run.
  virtual double get_rest_width(size_t from_position) const { return 0; }

  // Get the break position from [from_position] which consumed width
  // must <= [width].
  //
  // The resulting break position will give back to [from_position], and the
  // consumed width will give back to [width].
  //
  // Note that the trailing spaces will be ignored when calculating the consumed
  // width. And in some cases, the consumed width could be < 0, for example,
  // consider the string 'hello word', at the previous iteration, the sub-string
  // 'hello ' has been added into the line, at this point, the break position
  // happens to be the position of the character 'w', it shows us that the
  // string 'hello ' was laying out at the end of the line, we need to subtract
  // the width of the trailing spaces, thus the consumed width would be < 0.
  //
  // You must call `breaker.finish()` after done.
  virtual void get_break(size_t& from_position,
                         double& width,
                         const std::vector<uint16_t>& text,
                         minikin::WordBreaker& breaker,
                         bool is_head) const {}

  // Get the start position of the glyph whose index is [position] in
  // x-direction.
  virtual double get_x_start(size_t position) const { return 0; }

  // Get the end position of the glyph whose index is [position] in x-direction.
  //
  // Note: in most scripts, the glyph's end position is NOT the start position
  // of its succeeding glyph, 'cause there's maybe an adjustment between them
  // (or namely 'kerning').
  //
  // For example, consider the string 'Va', in most non-mono-width fonts,
  // there's a negative space between glyph 'V' and 'a', that causes 'Va' looks
  // much tighter in visual.
  virtual double get_x_end(size_t position) const { return 0; }

  virtual sk_sp<SkTextBlob> build_sk_text_run(SkTextBlobBuilder& builder,
                                              size_t from_position,
                                              size_t count) const {
    return nullptr;
  }

  FML_DISALLOW_COPY_AND_ASSIGN(GlyphRun);
};

class PlaceholderGlyphRun : public GlyphRun {
 public:
  PlaceholderGlyphRun(const SkFont& font,
                      const TextStyle& style,
                      size_t start,
                      size_t end,
                      PlaceholderRun& placeholder)
      : GlyphRun(font, style, start, end), placeholder(placeholder) {}

  PlaceholderRun& placeholder;
  // Will be settled after the computation of text lines
  mutable double left = 0;
  mutable size_t line_num = 0;

  inline bool is_placeholder() const override { return true; }
  inline size_t glyph_count() const override { return 1; }
  inline double max_word_width() const override { return placeholder.width; }
  inline double get_rest_width(size_t) const override {
    return placeholder.width;
  }
  void get_break(size_t& from_position,
                 double& width,
                 const std::vector<uint16_t>& text,
                 minikin::WordBreaker& breaker,
                 bool is_head) const override;

  inline double get_x_start(size_t position) const override { return 0; }
  inline double get_x_end(size_t position) const override {
    return placeholder.width;
  }

  FML_DISALLOW_COPY_AND_ASSIGN(PlaceholderGlyphRun);
};

class MonoWidthGlyphRun : public GlyphRun {
 public:
  MonoWidthGlyphRun(const SkFont& font,
                    const TextStyle& style,
                    size_t start,
                    size_t end,
                    SkScalar advance,
                    std::unique_ptr<SkGlyphID[]> glyph_ids);

  inline const uint16_t* glyphs() const override { return ids_.get(); }
  inline const SkScalar* positions() const override { return positions_.get(); }

  size_t glyph_count() const override;
  double max_word_width() const override;
  double get_rest_width(size_t from_position) const override;
  void get_break(size_t& from_position,
                 double& width,
                 const std::vector<uint16_t>& text,
                 minikin::WordBreaker& breaker,
                 bool is_head) const override;

  double get_x_start(size_t position) const override;
  double get_x_end(size_t position) const override;
  sk_sp<SkTextBlob> build_sk_text_run(SkTextBlobBuilder& builder,
                                      size_t from_position,
                                      size_t count) const override;

 private:
  SkScalar advance_;
  std::unique_ptr<SkGlyphID[]> ids_;
  std::unique_ptr<SkScalar[]> positions_;

  FML_DISALLOW_COPY_AND_ASSIGN(MonoWidthGlyphRun);
};

class ComplexGlyphRun : public GlyphRun {
 public:
  ComplexGlyphRun(const SkFont& font,
                  const TextStyle& style,
                  size_t start,
                  size_t end,
                  size_t glyph_count,
                  std::unique_ptr<SkGlyphID[]> glyph_ids,
                  std::unique_ptr<SkScalar[]> positions,
                  std::unique_ptr<SkScalar[]> advances,
                  std::unique_ptr<size_t[]> code_unit_glyph_idx,
                  bool is_space_standalone,
                  bool is_space = false);

  inline bool is_space() const override { return is_space_; }
  inline const uint16_t* glyphs() const override { return ids_.get(); }
  inline const SkScalar* positions() const override { return positions_.get(); }
  inline size_t glyph_count() const override { return glyph_count_; }

  double max_word_width() const override;
  double get_rest_width(size_t from_position) const override;
  void get_break(size_t& from_position,
                 double& width,
                 const std::vector<uint16_t>& text,
                 minikin::WordBreaker& breaker,
                 bool is_head) const override;

  double get_x_start(size_t position) const override;
  double get_x_end(size_t position) const override;
  sk_sp<SkTextBlob> build_sk_text_run(SkTextBlobBuilder& builder,
                                      size_t from_position,
                                      size_t count) const override;

 private:
  bool is_space_;
  bool is_space_standalone_;
  size_t glyph_count_;
  double total_advance_;
  std::unique_ptr<SkGlyphID[]> ids_;
  std::unique_ptr<SkScalar[]> positions_;
  // Advances are organized by code units, if multiple code units form a single
  // glyph, the first advance of these code units would be the total width of
  // the resulting glyph, and the rests would be 0.
  std::unique_ptr<SkScalar[]> advances_;
  std::unique_ptr<size_t[]> code_unit_glyph_idx_;

  double get_advance(size_t position) const;

  ssize_t look_back_spaces(const std::vector<uint16_t>& text,
                           ssize_t position) const;

  FML_DISALLOW_COPY_AND_ASSIGN(ComplexGlyphRun);
};

}  // namespace txt

#endif  // FLUTTER_TXT_CJK_GLYPH_RUN_H
