#ifndef FLUTTER_PARAGRAPH_CJK_H
#define FLUTTER_PARAGRAPH_CJK_H

#include "txt/font_collection.h"
#include "txt/paint_record.h"
#include "txt/paragraph.h"
#include "txt/styled_runs.h"

namespace txt {

class ParagraphCJK : public Paragraph {
 public:
  // TODO jkj inline placeholders
  ParagraphCJK(std::vector<uint16_t> text,
               const ParagraphStyle& style,
               StyledRuns runs,
               std::shared_ptr<FontCollection> font_collection);

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
  void LayoutStyledRun(const TextStyle& style);

  std::vector<uint16_t> text_;
  StyledRuns runs_;
  ParagraphStyle paragraph_style_;
  std::shared_ptr<FontCollection> font_collection_;

  std::vector<LineMetrics> line_metrics_;
  std::vector<double> line_widths_;
  size_t final_line_count_ = 0;

  bool did_exceed_max_lines_;
  double width_ = -1.0;
  double longest_line_ = -1.0;
  double max_intrinsic_width_ = 0;
  double min_intrinsic_width_ = 0;
  double alphabetic_baseline_ = std::numeric_limits<double>::max();
  double ideographic_baseline_ = std::numeric_limits<double>::max();

  bool needs_layout_ = true;

  std::vector<PaintRecord> paint_records_;
  std::vector<sk_sp<SkTextBlob>> text_blobs_;
};

}  // namespace txt

#endif  // FLUTTER_PARAGRAPH_CJK_H
