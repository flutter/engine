#include "paragraph_cjk.h"
#include <src/core/SkFontPriv.h>
#include <txt/font_skia.h>
#include "flutter/fml/logging.h"
#include "minikin/FontLanguageListCache.h"
#include "minikin/Layout.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkTextBlob.h"
#include "third_party/skia/src/core/SkStrikeCache.h"
#include "third_party/skia/src/core/SkStrikeSpec.h"

namespace txt {

namespace {

int GetWeight(const FontWeight weight) {
  switch (weight) {
    case FontWeight::w100:
      return 1;
    case FontWeight::w200:
      return 2;
    case FontWeight::w300:
      return 3;
    case FontWeight::w400:  // Normal.
      return 4;
    case FontWeight::w500:
      return 5;
    case FontWeight::w600:
      return 6;
    case FontWeight::w700:  // Bold.
      return 7;
    case FontWeight::w800:
      return 8;
    case FontWeight::w900:
      return 9;
    default:
      return -1;
  }
}

int GetWeight(const TextStyle& style) {
  return GetWeight(style.font_weight);
}

bool GetItalic(const TextStyle& style) {
  switch (style.font_style) {
    case FontStyle::italic:
      return true;
    case FontStyle::normal:
    default:
      return false;
  }
}

minikin::FontStyle GetMinikinFontStyle(const TextStyle& style) {
  uint32_t language_list_id =
      style.locale.empty()
          ? minikin::FontLanguageListCache::kEmptyListId
          : minikin::FontStyle::registerLanguageList(style.locale);
  return minikin::FontStyle(language_list_id, 0, GetWeight(style),
                            GetItalic(style));
}

void GetFontAndMinikinPaint(const TextStyle& style,
                            minikin::FontStyle* font,
                            minikin::MinikinPaint* paint) {
  *font = GetMinikinFontStyle(style);
  paint->size = style.font_size;
  // Divide by font size so letter spacing is pixels, not proportional to font
  // size.
  paint->letterSpacing = style.letter_spacing / style.font_size;
  paint->wordSpacing = style.word_spacing;
  paint->scaleX = 1.0f;
  // Prevent spacing rounding in Minikin. This causes jitter when switching
  // between same text content with different runs composing it, however, it
  // also produces more accurate layouts.
  paint->paintFlags |= minikin::LinearTextFlag;
  paint->fontFeatureSettings = style.font_features.GetFeatureSettings();
}

std::shared_ptr<minikin::FontCollection> GetMinikinFontCollectionForStyle(
    std::shared_ptr<FontCollection> font_collection,
    const TextStyle& style) {
  std::string locale;
  if (!style.locale.empty()) {
    uint32_t language_list_id =
        minikin::FontStyle::registerLanguageList(style.locale);
    const minikin::FontLanguages& langs =
        minikin::FontLanguageListCache::getById(language_list_id);
    if (langs.size()) {
      locale = langs[0].getString();
    }
  }

  return font_collection->GetMinikinFontCollectionForFamilies(
      style.font_families, locale);
}

template <typename T>
using sp = std::shared_ptr<T>;

struct font_style_hash_t {
  size_t operator()(const minikin::FontStyle& style) const {
    return style.hash();
  }
};

uint32_t kCJKTemp = 0x4E2D;

sp<minikin::FontFamily> GetFontFamily(
    std::shared_ptr<minikin::FontCollection> minikin_font_collection,
    const minikin::FontStyle& minikin_style) {
  static std::unordered_map<minikin::FontStyle, sp<minikin::FontFamily>,
                            font_style_hash_t>
      cjk_family_map;

  sp<minikin::FontFamily> font_family;
  const auto& family_it = cjk_family_map.find(minikin_style);
  if (family_it != cjk_family_map.end()) {
    font_family = family_it->second;
  } else {
    const uint32_t lang_list_id = minikin_style.getLanguageListId();
    const int variant = minikin_style.getVariant();
    const auto& family = minikin_font_collection->getFamilyForChar(
        kCJKTemp, 0, lang_list_id, variant);
    cjk_family_map.emplace(minikin_style, family);
    font_family = family;
  }
  return font_family;
}

}  // namespace

ParagraphCJK::ParagraphCJK(std::vector<uint16_t> text,
                           const txt::ParagraphStyle& style,
                           txt::StyledRuns runs,
                           std::shared_ptr<FontCollection> font_collection)
    : text_(std::move(text)),
      runs_(std::move(runs)),
      paragraph_style_(style),
      font_collection_(std::move(font_collection)) {}

ParagraphCJK::~ParagraphCJK() = default;

void ParagraphCJK::LayoutStyledRun(const txt::TextStyle& style) {
  minikin::FontStyle minikin_style;
  minikin::MinikinPaint minikin_paint;
  GetFontAndMinikinPaint(style, &minikin_style, &minikin_paint);
  auto minikin_font_collection =
      GetMinikinFontCollectionForStyle(font_collection_, style);

  auto font_family = GetFontFamily(minikin_font_collection, minikin_style);
  auto font = font_family->getClosestMatch(minikin_style);
  auto font_skia = static_cast<txt::FontSkia*>(font.font);
  auto sk_typeface = font_skia->GetSkTypeface();

  SkFont sk_font;
  sk_font.setEdging(SkFont::Edging::kAntiAlias);
  sk_font.setSubpixel(true);
  sk_font.setHinting(SkFontHinting::kSlight);

  sk_font.setTypeface(sk_typeface);
  sk_font.setSize(style.font_size);
  sk_font.setEmbolden(font.fakery.isFakeBold());
  sk_font.setSkewX(font.fakery.isFakeItalic() ? -SK_Scalar1 / 4 : 0);

  // Convert chars to glyphs
  SkAutoToGlyphs atg(sk_font, text_.data(), text_.size(),
                     SkTextEncoding::kUTF16);
  const int glyph_count = atg.count();
  if (glyph_count == 0) {
    return;
  }
  const SkGlyphID* glyph_ids = atg.glyphs();
  SkPaint sk_paint = style.foreground;
  auto [strike_spec, strike_to_source_scale] =
      SkStrikeSpec::MakeCanonicalized(sk_font, &sk_paint);
  SkBulkGlyphMetrics metrics{strike_spec};
  SkSpan<const SkGlyph*> glyphs =
      metrics.glyphs(SkSpan(glyph_ids, glyph_count));

  // Convert to TextBlob
  SkTextBlobBuilder sk_text_builder;
  const SkTextBlobBuilder::RunBuffer& blob_buffer =
      sk_text_builder.allocRunPos(sk_font, glyphs.size());
  for (size_t glyph_index = 0; glyph_index < glyphs.size(); glyph_index++) {
    blob_buffer.glyphs[glyph_index] = glyphs[glyph_index]->getGlyphID();
    size_t pos_index = glyph_index * 2;
    blob_buffer.pos[pos_index] = glyphs[glyph_index]->advanceX();
    blob_buffer.pos[pos_index + 1] = glyphs[glyph_index]->advanceY();
  }
  auto blob = sk_text_builder.make();
  text_blobs_.push_back(blob);
}

// We assume the text is all CJK unified ideographic
void ParagraphCJK::Layout(double width) {
  double rounded_width = floor(width);
  if (!needs_layout_ && rounded_width == width_) {
    return;
  }

  width_ = rounded_width;
  needs_layout_ = false;
  final_line_count_ = 0;

  line_metrics_.clear();
  line_widths_.clear();
  max_intrinsic_width_ = 0;

  // Compute line breaks
  // find and add all hard breaks
  std::vector<size_t> newline_positions;
  for (size_t i = 0; i < text_.size(); ++i) {
    auto ulb = static_cast<ULineBreak>(
        u_getIntPropertyValue(text_[i], UCHAR_LINE_BREAK));
    if (ulb == U_LB_LINE_FEED || ulb == U_LB_MANDATORY_BREAK) {
      newline_positions.emplace_back(i);
    }
  }
  newline_positions.emplace_back(text_.size());

  LayoutStyledRun(runs_.GetStyle(0));
  // minikin layout
  //  minikin::Layout layout;
  //  layout.doLayout(text_.data(), 0, text_.size(), text_.size(), false,
  //                  minikin_style, minikin_paint, minikin_font_collection);
}

void ParagraphCJK::Paint(SkCanvas* canvas, double x, double y) {
  SkPaint paint;
  for (const auto& blob : text_blobs_) {
    canvas->drawTextBlob(blob, x, y, paint);
  }
}

double ParagraphCJK::GetAlphabeticBaseline() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return alphabetic_baseline_;
}

double ParagraphCJK::GetIdeographicBaseline() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return ideographic_baseline_;
}

double ParagraphCJK::GetMaxIntrinsicWidth() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return max_intrinsic_width_;
}

double ParagraphCJK::GetMinIntrinsicWidth() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return min_intrinsic_width_;
}

double ParagraphCJK::GetHeight() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return final_line_count_ == 0 ? 0
                                : line_metrics_[final_line_count_ - 1].height;
}

double ParagraphCJK::GetMaxWidth() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return width_;
}

double ParagraphCJK::GetLongestLine() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return longest_line_;
}

bool ParagraphCJK::DidExceedMaxLines() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return did_exceed_max_lines_;
}

std::vector<LineMetrics>& ParagraphCJK::GetLineMetrics() {
  FML_DCHECK(!needs_layout_) << "only valid after layout";
  return line_metrics_;
}

std::vector<Paragraph::TextBox> ParagraphCJK::GetRectsForRange(
    size_t start,
    size_t end,
    txt::Paragraph::RectHeightStyle rect_height_style,
    txt::Paragraph::RectWidthStyle rect_width_style) {
  // TODO jkj
  return {};
}

std::vector<Paragraph::TextBox> ParagraphCJK::GetRectsForPlaceholders() {
  // TODO jkj
  return {};
}

Paragraph::PositionWithAffinity ParagraphCJK::GetGlyphPositionAtCoordinate(
    double dx,
    double dy) {
  // TODO jkj
  return {0, Affinity::DOWNSTREAM};
}

Paragraph::Range<size_t> ParagraphCJK::GetWordBoundary(size_t offset) {
  // TODO jkj
  return {};
}

}  // namespace txt
