#include "paragraph_cjk.h"
#include <include/effects/SkDashPathEffect.h>
#include <src/core/SkFontPriv.h>
#include <src/core/SkStrikeCache.h>
#include <txt/font_skia.h>
#include "glyph_itemize.h"
#include "minikin/FontLanguageListCache.h"
#include "otf_cmap.h"

namespace txt {

namespace {

constexpr int kObjReplacementChar = 0xFFFC;

int get_weight(const FontWeight weight) {
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

bool is_italic(const TextStyle& style) {
  switch (style.font_style) {
    case FontStyle::italic:
      return true;
    case FontStyle::normal:
    default:
      return false;
  }
}

minikin::FontStyle get_minikin_font_style(const TextStyle& style) {
  uint32_t language_list_id =
      style.locale.empty()
          ? minikin::FontLanguageListCache::kEmptyListId
          : minikin::FontStyle::registerLanguageList(style.locale);
  return minikin::FontStyle(language_list_id, 0, get_weight(style.font_weight),
                            is_italic(style));
}

void get_minikin_font_paint(const TextStyle& style,
                            minikin::FontStyle* font,
                            minikin::MinikinPaint* paint) {
  *font = get_minikin_font_style(style);
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

std::shared_ptr<minikin::FontCollection> get_minikin_font_collection_for_style(
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

sk_sp<SkTypeface> get_default_skia_typeface(
    std::shared_ptr<FontCollection> font_collection,
    const TextStyle& style) {
  auto collection =
      get_minikin_font_collection_for_style(font_collection, style);
  if (!collection) {
    return nullptr;
  }
  auto faked_font = collection->baseFontFaked(get_minikin_font_style(style));
  return static_cast<FontSkia*>(faked_font.font)->GetSkTypeface();
}

template <typename T>
using sp = std::shared_ptr<T>;

struct font_style_hash_t {
  size_t operator()(const minikin::FontStyle& style) const {
    return style.hash();
  }
};

constexpr uint32_t kCJKAnchor = 0x4E2D;

sp<minikin::FontFamily> get_minikin_font_family(
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
        kCJKAnchor, 0, lang_list_id, variant);
    cjk_family_map.emplace(minikin_style, family);
    font_family = family;
  }
  return font_family;
}

#if 0
sk_sp<SkTextBlob> make_canonicalized_blob(const SkFont& sk_font,
                                          uint16_t* text,
                                          size_t count) {
  SkAutoToGlyphs atg(sk_font, text, count * sizeof(uint16_t),
                     SkTextEncoding::kUTF16);
  const int glyph_count = atg.count();
  if (glyph_count == 0) {
    return nullptr;
  }
  const SkGlyphID* glyph_ids = atg.glyphs();
  SkPaint sk_paint;
  auto [strike_spec, strike_to_source_scale] =
      SkStrikeSpec::MakeCanonicalized(sk_font, &sk_paint);
  SkBulkGlyphMetrics metrics{strike_spec};
  SkSpan<const SkGlyph*> glyphs =
      metrics.glyphs(SkSpan(glyph_ids, glyph_count));

  // Convert to TextBlob
  SkTextBlobBuilder sk_text_builder;
  const SkTextBlobBuilder::RunBuffer& blob_buffer =
      sk_text_builder.allocRunPos(sk_font, glyphs.size());
  int x_pos = 0;
  for (size_t glyph_index = 0; glyph_index < glyphs.size(); glyph_index++) {
    blob_buffer.glyphs[glyph_index] = glyphs[glyph_index]->getGlyphID();
    size_t pos_index = glyph_index * 2;
    blob_buffer.pos[pos_index] = x_pos;
    blob_buffer.pos[pos_index + 1] = glyphs[glyph_index]->advanceY();
    x_pos += glyphs[glyph_index]->advanceX();
  }
  return sk_text_builder.make();
}
#endif

}  // namespace

constexpr int kItemizeSkFont = 1001;
constexpr int kItemizeHb = 1002;
constexpr int kItemizeHbFont = 1003;
constexpr int kItemizeCmap = 1004;

ParagraphCJK::ParagraphCJK(
    std::vector<uint16_t>&& text,
    const txt::ParagraphStyle& style,
    txt::StyledRuns&& runs,
    std::shared_ptr<FontCollection> font_collection,
    std::vector<PlaceholderRun>&& inline_placeholders,
    std::unordered_set<size_t>&& obj_replacement_char_indexes)
    : text_(std::move(text)),
      styled_runs_(std::move(runs)),
      paragraph_style_(style),
      font_collection_(std::move(font_collection)),
      inline_placeholders_(std::move(inline_placeholders)),
      obj_replacement_char_indexes_(std::move(obj_replacement_char_indexes)) {}

ParagraphCJK::~ParagraphCJK() = default;

inline bool is_cjk_ideographic(uint16_t codepoint) {
  return is_cjk_ideographic_bmp(codepoint) || is_fullwidth(codepoint);
}

void ParagraphCJK::ComputeStyledRuns() {
  const size_t style_len = styled_runs_.size();
  for (size_t style_idx = 0; style_idx < style_len; ++style_idx) {
    const auto& run = styled_runs_.GetRun(style_idx);
    for (size_t i = run.start; i < run.end;) {
      size_t k = i + 1;
      if (is_cjk_ideographic(text_[i])) {
        while (k < run.end && is_cjk_ideographic(text_[k])) {
          ++k;
        }
        script_runs_.push_back({run.style, i, k, ScriptRunType::kCJKIdeograph});
      } else if (is_hard_break(text_[i])) {
        while (k < run.end && !is_cjk_ideographic(text_[k])) {
          ++k;
        }
        script_runs_.push_back({run.style, i, k, ScriptRunType::kHardbreak});
      } else {
        while (k < run.end && !is_cjk_ideographic(text_[k]) &&
               !is_hard_break(text_[k])) {
          ++k;
        }
        script_runs_.push_back({run.style, i, k, ScriptRunType::kComplex});
      }
      i = k;
    }
  }
}

SkScalar ParagraphCJK::ItemizeScriptRun(const txt::ScriptRun& run,
                                        SkFont& sk_font) {
  const txt::TextStyle& style = run.style;
  minikin::FontStyle minikin_style;
  minikin::MinikinPaint minikin_paint;
  get_minikin_font_paint(style, &minikin_style, &minikin_paint);
  auto minikin_font_collection =
      get_minikin_font_collection_for_style(font_collection_, style);

  auto font_family =
      get_minikin_font_family(minikin_font_collection, minikin_style);
  auto font = font_family->getClosestMatch(minikin_style);
  auto minikin_font = static_cast<txt::FontSkia*>(font.font);
  auto sk_typeface = minikin_font->GetSkTypeface();

  sk_font.setTypeface(sk_typeface);
  sk_font.setSize(style.font_size);
  sk_font.setEmbolden(font.fakery.isFakeBold());
  sk_font.setSkewX(font.fakery.isFakeItalic() ? -SK_Scalar1 / 4 : 0);

  int method = kItemizeCmap;
  ////////////////////////////////////
  method = paragraph_style_.max_lines;
  ////////////////////////////////////
  auto save_glyph_run = [runs = &glyph_runs_](std::unique_ptr<GlyphRun>&& run) {
    double width = run->get_width(0, run->len());
    runs->push_back(std::move(run));
    return width;
  };
  auto action = make_action(text_, run, minikin_font, sk_font);
  auto get_glyph_run = action | save_glyph_run;
  switch (method) {
    case kItemizeSkFont:
      return get_glyph_run(get_glyph_run_skfont);
    case kItemizeHb:
      return get_glyph_run(get_glyph_run_hb);
    case kItemizeHbFont:
      return get_glyph_run(get_glyph_run_hb_font);
    case kItemizeCmap:
      return get_glyph_run(get_glyph_run_cmap);
  }

  return 0;
}

void ParagraphCJK::ComputeTextLines(SkFont& sk_font) {
  if (width_ == max_intrinsic_width_) {
    return;
  }
  // Consider the string "Hello World":
  //
  // The maximum intrinsic width would be the width of the string without line
  // breaks.
  //
  // The minimum intrinsic width would be the width of the widest word, "Hello"
  // or "World". If the text is rendered in an even narrower width, however, it
  // might still not overflow. For example, maybe the rendering would put a
  // line-break half-way through the words, as in "Hel⁞lo⁞Wor⁞ld".
  //
  // For CJK ideographic writing system, the minimum intrinsic width would be
  // the width of a single glyph.
  max_intrinsic_width_ = 0;
  min_intrinsic_width_ = 0;

  line_metrics_.clear();
  final_line_count_ = 0;

  const size_t glyph_run_len = glyph_runs_.size();
  size_t line_limit = paragraph_style_.max_lines;

  size_t hard_break_idx = 0;
  size_t hard_break_position = hard_break_positions_.empty()
                                   ? std::numeric_limits<size_t>::max()
                                   : hard_break_positions_[hard_break_idx];
  size_t run_break_position = 0;
  size_t run_idx = 0;

  double cumulative_width = 0;
  double intrinsic_width = 0;

  double max_ascent = GetStrutAscent();
  double max_descent = GetStrutDescent();
  double max_unscaled_ascent = 0;

  double prev_max_descent = 0;
  double y_offset = 0;
  size_t paint_record_idx = 0;

  line_metrics_.emplace_back(0, 0, 0, 0, false);
#ifndef CJK_DRAW_DIRECT
  SkTextBlobBuilder blob_builder;
#endif
  // Add the given [run] to current line, calculate its metrics, make a draw
  // from the glyph ids and positions.
  auto add_glyph_run = [&max_ascent, &max_descent, &max_unscaled_ascent,
                        &line_limit, &cumulative_width, this
#ifndef CJK_DRAW_DIRECT
                        ,
                        &blob_builder
#endif
  ](const GlyphRun& run, size_t text_start, size_t text_end) {
    const size_t line_num = line_metrics_.size() - 1;
    auto& line_metric = line_metrics_.back();
    // Store the font metrics and TextStyle in the LineMetrics for this line
    // to provide metrics upon user request. We index this RunMetrics instance
    // at `text_end - 1` to allow map::lower_bound to access the correct
    // RunMetrics at any text index in this run.
    const size_t run_key = text_end - 1;
    line_metric.run_metrics.emplace(run_key, run.metrics);

    const bool is_placeholder = run.is_placeholder();
    PlaceholderRun* placeholder_run =
        is_placeholder
            ? &(static_cast<const PlaceholderGlyphRun&>(run).placeholder)
            : nullptr;
    UpdateLineMetrics(run.metrics.font_metrics, run.style, max_ascent,
                      max_descent, max_unscaled_ascent, placeholder_run,
                      line_num, line_limit);
    if (is_placeholder) {
      const auto& pgr = static_cast<const PlaceholderGlyphRun&>(run);
      pgr.left = cumulative_width;
      pgr.line_num = line_num;
      return;
    }

    double x_start = 0;
    double width = 0;
#ifndef CJK_DRAW_DIRECT
    sk_sp<SkTextBlob> blob;
    {
      CJK_MEASURE_TIME("create_text_blob");
      blob = run.build_sk_text_run(blob_builder, text_start - run.start,
                                   text_end - text_start, x_start, width);
    }
#else
    run.get_x_range(text_start - run.start, text_end - text_start, x_start,
                    width);
#endif
    paint_records_.emplace_back(
        &run, text_start - run.start, text_end - text_start,
        SkPoint::Make(cumulative_width - x_start, 0), line_num, x_start,
        x_start + width, /* TODO(nano) is ghost run? */ false
#ifndef CJK_DRAW_DIRECT
        ,
        blob
#endif
    );
  };

  SkFontMetrics tmp_metrics;
  TextStyle paragraph_text_style = paragraph_style_.GetTextStyle();
  // Finish the current text line when meet a break or reach the end of the
  // text.
  auto finish_text_line = [&max_ascent, &max_descent, &max_unscaled_ascent,
                           &line_limit, &add_glyph_run, &sk_font, &tmp_metrics,
                           &paragraph_text_style, &paint_record_idx, &y_offset,
                           &prev_max_descent,
                           this](bool is_hard_break, size_t text_start,
                                 size_t break_position, double line_width,
                                 const GlyphRun& run) {
    const size_t line_num = line_metrics_.size() - 1;
    const bool is_last_line = break_position >= text_.size();
    if (is_last_line) {
      line_limit = line_num + 1;
    }

    // Applying ellipsis if the run was not completely laid out and this is the
    // last line (or lines are unlimited).
    const std::u16string& ellipsis = paragraph_style_.ellipsis;
    if (ellipsis.length() && !isinf(width_) && !is_hard_break &&
        (line_num == line_limit - 1 || paragraph_style_.unlimited_lines())) {
      // TODO(nano)
    }

    auto& line_metrics = line_metrics_.back();

    size_t line_end_excluding_whitespace = break_position;
    while (line_end_excluding_whitespace > line_metrics.start_index &&
           minikin::isLineEndSpace(text_[line_end_excluding_whitespace - 1])) {
      --line_end_excluding_whitespace;
    }
    size_t next_offset = is_hard_break && break_position < text_.size() ? 1 : 0;

    line_metrics.line_number = line_num;
    line_metrics.hard_break = is_hard_break;
    line_metrics.end_index = break_position;
    line_metrics.end_excluding_whitespace = line_end_excluding_whitespace;
    line_metrics.end_including_newline = break_position + next_offset;

    if (line_metrics.end_index == line_metrics.start_index) {
      // An empty line (no other characters in this line except the hard-break
      // characters), there's no glyphs were actually rendered, then compute a
      // baseline based on the font of the paragraph style.
      sk_font.setTypeface(
          get_default_skia_typeface(font_collection_, paragraph_text_style));
      sk_font.setSize(paragraph_text_style.font_size);
      sk_font.getMetrics(&tmp_metrics);
      UpdateLineMetrics(tmp_metrics, paragraph_text_style, max_ascent,
                        max_descent, max_unscaled_ascent, nullptr, line_num,
                        line_limit);
    } else {
      add_glyph_run(run, text_start, break_position);
    }

    line_metrics.ascent = max_ascent;
    line_metrics.descent = max_descent;
    line_metrics.unscaled_ascent = max_unscaled_ascent;

    line_metrics.width = line_width;

    // Calculate the baselines. This is only done on the first line.
    if (line_num == 0) {
      alphabetic_baseline_ = max_ascent;
      // FIXME(nano): Ideographic baseline is currently bottom of EM box, which
      // is not correct. This should be obtained from metrics. Skia currently
      // does not support various baselines.
      ideographic_baseline_ = (max_ascent + max_descent);
    }

    line_metrics.height =
        (line_num == 0 ? 0 : line_metrics_[line_num - 1].height) +
        round(max_ascent + max_descent);
    line_metrics.baseline = line_metrics.height - max_descent;

    bool justify_line = paragraph_style_.text_align == TextAlign::justify &&
                        line_num != line_limit - 1 && !line_metrics.hard_break;
    const double line_x_offset = GetLineXOffset(line_width, justify_line);
    line_metrics.left = GetLineXOffset(line_width, justify_line);

    // Shift the paint records of this line
    y_offset += round(max_ascent + prev_max_descent);
    prev_max_descent = max_descent;
    for (size_t i = paint_record_idx; i < paint_records_.size(); ++i) {
      PaintRecord& record = paint_records_[i];
      record.SetOffset(
          SkPoint::Make(record.offset().x() + line_x_offset, y_offset));
    }
    paint_record_idx = paint_records_.size();

    ++final_line_count_;

    // Prepare next text line
    if (!is_last_line) {
      line_metrics_.emplace_back(break_position + next_offset, 0, 0, 0, false);
    }

    // Reset the line metrics states for further calculation
    max_ascent = GetStrutAscent();
    max_descent = GetStrutDescent();
    max_unscaled_ascent = 0;
  };

  auto add_hard_break_in_run = [&run_break_position, &hard_break_position,
                                &hard_break_idx, &run_idx, &intrinsic_width,
                                &cumulative_width, &finish_text_line,
                                this](const GlyphRun& run,
                                      size_t prev_break_position) {
    const size_t next_break_position = hard_break_position - run.start;
    double rest_width = run.get_width(prev_break_position, next_break_position);
    intrinsic_width += rest_width;

    finish_text_line(true, prev_break_position + run.start, hard_break_position,
                     cumulative_width + rest_width, run);

    ++hard_break_idx;
    hard_break_position = hard_break_idx >= hard_break_positions_.size()
                              ? std::numeric_limits<size_t>::max()
                              : hard_break_positions_[hard_break_idx];
    max_intrinsic_width_ = std::max(max_intrinsic_width_, intrinsic_width);

    // Exclude the hard-break
    run_break_position = next_break_position + 1;
    cumulative_width = 0;
    intrinsic_width = 0;

    // That means the end of the current run is a hard-break, switch to next
    // glyph run.
    if (run_break_position + run.start >= run.end) {
      ++run_idx;
      run_break_position = 0;
    }
  };

  while (run_idx < glyph_run_len && final_line_count_ < line_limit) {
    const auto& run = *(glyph_runs_[run_idx]);
    min_intrinsic_width_ =
        std::max<double>(min_intrinsic_width_, run.max_word_width());

    const size_t prev_break_position = run_break_position;
    const size_t text_start = prev_break_position + run.start;
    const double rest_width = run.get_width(run_break_position, run.len());

    if (cumulative_width + rest_width >= width_) {
      double width = width_ - cumulative_width;
      // The `run_break_position` will be changed to the next break position,
      // and width will be changed to the actually consumed width (<= width).
      run.get_break(run_break_position, width);

      // Check if we meet the hard break in current run
      if (run_break_position + run.start >= hard_break_position) {
        add_hard_break_in_run(run, prev_break_position);
      } else {
        // Soft-break
        finish_text_line(false, text_start, run_break_position + run.start,
                         cumulative_width + width, run);
        intrinsic_width += width;
        cumulative_width = 0;
      }
    } else if (run.end > hard_break_position || run.end >= text_.size()) {
      add_hard_break_in_run(run, prev_break_position);
    } else {
      // Current line includes the rest of the glyph run, add it to the line and
      // update the line metrics
      add_glyph_run(run, text_start, run.end);
      // Switch to next glyph run
      ++run_idx;
      run_break_position = 0;
      intrinsic_width += rest_width;
      cumulative_width += rest_width;
    }
  }

  if (paragraph_style_.max_lines == 1 ||
      (paragraph_style_.unlimited_lines() && paragraph_style_.ellipsized())) {
    min_intrinsic_width_ = max_intrinsic_width_;
  }

#if 0
  for (const auto& line_metric : line_metrics_) {
    FML_DLOG(ERROR) << "metric: [" << line_metric.start_index << ", "
                    << line_metric.end_index << ", "
                    << line_metric.end_excluding_whitespace << ", "
                    << line_metric.end_including_newline << ", "
                    << line_metric.hard_break << "], ["
                    << line_metric.line_number << ", " << line_metric.width
                    << ", " << line_metric.height << "]";

    for (const auto& it : line_metric.run_metrics) {
      FML_LOG(ERROR) << "\t" << it.first << ", "
                     << it.second.font_metrics.fAscent;
    }
  }
  FML_LOG(ERROR) << "max intrinsic width: " << max_intrinsic_width_
                 << ", min intrinsic width: " << min_intrinsic_width_;
#endif
}

// We assume the text is all CJK unified ideographic
void ParagraphCJK::Layout(double width) {
  double rounded_width = floor(width);
  if ((!needs_layout_ && rounded_width == width_) || rounded_width == 0) {
    return;
  }

  width_ = rounded_width;
  needs_layout_ = false;

  if (text_.empty()) {
    return;
  }

  SkFont sk_font;
  sk_font.setEdging(SkFont::Edging::kAntiAlias);
  sk_font.setSubpixel(true);
  sk_font.setHinting(SkFontHinting::kSlight);
  sk_font.setLinearMetrics(true);

  if (is_first_layout_) {
    is_first_layout_ = false;
    ComputeStrut(&strut_, sk_font);
    ComputeStyledRuns();
  }

  if (glyph_runs_.empty()) {
    size_t inline_placeholder_index = 0;
    for (size_t i = 0; i < script_runs_.size(); ++i) {
      const auto& run = script_runs_[i];
      if (run.end - run.start == 1 &&
          obj_replacement_char_indexes_.count(run.start) != 0 &&
          text_[run.start] == kObjReplacementChar &&
          inline_placeholder_index < inline_placeholders_.size()) {
        // Is an inline placeholder run
        placeholder_run_indexes_.emplace_back(glyph_runs_.size());
        glyph_runs_.push_back(std::make_unique<PlaceholderGlyphRun>(
            sk_font, run.style, run.start, run.end,
            inline_placeholders_[inline_placeholder_index]));
        ++inline_placeholder_index;
      } else {
        ItemizeScriptRun(run, sk_font);
      }
    }
#if 0
    FML_LOG(ERROR) << "glyph ids:";
    for (const auto& run : glyph_runs_) {
      for (size_t i = 0; i < run->glyph_count(); i++) {
        FML_DLOG(ERROR) << run->get_glyph_id(i);
      }
    }
#endif
  }
  ComputeTextLines(sk_font);
}

bool ParagraphCJK::IsStrutValid() {
  return paragraph_style_.strut_enabled &&
         paragraph_style_.strut_font_size >= 0;
}

double ParagraphCJK::GetStrutAscent() {
  return IsStrutValid() ? strut_.ascent + strut_.half_leading
                        : std::numeric_limits<double>::lowest();
}

double ParagraphCJK::GetStrutDescent() {
  return IsStrutValid() ? strut_.descent + strut_.half_leading
                        : std::numeric_limits<double>::lowest();
}

void ParagraphCJK::ComputeStrut(StrutMetrics* strut, SkFont& font) {
  strut->ascent = 0;
  strut->descent = 0;
  strut->leading = 0;
  strut->half_leading = 0;
  strut->line_height = 0;
  strut->force_strut = false;

  if (!IsStrutValid())
    return;

  // force_strut makes all lines have exactly the strut metrics, and ignores all
  // actual metrics. We only force the strut if the strut is non-zero and valid.
  strut->force_strut = paragraph_style_.force_strut_height;
  minikin::FontStyle minikin_font_style(
      0, get_weight(paragraph_style_.strut_font_weight),
      paragraph_style_.strut_font_style == FontStyle::italic);

  std::shared_ptr<minikin::FontCollection> collection =
      font_collection_->GetMinikinFontCollectionForFamilies(
          paragraph_style_.strut_font_families, "");
  if (!collection) {
    return;
  }
  minikin::FakedFont faked_font = collection->baseFontFaked(minikin_font_style);

  if (faked_font.font != nullptr) {
    SkString str;
    static_cast<FontSkia*>(faked_font.font)
        ->GetSkTypeface()
        ->getFamilyName(&str);
    font.setTypeface(static_cast<FontSkia*>(faked_font.font)->GetSkTypeface());
    font.setSize(paragraph_style_.strut_font_size);
    SkFontMetrics strut_metrics;
    font.getMetrics(&strut_metrics);

    const double metrics_height =
        -strut_metrics.fAscent + strut_metrics.fDescent;

    if (paragraph_style_.strut_has_height_override) {
      const double strut_height =
          paragraph_style_.strut_height * paragraph_style_.strut_font_size;
      const double metrics_leading =
          // Zero extra leading if there is no user specified strut leading.
          paragraph_style_.strut_leading < 0
              ? 0
              : (paragraph_style_.strut_leading *
                 paragraph_style_.strut_font_size);

      const double available_height =
          paragraph_style_.strut_half_leading ? metrics_height : strut_height;

      strut->ascent =
          (-strut_metrics.fAscent / metrics_height) * available_height;
      strut->descent =
          (strut_metrics.fDescent / metrics_height) * available_height;

      strut->leading = metrics_leading + strut_height - available_height;
    } else {
      strut->ascent = -strut_metrics.fAscent;
      strut->descent = strut_metrics.fDescent;
      strut->leading =
          // Use font's leading if there is no user specified strut leading.
          paragraph_style_.strut_leading < 0
              ? strut_metrics.fLeading
              : (paragraph_style_.strut_leading *
                 paragraph_style_.strut_font_size);
    }
    strut->half_leading = strut->leading / 2;
    strut->line_height = strut->ascent + strut->descent + strut->leading;
  }
}

double ParagraphCJK::GetLineXOffset(double line_total_advance,
                                    bool justify_line) {
  if (isinf(width_)) {
    return 0;
  }
  TextAlign align = paragraph_style_.effective_align();
  if (align == TextAlign::right ||
      (align == TextAlign::justify &&
       paragraph_style_.text_direction == TextDirection::rtl &&
       !justify_line)) {
    return width_ - line_total_advance;
  } else if (align == TextAlign::center) {
    return (width_ - line_total_advance) / 2;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
//                Copy and modified from paragraph_txt.cc                     //
////////////////////////////////////////////////////////////////////////////////
void ParagraphCJK::ComputePlaceholderMetrics(
    txt::PlaceholderRun* placeholder_run,
    double& ascent,
    double& descent) {
  if (placeholder_run == nullptr) {
    return;
  }
  // Calculate how much to shift the ascent and descent to account for the
  // baseline choice.
  double baseline_adjustment = 0;
  switch (placeholder_run->baseline) {
    case TextBaseline::kAlphabetic: {
      baseline_adjustment = 0;
      break;
    }
    case TextBaseline::kIdeographic: {
      baseline_adjustment = -descent / 2;
      break;
    }
  }
  // Convert the ascent and descent from the font's to the placeholder rect's.
  switch (placeholder_run->alignment) {
    case PlaceholderAlignment::kBaseline: {
      ascent = baseline_adjustment + placeholder_run->baseline_offset;
      descent = -baseline_adjustment + placeholder_run->height -
                placeholder_run->baseline_offset;
      break;
    }
    case PlaceholderAlignment::kAboveBaseline: {
      ascent = baseline_adjustment + placeholder_run->height;
      descent = -baseline_adjustment;
      break;
    }
    case PlaceholderAlignment::kBelowBaseline: {
      descent = baseline_adjustment + placeholder_run->height;
      ascent = -baseline_adjustment;
      break;
    }
    case PlaceholderAlignment::kTop: {
      descent = placeholder_run->height - ascent;
      break;
    }
    case PlaceholderAlignment::kBottom: {
      ascent = placeholder_run->height - descent;
      break;
    }
    case PlaceholderAlignment::kMiddle: {
      double mid = (ascent - descent) / 2;
      ascent = mid + placeholder_run->height / 2;
      descent = -mid + placeholder_run->height / 2;
      break;
    }
  }
  placeholder_run->baseline_offset = ascent;
}

void ParagraphCJK::UpdateLineMetrics(const SkFontMetrics& metrics,
                                     const txt::TextStyle& style,
                                     double& max_ascent,
                                     double& max_descent,
                                     double& max_unscaled_ascent,
                                     txt::PlaceholderRun* placeholder_run,
                                     size_t line_number,
                                     size_t line_limit) {
  if (!strut_.force_strut) {
    const double metrics_font_height = metrics.fDescent - metrics.fAscent;
    // The overall height of the glyph blob. If neither the ascent or the
    // descent is disabled, we have block_height = ascent + descent, where
    // "ascent" is the extent from the top of the blob to its baseline, and
    // "descent" is the extent from the text blob's baseline to its bottom. Not
    // to be mistaken with the font's ascent and descent.
    const double blob_height = style.has_height_override
                                   ? style.height * style.font_size
                                   : metrics_font_height + metrics.fLeading;

    // Scale the ascent and descent such that the sum of ascent and
    // descent is `style.height * style.font_size`.
    //
    // The raw metrics do not add up to fontSize. The state of font
    // metrics is a mess:
    //
    // Each font has 4 sets of vertical metrics:
    //
    // * hhea: hheaAscender, hheaDescender, hheaLineGap.
    //     Used by Apple.
    // * OS/2 typo: typoAscender, typoDescender, typoLineGap.
    //     Used sometimes by Windows for layout.
    // * OS/2 win: winAscent, winDescent.
    //     Also used by Windows, generally will be cut if extends past
    //     these metrics.
    // * EM Square: ascent, descent
    //     Not actively used, but this defines the 'scale' of the
    //     units used.
    //
    // `Use Typo Metrics` is a boolean that, when enabled, prefers
    // typo metrics over win metrics. Default is off. Enabled by most
    // modern fonts.
    //
    // In addition to these different sets of metrics, there are also
    // multiple strategies for using these metrics:
    //
    // * Adobe: Set hhea values to typo equivalents.
    // * Microsoft: Set hhea values to win equivalents.
    // * Web: Use hhea values for text, regardless of `Use Typo Metrics`
    //     The hheaLineGap is distributed half across the top and half
    //     across the bottom of the line.
    //   Exceptions:
    //     Windows: All browsers respect `Use Typo Metrics`
    //     Firefox respects `Use Typo Metrics`.
    //
    // This pertains to this code in that it is ambiguous which set of
    // metrics we are actually using via SkFontMetrics. This in turn
    // means that if we use the raw metrics, we will see differences
    // between platforms as well as unpredictable line heights.
    //
    // A more thorough explanation is available at
    // https://glyphsapp.com/tutorials/vertical-metrics
    //
    // Doing this ascent/descent normalization to the EM Square allows
    // a sane, consistent, and reasonable "blob_height" to be specified,
    // though it breaks with what is done by any of the platforms above.
    const bool shouldNormalizeFont =
        style.has_height_override && !style.half_leading;
    const double font_height =
        shouldNormalizeFont ? style.font_size : metrics_font_height;

    // Reserve the outermost vertical space we want to distribute evenly over
    // and under the text ("half-leading").
    double leading;
    if (style.half_leading) {
      leading = blob_height - font_height;
    } else {
      leading = style.has_height_override ? 0.0 : metrics.fLeading;
    }
    const double half_leading = leading / 2;

    // Proportionally distribute the remaining vertical space above and below
    // the glyph blob's baseline, per the font's ascent/discent ratio.
    const double available_vspace = blob_height - leading;
    const double modifiedAscent =
        -metrics.fAscent / metrics_font_height * available_vspace +
        half_leading;
    const double modifiedDescent =
        metrics.fDescent / metrics_font_height * available_vspace +
        half_leading;

    const bool disableAscent =
        line_number == 0 && paragraph_style_.text_height_behavior &
                                TextHeightBehavior::kDisableFirstAscent;
    const bool disableDescent = line_number == line_limit - 1 &&
                                paragraph_style_.text_height_behavior &
                                    TextHeightBehavior::kDisableLastDescent;

    double ascent = disableAscent ? -metrics.fAscent : modifiedAscent;
    double descent = disableDescent ? metrics.fDescent : modifiedDescent;

    ComputePlaceholderMetrics(placeholder_run, ascent, descent);

    max_ascent = std::max(ascent, max_ascent);
    max_descent = std::max(descent, max_descent);
  }

  max_unscaled_ascent =
      std::max(placeholder_run == nullptr ? -metrics.fAscent
                                          : placeholder_run->baseline_offset,
               max_unscaled_ascent);
}
////////////////////////////////////////////////////////////////////////////////
//                Copy and modified from paragraph_txt.cc                     //
////////////////////////////////////////////////////////////////////////////////

void ParagraphCJK::Paint(SkCanvas* canvas, double x, double y) {
  SkPoint base_offset = SkPoint::Make(x, y);
  SkPaint paint;
  for (const auto& record : paint_records_) {
    record.paint_background(canvas, base_offset);
  }
  for (const auto& record : paint_records_) {
    if (record.GetPlaceholderRun() == nullptr) {
      if (record.style().has_foreground) {
        paint = record.style().foreground;
      } else {
        paint.reset();
        paint.setColor(record.style().color);
      }
      record.paint_glyphs_with_shadow(canvas, base_offset, paint);
    }
    record.paint_decorations(canvas, base_offset);
  }
}

#define CHECK_LAYOUT (FML_DCHECK(!needs_layout_) << "only valid after layout")

double ParagraphCJK::GetAlphabeticBaseline() {
  CHECK_LAYOUT;
  return alphabetic_baseline_;
}

double ParagraphCJK::GetIdeographicBaseline() {
  CHECK_LAYOUT;
  return ideographic_baseline_;
}

double ParagraphCJK::GetMaxIntrinsicWidth() {
  CHECK_LAYOUT;
  return max_intrinsic_width_;
}

double ParagraphCJK::GetMinIntrinsicWidth() {
  CHECK_LAYOUT;
  return min_intrinsic_width_;
}

double ParagraphCJK::GetHeight() {
  CHECK_LAYOUT;
  return final_line_count_ == 0 ? 0
                                : line_metrics_[final_line_count_ - 1].height;
}

double ParagraphCJK::GetMaxWidth() {
  CHECK_LAYOUT;
  return width_;
}

double ParagraphCJK::GetLongestLine() {
  CHECK_LAYOUT;
  return longest_line_;
}

bool ParagraphCJK::DidExceedMaxLines() {
  CHECK_LAYOUT;
  return did_exceed_max_lines_;
}

std::vector<LineMetrics>& ParagraphCJK::GetLineMetrics() {
  CHECK_LAYOUT;
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
  CHECK_LAYOUT;
  std::vector<Paragraph::TextBox> boxes;
  for (size_t placeholder_run_index : placeholder_run_indexes_) {
    const auto& run = static_cast<const PlaceholderGlyphRun&>(
        *(glyph_runs_[placeholder_run_index]));
    double baseline = line_metrics_[run.line_num].baseline;
    double top = baseline - run.placeholder.baseline_offset;
    double bottom =
        baseline + run.placeholder.height - run.placeholder.baseline_offset;
    double left = run.left;
    double right = left + run.placeholder.width;
    // TODO(nano): currently only support LTR
    boxes.emplace_back(SkRect::MakeLTRB(left, top, right, bottom),
                       TextDirection::ltr);
  }
  return boxes;
}

Paragraph::PositionWithAffinity ParagraphCJK::GetGlyphPositionAtCoordinate(
    double dx,
    double dy) {
  CHECK_LAYOUT;
  if (final_line_count_ <= 0) {
    return {0, Affinity::DOWNSTREAM};
  }
  return {0, Affinity::DOWNSTREAM};
}

Paragraph::Range<size_t> ParagraphCJK::GetWordBoundary(size_t offset) {
  CHECK_LAYOUT;
  if (text_.size() == 0) {
    return Range<size_t>(0, 0);
  }

  if (!word_breaker_) {
    UErrorCode status = U_ZERO_ERROR;
    word_breaker_.reset(
        icu::BreakIterator::createWordInstance(icu::Locale(), status));
    if (!U_SUCCESS(status)) {
      return Range<size_t>(0, 0);
    }
  }

  icu::UnicodeString icu_text(false, text_.data(), text_.size());
  word_breaker_->setText(icu_text);

  int32_t prev_boundary = word_breaker_->preceding(offset + 1);
  int32_t next_boundary = word_breaker_->next();
  if (prev_boundary == icu::BreakIterator::DONE) {
    prev_boundary = offset;
  }
  if (next_boundary == icu::BreakIterator::DONE) {
    next_boundary = offset;
  }
  return Range<size_t>(prev_boundary, next_boundary);
}

}  // namespace txt
