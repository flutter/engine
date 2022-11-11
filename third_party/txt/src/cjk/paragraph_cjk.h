#ifndef FLUTTER_PARAGRAPH_CJK_H
#define FLUTTER_PARAGRAPH_CJK_H

#include "glyph_itemize.h"
#include "glyph_run.h"
#include "glyph_run_paint_record.h"
#include "txt/font_collection.h"
#include "txt/paint_record.h"
#include "txt/paragraph.h"
#include "txt/styled_runs.h"

#include <unordered_set>

namespace txt {

class ParagraphCJK : public Paragraph {
 public:
  ParagraphCJK(std::vector<uint16_t>&& text,
               const ParagraphStyle& style,
               StyledRuns&& runs,
               std::shared_ptr<FontCollection> font_collection,
               std::vector<PlaceholderRun>&& inline_placeholders,
               std::unordered_set<size_t>&& obj_replacement_char_indexes);

  virtual ~ParagraphCJK();

  void Layout(double width) override;
  void Paint(SkCanvas* canvas, double x, double y) override;

  double GetMaxWidth() override;
  double GetHeight() override;
  double GetLongestLine() override;
  double GetMinIntrinsicWidth() override;
  double GetMaxIntrinsicWidth() override;
  double GetAlphabeticBaseline() override;
  double GetIdeographicBaseline() override;
  bool DidExceedMaxLines() override;

  std::vector<TextBox> GetRectsForRange(
      size_t start,
      size_t end,
      RectHeightStyle rect_height_style,
      RectWidthStyle rect_width_style) override;
  std::vector<TextBox> GetRectsForPlaceholders() override;
  PositionWithAffinity GetGlyphPositionAtCoordinate(double dx,
                                                    double dy) override;
  Range<size_t> GetWordBoundary(size_t offset) override;
  std::vector<LineMetrics>& GetLineMetrics() override;

 private:
  // Strut metrics of zero will have no effect on the layout.
  struct StrutMetrics {
    double ascent = 0;  // Positive value to keep signs clear.
    double descent = 0;
    double leading = 0;
    double half_leading = 0;
    double line_height = 0;
    bool force_strut = false;
  };

  void ComputeStyledRuns();
  SkScalar ItemizeScriptRun(const ScriptRun& run, SkFont& sk_font);
  void ComputeTextLines(SkFont& sk_font);
  void UpdateLineMetrics(const SkFontMetrics& metrics,
                         const TextStyle& style,
                         double& max_ascent,
                         double& max_descent,
                         double& max_unscaled_ascent,
                         PlaceholderRun* placeholder_run,
                         size_t line_number,
                         size_t line_limit);
  void ComputePlaceholderMetrics(PlaceholderRun* placeholder_run,
                                 double& ascent,
                                 double& descent);

  void ComputeStrut(StrutMetrics* strut, SkFont& font);
  bool IsStrutValid();
  double GetStrutAscent();
  double GetStrutDescent();
  double GetLineXOffset(double line_total_advance, bool justify_line);

  // Get from builder
  std::vector<uint16_t> text_;
  StyledRuns styled_runs_;
  ParagraphStyle paragraph_style_;
  std::shared_ptr<FontCollection> font_collection_;

  // A vector of PlaceholderRuns, which detail the sizes, positioning and break
  // behavior of the empty spaces to leave. Each placeholder span corresponds to
  // a 0xFFFC (object replacement character) in text_, which indicates the
  // position in the text where the placeholder will occur. There should be an
  // equal number of 0xFFFC characters and elements in this vector.
  std::vector<PlaceholderRun> inline_placeholders_;
  // The indexes of instances of 0xFFFC that correspond to placeholders. This is
  // necessary since the user may pass in manually entered 0xFFFC values using
  // AddText().
  std::unordered_set<size_t> obj_replacement_char_indexes_;
  std::vector<size_t> placeholder_run_indexes_;

  std::vector<size_t> hard_break_positions_;
  std::vector<ScriptRun> script_runs_;

  std::vector<LineMetrics> line_metrics_;
  size_t final_line_count_ = 0;

  StrutMetrics strut_;
  bool did_exceed_max_lines_;
  double width_ = -1.0;
  double longest_line_ = -1.0;
  double max_intrinsic_width_ = 0;
  double min_intrinsic_width_ = 0;
  double alphabetic_baseline_ = std::numeric_limits<double>::max();
  double ideographic_baseline_ = std::numeric_limits<double>::max();

  bool is_first_layout_ = true;
  bool needs_layout_ = true;
  std::vector<std::unique_ptr<GlyphRun>> glyph_runs_;
  std::vector<GlyphRunPaintRecord> paint_records_;

  std::unique_ptr<icu::BreakIterator> word_breaker_;
};

}  // namespace txt

#endif  // FLUTTER_PARAGRAPH_CJK_H
