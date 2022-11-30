#include "glyph_itemize.h"

#include <flutter/fml/logging.h>
#include <hb-icu.h>
#include <hb-ot.h>
#include <include/core/SkCanvas.h>

#include "cjk_utils.h"
#include "glyph_run.h"
#include "otf_cmap.h"
#include "txt/font_collection.h"
#include "txt/font_skia.h"

namespace txt {

SkScalar get_ideographic_advance(uint16_t glyph_id, SkFont& sk_font) {
  SkScalar advance;
  SkPaint sk_paint;
  sk_font.getWidthsBounds(&glyph_id, 1, &advance, nullptr, &sk_paint);
  return advance;
}

SHAPE_METHOD(skfont_space) {
  // TODO(nano): cache the spaces
  const size_t cnt = run.end - run.start;
  auto glyph_ids = std::make_unique<SkGlyphID[]>(cnt);
  sk_font.textToGlyphs(text.data() + run.start, cnt * sizeof(uint16_t),
                       SkTextEncoding::kUTF16, glyph_ids.get(), cnt);
  auto advances = std::make_unique<SkScalar[]>(cnt);
  sk_font.getWidths(glyph_ids.get(), cnt, advances.get());

  auto code_unit_glyph_idx = std::make_unique<size_t[]>(cnt);
  auto positions = std::make_unique<SkScalar[]>(cnt << 1);
  SkScalar cursor_x = 0;
  for (size_t i = 0; i < cnt; ++i) {
    code_unit_glyph_idx[i] = i;
    positions[i << 1] = cursor_x;
    cursor_x += advances[i];
  }

  glyph_runs.emplace_back(std::make_unique<ComplexGlyphRun>(
      sk_font, run.style, run.start, run.end, cnt, std::move(glyph_ids),
      std::move(positions), std::move(advances), std::move(code_unit_glyph_idx),
      is_space_standalone, true));
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
  hb_font_t* tmp_font = hb_font_create(face);
  hb_ot_font_set_funcs(tmp_font);
  unsigned int upem = hb_face_get_upem(face);
  hb_font_set_scale(tmp_font, upem, upem);

  hb_font_t* sub_font = hb_font_create_sub_font(tmp_font);
  std::vector<hb_variation_t> variations;
  for (const auto& variation : minikin_font->GetAxes()) {
    variations.push_back({variation.axisTag, variation.value});
  }
  hb_font_set_variations(sub_font, variations.data(), variations.size());

  hb_font_destroy(tmp_font);
  hb_face_destroy(face);

  hb_font_cache.emplace(font_id, sub_font);
  return hb_font_reference(sub_font);
}

SHAPE_METHOD(hb_complex) {
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
    hb_buffer_set_script(hb_buffer, HB_SCRIPT_COMMON);
    hb_buffer_set_language(hb_buffer, hb_language_from_string("en", -1));
  };

  // TODO(nano): retrieve from minikin HBFontCache?
  hb_font_t* hb_font = get_hb_font(minikin_font);
  double size = minikin_paint.size;
  double scale_x = minikin_paint.scaleX;
  hb_font_set_ppem(hb_font, size * scale_x, size);
  hb_font_set_scale(hb_font, hb_float_to_fixed(size * scale_x),
                    hb_float_to_fixed(size));

  reset_hb_buffer();
  hb_buffer_add_utf16(hb_buffer, text.data(), text.size(), run.start,
                      run.end - run.start);

  hb_shape(hb_font, hb_buffer, NULL, 0);
  unsigned int glyph_cnt;
  hb_glyph_info_t* glyph_info =
      hb_buffer_get_glyph_infos(hb_buffer, &glyph_cnt);
  hb_glyph_position_t* glyph_pos =
      hb_buffer_get_glyph_positions(hb_buffer, NULL);

  auto glyph_ids = std::make_unique<SkGlyphID[]>(glyph_cnt);
  auto advances = std::make_unique<SkScalar[]>(glyph_cnt);
  auto positions = std::make_unique<SkScalar[]>(glyph_cnt << 1);
  SkScalar cursor_x = 0;
  SkScalar cursor_y = 0;
  for (size_t i = 0; i < glyph_cnt; ++i) {
    const auto& info = glyph_info[i];
    glyph_ids[i] = info.codepoint;

    const auto& pos = glyph_pos[i];
    SkScalar x_offset = hb_fixed_to_float(pos.x_offset);
    SkScalar y_offset = hb_fixed_to_float(pos.y_offset);
    SkScalar x_advance = hb_fixed_to_float(pos.x_advance);
    SkScalar y_advance = hb_fixed_to_float(pos.y_advance);
    advances[i] = x_advance;

    size_t j = i << 1;
    positions[j] = cursor_x + x_offset;
    positions[j + 1] = cursor_y + y_offset;

    // Forward the cursor
    cursor_x += x_advance;
    cursor_y += y_advance;
  }

  // Dereference hb font
  hb_font_destroy(hb_font);

  auto code_unit_glyph_idx = std::make_unique<size_t[]>(run.end - run.start);
  // Definitely no clusters
  for (size_t i = 0; i < glyph_cnt; ++i) {
    code_unit_glyph_idx[i] = i;
  }
  glyph_runs.emplace_back(std::make_unique<ComplexGlyphRun>(
      sk_font, run.style, run.start, run.end, glyph_cnt, std::move(glyph_ids),
      std::move(positions), std::move(advances), std::move(code_unit_glyph_idx),
      true));
}

inline bool operator!=(minikin::FontFakery& a, minikin::FontFakery& b) {
  return a.isFakeBold() != b.isFakeBold() ||
         a.isFakeItalic() != b.isFakeItalic();
}

struct typeface_run_t {
  SkFont font;
  size_t start;
  size_t end;
};

inline SkFont get_font(SkFont& sk_font,
                       sk_sp<SkTypeface>& typeface,
                       minikin::FontFakery& fakery) {
  SkFont font = sk_font;
  font.setTypeface(typeface);
  font.setEmbolden(fakery.isFakeBold());
  font.setSkewX(fakery.isFakeItalic() ? -SK_Scalar1 / 4 : 0);
  return font;
}

std::vector<typeface_run_t> get_typeface_runs(const minikin::Layout& layout,
                                              SkFont& sk_font) {
  std::vector<typeface_run_t> runs;
  const size_t glyph_cnt = layout.nGlyphs();
  if (glyph_cnt == 0) {
    return runs;
  }

#define get_layout_font(i, t, f)                                        \
  t = static_cast<const FontSkia*>(layout.getFont(i))->GetSkTypeface(); \
  f = layout.getFakery(i);

  size_t run_start = 0;
  sk_sp<SkTypeface> typeface;
  minikin::FontFakery fakery;

  get_layout_font(0, typeface, fakery);
  for (size_t i = 1; i < glyph_cnt; ++i) {
    sk_sp<SkTypeface> glyph_typeface;
    minikin::FontFakery glyph_fakery;
    get_layout_font(i, glyph_typeface, glyph_fakery);

    if (glyph_typeface != typeface || fakery != glyph_fakery) {
      auto font = get_font(sk_font, typeface, fakery);
      runs.push_back({font, run_start, i});
      typeface = glyph_typeface;
      fakery = glyph_fakery;
      run_start = i;
    }
  }
  auto font = get_font(sk_font, typeface, fakery);
  runs.push_back({font, run_start, glyph_cnt});

  return runs;
#undef get_layout_font
}

struct cluster_t {
  size_t start;
  size_t end;
};

SHAPE_METHOD(minikin_complex) {
  const size_t text_count = run.end - run.start;
  layout.doLayout(text.data(), run.start, text_count, text.size(),
                  /* TODO(nano) Currently only support LTR */ false,
                  minikin_style, minikin_paint, font_collection);
  if (layout.nGlyphs() == 0) {
    return;
  }

  // All glyphs in this layout has the same font size
  sk_font.setSize(minikin_paint.size);
  auto typeface_runs = get_typeface_runs(layout, sk_font);

  for (const auto& typeface_run : typeface_runs) {
    const size_t glyph_cnt = typeface_run.end - typeface_run.start;
    auto ids = std::make_unique<SkGlyphID[]>(glyph_cnt);
    auto pos = std::make_unique<SkScalar[]>(glyph_cnt << 1);

    const size_t start_cluster = layout.getGlyphCluster(typeface_run.start);
    const size_t end_cluster = typeface_run.end < layout.nGlyphs()
                                   ? layout.getGlyphCluster(typeface_run.end)
                                   : text_count;

    const size_t text_len = end_cluster - start_cluster;
    auto code_unit_glyph_idx = std::make_unique<size_t[]>(text_len);
    auto advances = std::make_unique<SkScalar[]>(text_len);
    // Advances are organized by code units
    layout.getAdvances(start_cluster, text_len, advances.get());

    // TODO(nano): use memset? But `memset` only support bytes
#define set_cluster(cluster)                                             \
  {                                                                      \
    const size_t cluster_glyph_idx = glyph_idx - typeface_run.start - 1; \
    const size_t start = prev_cluster - start_cluster;                   \
    const size_t end = cluster - start_cluster;                          \
    for (size_t i = start; i < end; ++i) {                               \
      code_unit_glyph_idx[i] = cluster_glyph_idx;                        \
    }                                                                    \
  }

    uint32_t prev_cluster = start_cluster;
    size_t glyph_idx = typeface_run.start;

    while (glyph_idx < typeface_run.end) {
      const uint32_t cluster = layout.getGlyphCluster(glyph_idx);
      if (cluster != prev_cluster) {
        set_cluster(cluster);
        prev_cluster = cluster;
      }
      // size_t cluster_start_glyph_idx = glyph_idx;

      // Retrieve all the glyphs in the same cluster.
      //
      // For buffers in the left-to-right (LTR) or top-to-bottom (TTB) text flow
      // direction, HarfBuzz will preserve the monotonic property: client
      // programs are guaranteed that monotonically increasing initial cluster
      // values will be returned as monotonically increasing final cluster
      // values.
      //
      // TODO(nano) Currently only support LTR
      //
      // See
      // https://harfbuzz.github.io/a-clustering-example-for-levels-0-and-1.html
      // and
      // https://harfbuzz.github.io/working-with-harfbuzz-clusters.html for
      // details.
      do {
        size_t run_item_idx = glyph_idx - typeface_run.start;
        ids[run_item_idx] = layout.getGlyphId(glyph_idx);
        size_t j = run_item_idx << 1;
        pos[j] = layout.getX(glyph_idx);
        pos[j + 1] = layout.getY(glyph_idx);
        ++glyph_idx;
      } while (glyph_idx < typeface_run.end &&
               layout.getGlyphCluster(glyph_idx) == cluster);

      cluster_t cluster_code_units{cluster, 0};
      cluster_code_units.end = glyph_idx < layout.nGlyphs()
                                   ? layout.getGlyphCluster(glyph_idx)
                                   : text_count;
    }
    set_cluster(end_cluster);
#undef set_cluster
#if 0
    FML_DLOG(ERROR) << "typeface run: [" << typeface_run.start << ","
                    << typeface_run.end << ","
                    << typeface_run.font.getTypeface() << "]";
    auto show_array = [](const char* label, size_t cnt, auto arr) {
      std::string arr_str("[");
      for (size_t i = 0; i < cnt; ++i) {
        arr_str.append(std::to_string(arr[i])).append(1, ',');
      }
      FML_DLOG(ERROR) << label << arr_str << "]";
    };
    show_array("ids: ", glyph_cnt, ids.get());
    show_array("cluster: ", text_len, code_unit_glyph_idx.get());
    show_array("advance: ", text_len, advances.get());
    FML_DLOG(ERROR) << "----------------------------------";
#endif

    size_t text_start = start_cluster + run.start;
    glyph_runs.emplace_back(std::make_unique<ComplexGlyphRun>(
        typeface_run.font, run.style, text_start, text_start + text_len,
        glyph_cnt, std::move(ids), std::move(pos), std::move(advances),
        std::move(code_unit_glyph_idx), is_space_standalone));
  }
}

void get_glyph_run_help(const std::vector<uint16_t>& text,
                        const ScriptRun& run,
                        SkFont& sk_font,
                        std::vector<std::unique_ptr<GlyphRun>>& glyph_runs,
                        const std::function<uint16_t(uint32_t)>& get_glyph_id) {
  const size_t len = run.end - run.start;
  auto glyph_ids = std::make_unique<SkGlyphID[]>(len);

  size_t cnt = 0;
  for (size_t i = run.start; i < run.end;) {
    // We only deal with the codepoints in BMP
    uint32_t codepoint = text[i];
    glyph_ids[cnt++] = get_glyph_id(codepoint);
  }

  SkScalar ideographic_advance = get_ideographic_advance(glyph_ids[0], sk_font);
  glyph_runs.emplace_back(std::make_unique<MonoWidthGlyphRun>(
      sk_font, run.style, run.start, run.end, ideographic_advance,
      std::move(glyph_ids)));
}

SHAPE_METHOD(hb_font) {
  hb_font_t* hb_font = get_hb_font(minikin_font);
  get_glyph_run_help(text, run, sk_font, glyph_runs,
                     [hb_font](uint32_t codepoint) -> uint16_t {
                       uint32_t glyph_id;
                       bool found =
                           hb_font_get_glyph(hb_font, codepoint, 0, &glyph_id);
                       return found ? glyph_id : 0;
                     });
  // Dereference the hb-font
  hb_font_destroy(hb_font);
}

SHAPE_METHOD(cmap) {
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
  get_glyph_run_help(text, run, sk_font, glyph_runs,
                     [cmap](uint32_t codepoint) -> uint16_t {
                       return cmap->getGlyphId(codepoint);
                     });
}

}  // namespace txt
