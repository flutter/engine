#include "glyph_itemize.h"

#include <hb-icu.h>
#include <hb-ot.h>
#include <include/core/SkCanvas.h>

#include "cjk_utils.h"
#include "glyph_run.h"
#include "otf_cmap.h"

namespace txt {

SkScalar get_ideographic_advance(uint16_t glyph_id, SkFont& sk_font) {
  SkScalar advance;
  SkPaint sk_paint;
  sk_font.getWidthsBounds(&glyph_id, 1, &advance, nullptr, &sk_paint);
  return advance;
}

ITEMIZE_METHOD(skfont) {
  // Convert chars to glyphs
  // The capacity definitely >= the actual glyph count, since a single CJK
  // character may compose of 2 u16 code unit
  const size_t u16_count = run.end - run.start;
  auto glyph_ids = std::make_unique<SkGlyphID[]>(u16_count);
  sk_font.textToGlyphs(text.data() + run.start, u16_count * sizeof(uint16_t),
                       SkTextEncoding::kUTF16, glyph_ids.get(), u16_count);

  SkScalar ideographic_advance = get_ideographic_advance(glyph_ids[0], sk_font);
  return std::make_unique<IdeographicGlyphRun>(sk_font, run.style, run.start,
                                               run.end, ideographic_advance,
                                               std::move(glyph_ids));
}

float hb_fixed_to_float(hb_position_t v) {
  return scalbnf(v, -8);
}

hb_position_t hb_float_to_fixed(float v) {
  return scalbnf(v, +8);
}

hb_font_t* get_hb_font(const minikin::MinikinFont* minikin_font) {
  // Get hb font from cache
  // TODO(nano) better to destroy the hb-font after it is not in use, but that's
  // OK since the cache will exists in the whole APP lifecycle.
  static std::unordered_map<int32_t, hb_font_t*> hb_font_cache;
  const int32_t font_id = minikin_font->GetUniqueId();
  auto&& it = hb_font_cache.find(font_id);
  if (it != hb_font_cache.end()) {
    return hb_font_reference(it->second);
  }
  hb_face_t* face = minikin_font->CreateHarfBuzzFace();
  hb_font_t* temp_font = hb_font_create(face);
  hb_ot_font_set_funcs(temp_font);
  unsigned int upem = hb_face_get_upem(face);
  hb_font_set_scale(temp_font, upem, upem);

  hb_font_t* sub_font = hb_font_create_sub_font(temp_font);
  std::vector<hb_variation_t> variations;
  for (const auto& variation : minikin_font->GetAxes()) {
    variations.push_back({variation.axisTag, variation.value});
  }
  hb_font_set_variations(sub_font, variations.data(), variations.size());

  hb_font_destroy(temp_font);
  hb_face_destroy(face);

  hb_font_cache.emplace(font_id, sub_font);
  return hb_font_reference(sub_font);
}

ITEMIZE_METHOD(hb) {
  static auto hb_buffer = []() -> hb_buffer_t* {
    hb_unicode_funcs_t* unicode_funcs =
        hb_unicode_funcs_create(hb_icu_get_unicode_funcs());
    auto buffer = hb_buffer_create();
    hb_buffer_set_unicode_funcs(buffer, unicode_funcs);
    return buffer;
  }();
  static auto reset_hb_buffer = []() {
    hb_buffer_clear_contents(hb_buffer);
    // Currently only support LTR text
    hb_buffer_set_direction(hb_buffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(hb_buffer, HB_SCRIPT_HAN);
    hb_buffer_set_language(hb_buffer, hb_language_from_string("zh", -1));
  };

  hb_font_t* hb_font = get_hb_font(minikin_font);
  float size = sk_font.getSize();
  hb_font_set_ppem(hb_font, size, size);
  hb_font_set_scale(hb_font, hb_float_to_fixed(size), hb_float_to_fixed(size));

  reset_hb_buffer();
  hb_buffer_add_utf16(hb_buffer, text.data(), text.size(), run.start,
                      run.end - run.start);

  hb_shape(hb_font, hb_buffer, NULL, 0);
  unsigned int glyph_count;
  hb_glyph_info_t* glyph_info =
      hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos =
      hb_buffer_get_glyph_positions(hb_buffer, NULL);

  SkScalar ideographic_advance = hb_fixed_to_float(glyph_pos[0].x_advance);
  auto glyph_ids = std::make_unique<SkGlyphID[]>(glyph_count);
  for (size_t i = 0; i < glyph_count; ++i) {
    glyph_ids[i] = glyph_info[i].codepoint;
  }

  // Dereference hb font
  hb_font_destroy(hb_font);

  return std::make_unique<IdeographicGlyphRun>(sk_font, run.style, run.start,
                                               run.end, ideographic_advance,
                                               std::move(glyph_ids));
}

std::unique_ptr<GlyphRun> get_glyph_run_util(
    const std::vector<uint16_t>& text,
    const ScriptRun& run,
    SkFont& sk_font,
    const std::function<uint16_t(uint32_t)>& get_glyph_id) {
  const size_t len = run.end - run.start;
  auto glyph_ids = std::make_unique<SkGlyphID[]>(len);

  size_t cnt = 0;
  for (size_t i = run.start; i < run.end;) {
    uint32_t codepoint = next_u16_unicode(text.data(), len, i);
    glyph_ids[cnt++] = get_glyph_id(codepoint);
  }

  SkScalar ideographic_advance = get_ideographic_advance(glyph_ids[0], sk_font);
  return std::make_unique<IdeographicGlyphRun>(sk_font, run.style, run.start,
                                               run.end, ideographic_advance,
                                               std::move(glyph_ids));
}

ITEMIZE_METHOD(hb_font) {
  hb_font_t* hb_font = get_hb_font(minikin_font);
  auto&& glyph_run = get_glyph_run_util(
      text, run, sk_font, [hb_font](uint32_t codepoint) -> uint16_t {
        uint32_t glyph_id;
        bool found = hb_font_get_glyph(hb_font, codepoint, 0, &glyph_id);
        return found ? glyph_id : 0;
      });
  hb_font_destroy(hb_font);
  return glyph_run;
}

ITEMIZE_METHOD(cmap) {
  static std::unordered_map<int32_t, std::unique_ptr<OtfCmap>> cmaps;

  OtfCmap* cmap = nullptr;
  auto&& it = cmaps.find(minikin_font->GetUniqueId());
  if (it != cmaps.end()) {
    cmap = it->second.get();
  } else {
    auto unique_cmap = std::make_unique<OtfCmap>(sk_font.refTypeface());
    cmap = unique_cmap.get();
    cmaps.emplace(minikin_font->GetUniqueId(), std::move(unique_cmap));
  }
  return get_glyph_run_util(text, run, sk_font,
                            [cmap](uint32_t codepoint) -> uint16_t {
                              return cmap->getGlyphId(codepoint);
                            });
}

}  // namespace txt
