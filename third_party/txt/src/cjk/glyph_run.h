#ifndef FLUTTER_TXT_CJK_GLYPH_RUN_H
#define FLUTTER_TXT_CJK_GLYPH_RUN_H

#include <include/core/SkFont.h>
#include <include/core/SkTextBlob.h>
#include <txt/placeholder_run.h>
#include "flutter/fml/macros.h"
#include "txt/run_metrics.h"
#include "txt/text_style.h"

namespace txt {

class GlyphRun {
 public:
  const SkFont font;
  const TextStyle& style;
  const size_t start;  // start index in text, inclusive
  const size_t end;    // end index in text, exclusive
  RunMetrics metrics;

  GlyphRun(const SkFont& sk_font,
           const TextStyle& txt_style,
           size_t start,
           size_t end);

  virtual ~GlyphRun() = default;

  inline size_t len() const { return end - start; }
  virtual inline bool is_placeholder() const { return false; }

  virtual const uint16_t* glyphs() const = 0;
  virtual const SkScalar* positions() const = 0;

  virtual size_t glyph_count() const = 0;
  virtual double max_word_width() const = 0;
  virtual double get_width(size_t from_position, size_t to_position) const = 0;

  // Get the break position from [from_position] which consumed width
  // must <= [width].
  //
  // The resulting break position will give back to [from_position], and the
  // consumed width will give back to [width].
  virtual void get_break(size_t& from_position, double& width) const = 0;

  // Get the x position of the glyph whose index is [i] relative to the glyph
  // whose index is [from_position].
  //
  // For example, consider the string: 'hello' whose glyph advances are [0, 10,
  // 20, 38, 56, 66], the result of the call `get_x_position(3, 1)` is 28, and
  // the result of the call `get_x_position(5, 2)` is 46.
  virtual double get_x_position(size_t i, size_t from_position) const = 0;
  virtual double get_y_position(size_t i) const = 0;

  virtual void get_x_range(size_t from_position,
                           size_t count,
                           double& left,
                           double& width) const = 0;

  virtual sk_sp<SkTextBlob> build_sk_text_run(SkTextBlobBuilder& builder,
                                              size_t from_position,
                                              size_t count,
                                              double& left,
                                              double& width) const = 0;
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
  inline const uint16_t* glyphs() const override { return nullptr; }
  inline const SkScalar* positions() const override { return nullptr; }
  inline size_t glyph_count() const override { return 1; }
  inline double max_word_width() const override { return placeholder.width; }
  inline double get_width(size_t, size_t) const override {
    return placeholder.width;
  }
  void get_break(size_t& from_position, double& width) const override;
  inline double get_x_position(size_t, size_t) const override { return 0; }
  inline double get_y_position(size_t) const override { return 0; }

  void get_x_range(size_t from_position,
                   size_t count,
                   double& left,
                   double& width) const override;
  sk_sp<SkTextBlob> build_sk_text_run(SkTextBlobBuilder&,
                                      size_t,
                                      size_t,
                                      double& left,
                                      double& width) const override;

  FML_DISALLOW_COPY_AND_ASSIGN(PlaceholderGlyphRun);
};

class IdeographicGlyphRun : public GlyphRun {
 public:
  IdeographicGlyphRun(const SkFont& font,
                      const TextStyle& style,
                      size_t start,
                      size_t end,
                      SkScalar advance,
                      std::unique_ptr<SkGlyphID[]> glyph_ids);

  inline const uint16_t* glyphs() const override { return glyph_ids_.get(); }
  inline const SkScalar* positions() const override {
    return glyph_positions_.get();
  }

  size_t glyph_count() const override;
  double max_word_width() const override;
  double get_width(size_t from_position, size_t to_position) const override;
  void get_break(size_t& from_position, double& width) const override;
  double get_x_position(size_t i, size_t from_position) const override;
  double get_y_position(size_t i) const override;

  void get_x_range(size_t from_position,
                   size_t count,
                   double& left,
                   double& width) const override;
  sk_sp<SkTextBlob> build_sk_text_run(SkTextBlobBuilder& builder,
                                      size_t from_position,
                                      size_t count,
                                      double& left,
                                      double& width) const override;

 private:
  SkScalar ideographic_advance_;
  std::unique_ptr<SkGlyphID[]> glyph_ids_;
  std::unique_ptr<SkScalar[]> glyph_positions_;

  FML_DISALLOW_COPY_AND_ASSIGN(IdeographicGlyphRun);
};

}  // namespace txt

#endif  // FLUTTER_TXT_CJK_GLYPH_RUN_H
