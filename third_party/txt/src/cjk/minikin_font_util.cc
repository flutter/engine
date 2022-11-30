#include "minikin_font_util.h"

#include "minikin/FontLanguageListCache.h"
#include "txt/font_skia.h"
#include "utils/JenkinsHash.h"

namespace txt {

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

struct font_family_key_t {
  const uint32_t font_collection_id;
  const minikin::FontStyle minikin_style;

  bool operator==(const font_family_key_t& other) const {
    return font_collection_id == other.font_collection_id &&
           minikin_style == other.minikin_style;
  }
};

struct font_family_hash_t {
  size_t operator()(const font_family_key_t& key) const {
    uint32_t hash = key.minikin_style.hash();
    hash = android::JenkinsHashMix(hash, key.font_collection_id);
    return android::JenkinsHashWhiten(hash);
  }
};

constexpr uint32_t kCJKAnchor = 0x4E2D;
constexpr uint32_t kSpaceAnchor = ' ';

using family_map_t = std::unordered_map<font_family_key_t,
                                        std::shared_ptr<minikin::FontFamily>,
                                        font_family_hash_t>;

std::shared_ptr<minikin::FontFamily> get_anchor_font_family(
    std::shared_ptr<minikin::FontCollection> minikin_font_collection,
    const minikin::FontStyle& minikin_style,
    family_map_t& cache,
    uint32_t anchor) {
  std::shared_ptr<minikin::FontFamily> font_family;
  const font_family_key_t key{minikin_font_collection->getId(), minikin_style};
  const auto& family_it = cache.find(key);
  if (family_it != cache.end()) {
    font_family = family_it->second;
  } else {
    const uint32_t lang_list_id = minikin_style.getLanguageListId();
    const int variant = minikin_style.getVariant();
    const auto& family = minikin_font_collection->getFamilyForChar(
        anchor, 0, lang_list_id, variant);
    cache.emplace(key, family);
    font_family = family;
  }
  return font_family;
}

std::shared_ptr<minikin::FontFamily> get_cjk_font_family(
    std::shared_ptr<minikin::FontCollection> minikin_font_collection,
    const minikin::FontStyle& minikin_style) {
  static family_map_t cjk_family_map;
  return get_anchor_font_family(minikin_font_collection, minikin_style,
                                cjk_family_map, kCJKAnchor);
}

std::shared_ptr<minikin::FontFamily> get_space_font_family(
    std::shared_ptr<minikin::FontCollection> minikin_font_collection,
    const minikin::FontStyle& minikin_style) {
  static family_map_t space_family_map;
  return get_anchor_font_family(minikin_font_collection, minikin_style,
                                space_family_map, kSpaceAnchor);
}

}  // namespace txt
