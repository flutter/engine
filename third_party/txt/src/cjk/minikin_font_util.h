#ifndef FLUTTER_TXT_CJK_MINIKIN_FONT_UTIL_H
#define FLUTTER_TXT_CJK_MINIKIN_FONT_UTIL_H

#include "minikin/FontCollection.h"
#include "minikin/FontFamily.h"
#include "minikin/MinikinFont.h"
#include "txt/font_collection.h"
#include "txt/font_weight.h"
#include "txt/text_style.h"

namespace txt {

int get_weight(const FontWeight weight);

bool is_italic(const TextStyle& style);

minikin::FontStyle get_minikin_font_style(const TextStyle& style);

void get_minikin_font_paint(const TextStyle& style,
                            minikin::FontStyle* font,
                            minikin::MinikinPaint* paint);

std::shared_ptr<minikin::FontCollection> get_minikin_font_collection_for_style(
    std::shared_ptr<FontCollection> font_collection,
    const TextStyle& style);

sk_sp<SkTypeface> get_default_skia_typeface(
    std::shared_ptr<FontCollection> font_collection,
    const TextStyle& style);

std::shared_ptr<minikin::FontFamily> get_cjk_font_family(
    std::shared_ptr<minikin::FontCollection> minikin_font_collection,
    const minikin::FontStyle& minikin_style);

std::shared_ptr<minikin::FontFamily> get_space_font_family(
    std::shared_ptr<minikin::FontCollection> minikin_font_collection,
    const minikin::FontStyle& minikin_style);

}  // namespace txt

#endif  // FLUTTER_TXT_CJK_MINIKIN_FONT_UTIL_H
