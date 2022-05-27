// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/text/paragraph_builder.h"

#include <cstring>

#include "flutter/common/settings.h"
#include "flutter/common/task_runners.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/task_runner.h"
#include "flutter/lib/ui/text/font_collection.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_configuration.h"
#include "flutter/third_party/txt/src/txt/font_style.h"
#include "flutter/third_party/txt/src/txt/font_weight.h"
#include "flutter/third_party/txt/src/txt/paragraph_style.h"
#include "flutter/third_party/txt/src/txt/text_baseline.h"
#include "flutter/third_party/txt/src/txt/text_decoration.h"
#include "flutter/third_party/txt/src/txt/text_style.h"
#include "third_party/icu/source/common/unicode/ustring.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {
namespace {

// TextStyle

const int kTsLeadingDistributionIndex = 0;
const int kTsColorIndex = 1;
const int kTsTextDecorationIndex = 2;
const int kTsTextDecorationColorIndex = 3;
const int kTsTextDecorationStyleIndex = 4;
const int kTsFontWeightIndex = 5;
const int kTsFontStyleIndex = 6;
const int kTsTextBaselineIndex = 7;
const int kTsTextDecorationThicknessIndex = 8;
const int kTsFontFamilyIndex = 9;
const int kTsFontSizeIndex = 10;
const int kTsLetterSpacingIndex = 11;
const int kTsWordSpacingIndex = 12;
const int kTsHeightIndex = 13;
const int kTsLocaleIndex = 14;
const int kTsBackgroundIndex = 15;
const int kTsForegroundIndex = 16;
const int kTsTextShadowsIndex = 17;
const int kTsFontFeaturesIndex = 18;
const int kTsFontVariationsIndex = 19;

const int kTsLeadingDistributionMask = 1 << kTsLeadingDistributionIndex;
const int kTsColorMask = 1 << kTsColorIndex;
const int kTsTextDecorationMask = 1 << kTsTextDecorationIndex;
const int kTsTextDecorationColorMask = 1 << kTsTextDecorationColorIndex;
const int kTsTextDecorationStyleMask = 1 << kTsTextDecorationStyleIndex;
const int kTsTextDecorationThicknessMask = 1 << kTsTextDecorationThicknessIndex;
const int kTsFontWeightMask = 1 << kTsFontWeightIndex;
const int kTsFontStyleMask = 1 << kTsFontStyleIndex;
const int kTsTextBaselineMask = 1 << kTsTextBaselineIndex;
const int kTsFontFamilyMask = 1 << kTsFontFamilyIndex;
const int kTsFontSizeMask = 1 << kTsFontSizeIndex;
const int kTsLetterSpacingMask = 1 << kTsLetterSpacingIndex;
const int kTsWordSpacingMask = 1 << kTsWordSpacingIndex;
const int kTsHeightMask = 1 << kTsHeightIndex;
const int kTsLocaleMask = 1 << kTsLocaleIndex;
const int kTsBackgroundMask = 1 << kTsBackgroundIndex;
const int kTsForegroundMask = 1 << kTsForegroundIndex;
const int kTsTextShadowsMask = 1 << kTsTextShadowsIndex;
const int kTsFontFeaturesMask = 1 << kTsFontFeaturesIndex;
const int kTsFontVariationsMask = 1 << kTsFontVariationsIndex;

// ParagraphStyle

const int kPsTextAlignIndex = 1;
const int kPsTextDirectionIndex = 2;
const int kPsFontWeightIndex = 3;
const int kPsFontStyleIndex = 4;
const int kPsMaxLinesIndex = 5;
const int kPsTextHeightBehaviorIndex = 6;
const int kPsFontFamilyIndex = 7;
const int kPsFontSizeIndex = 8;
const int kPsHeightIndex = 9;
const int kPsStrutStyleIndex = 10;
const int kPsEllipsisIndex = 11;
const int kPsLocaleIndex = 12;

const int kPsTextAlignMask = 1 << kPsTextAlignIndex;
const int kPsTextDirectionMask = 1 << kPsTextDirectionIndex;
const int kPsFontWeightMask = 1 << kPsFontWeightIndex;
const int kPsFontStyleMask = 1 << kPsFontStyleIndex;
const int kPsMaxLinesMask = 1 << kPsMaxLinesIndex;
const int kPsFontFamilyMask = 1 << kPsFontFamilyIndex;
const int kPsFontSizeMask = 1 << kPsFontSizeIndex;
const int kPsHeightMask = 1 << kPsHeightIndex;
const int kPsTextHeightBehaviorMask = 1 << kPsTextHeightBehaviorIndex;
const int kPsStrutStyleMask = 1 << kPsStrutStyleIndex;
const int kPsEllipsisMask = 1 << kPsEllipsisIndex;
const int kPsLocaleMask = 1 << kPsLocaleIndex;

// TextShadows decoding

constexpr uint32_t kColorDefault = 0xFF000000;
constexpr uint32_t kBytesPerShadow = 16;
constexpr uint32_t kShadowPropertiesCount = 4;
constexpr uint32_t kColorOffset = 0;
constexpr uint32_t kXOffset = 1;
constexpr uint32_t kYOffset = 2;
constexpr uint32_t kBlurOffset = 3;

// FontFeature decoding
constexpr uint32_t kBytesPerFontFeature = 8;
constexpr uint32_t kFontFeatureTagLength = 4;

// FontVariation decoding
constexpr uint32_t kBytesPerFontVariation = 8;
constexpr uint32_t kFontVariationTagLength = 4;

// Strut decoding
const int kSFontWeightIndex = 0;
const int kSFontStyleIndex = 1;
const int kSFontFamilyIndex = 2;
const int kSLeadingDistributionIndex = 3;
const int kSFontSizeIndex = 4;
const int kSHeightIndex = 5;
const int kSLeadingIndex = 6;
const int kSForceStrutHeightIndex = 7;

const int kSFontWeightMask = 1 << kSFontWeightIndex;
const int kSFontStyleMask = 1 << kSFontStyleIndex;
const int kSFontFamilyMask = 1 << kSFontFamilyIndex;
const int kSLeadingDistributionMask = 1 << kSLeadingDistributionIndex;
const int kSFontSizeMask = 1 << kSFontSizeIndex;
const int kSHeightMask = 1 << kSHeightIndex;
const int kSLeadingMask = 1 << kSLeadingIndex;
const int kSForceStrutHeightMask = 1 << kSForceStrutHeightIndex;

}  // namespace

static void ParagraphBuilder_constructor(Dart_NativeArguments args) {
  UIDartState::ThrowIfUIOperationsProhibited();
  DartCallConstructor(&ParagraphBuilder::create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, ParagraphBuilder);

#define FOR_EACH_BINDING(V)           \
  V(ParagraphBuilder, pushStyle)      \
  V(ParagraphBuilder, pop)            \
  V(ParagraphBuilder, addText)        \
  V(ParagraphBuilder, addPlaceholder) \
  V(ParagraphBuilder, build)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void ParagraphBuilder::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"ParagraphBuilder_constructor", ParagraphBuilder_constructor, 9, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

fml::RefPtr<ParagraphBuilder> ParagraphBuilder::create(
    tonic::Int32List& encoded,
    Dart_Handle strutData,
    const std::string& fontFamily,
    const std::vector<std::string>& strutFontFamilies,
    double fontSize,
    double height,
    const std::u16string& ellipsis,
    const std::string& locale) {
  return fml::MakeRefCounted<ParagraphBuilder>(encoded, strutData, fontFamily,
                                               strutFontFamilies, fontSize,
                                               height, ellipsis, locale);
}

// returns true if there is a font family defined. Font family is the only
// parameter passed directly.
void decodeStrut(Dart_Handle strut_data,
                 const std::vector<std::string>& strut_font_families,
                 txt::ParagraphStyle& paragraph_style) {  // NOLINT
  if (strut_data == Dart_Null()) {
    return;
  }

  tonic::DartByteData byte_data(strut_data);
  if (byte_data.length_in_bytes() == 0) {
    return;
  }
  paragraph_style.strut_enabled = true;

  const uint8_t* uint8_data = static_cast<const uint8_t*>(byte_data.data());
  uint8_t mask = uint8_data[0];

  // Data is stored in order of increasing size, eg, 8 bit ints will be before
  // any 32 bit ints. In addition, the order of decoding is the same order
  // as it is encoded, and the order is used to maintain consistency.
  size_t byte_count = 1;
  if (mask & kSFontWeightMask) {
    paragraph_style.strut_font_weight =
        static_cast<txt::FontWeight>(uint8_data[byte_count++]);
  }
  if (mask & kSFontStyleMask) {
    paragraph_style.strut_font_style =
        static_cast<txt::FontStyle>(uint8_data[byte_count++]);
  }

  paragraph_style.strut_half_leading = mask & kSLeadingDistributionMask;

  std::vector<float> float_data;
  float_data.resize((byte_data.length_in_bytes() - byte_count) / 4);
  memcpy(float_data.data(),
         static_cast<const char*>(byte_data.data()) + byte_count,
         byte_data.length_in_bytes() - byte_count);
  size_t float_count = 0;
  if (mask & kSFontSizeMask) {
    paragraph_style.strut_font_size = float_data[float_count++];
  }
  if (mask & kSHeightMask) {
    paragraph_style.strut_height = float_data[float_count++];
    paragraph_style.strut_has_height_override = true;
  }
  if (mask & kSLeadingMask) {
    paragraph_style.strut_leading = float_data[float_count++];
  }

  // The boolean is stored as the last bit in the bitmask, as null
  // and false have the same behavior.
  paragraph_style.force_strut_height = mask & kSForceStrutHeightMask;

  if (mask & kSFontFamilyMask) {
    paragraph_style.strut_font_families = strut_font_families;
  } else {
    // Provide an empty font name so that the platform default font will be
    // used.
    paragraph_style.strut_font_families.push_back("");
  }
}

ParagraphBuilder::ParagraphBuilder(
    tonic::Int32List& encoded,
    Dart_Handle strutData,
    const std::string& fontFamily,
    const std::vector<std::string>& strutFontFamilies,
    double fontSize,
    double height,
    const std::u16string& ellipsis,
    const std::string& locale) {
  int32_t mask = encoded[0];
  txt::ParagraphStyle style;

  if (mask & kPsTextAlignMask) {
    style.text_align = static_cast<txt::TextAlign>(encoded[kPsTextAlignIndex]);
  }

  if (mask & kPsTextDirectionMask) {
    style.text_direction =
        static_cast<txt::TextDirection>(encoded[kPsTextDirectionIndex]);
  }

  if (mask & kPsFontWeightMask) {
    style.font_weight =
        static_cast<txt::FontWeight>(encoded[kPsFontWeightIndex]);
  }

  if (mask & kPsFontStyleMask) {
    style.font_style = static_cast<txt::FontStyle>(encoded[kPsFontStyleIndex]);
  }

  if (mask & kPsFontFamilyMask) {
    style.font_family = fontFamily;
  }

  if (mask & kPsFontSizeMask) {
    style.font_size = fontSize;
  }

  if (mask & kPsHeightMask) {
    style.height = height;
    style.has_height_override = true;
  }

  if (mask & kPsTextHeightBehaviorMask) {
    style.text_height_behavior = encoded[kPsTextHeightBehaviorIndex];
  }

  if (mask & kPsStrutStyleMask) {
    decodeStrut(strutData, strutFontFamilies, style);
  }

  if (mask & kPsMaxLinesMask) {
    style.max_lines = encoded[kPsMaxLinesIndex];
  }

  if (mask & kPsEllipsisMask) {
    style.ellipsis = ellipsis;
  }

  if (mask & kPsLocaleMask) {
    style.locale = locale;
  }

  FontCollection& font_collection = UIDartState::Current()
                                        ->platform_configuration()
                                        ->client()
                                        ->GetFontCollection();

  typedef std::unique_ptr<txt::ParagraphBuilder> (*ParagraphBuilderFactory)(
      const txt::ParagraphStyle& style,
      std::shared_ptr<txt::FontCollection> font_collection);
  ParagraphBuilderFactory factory = txt::ParagraphBuilder::CreateTxtBuilder;

#if FLUTTER_ENABLE_SKSHAPER
#if FLUTTER_ALWAYS_USE_SKSHAPER
  bool enable_skparagraph = true;
#else
  bool enable_skparagraph = UIDartState::Current()->enable_skparagraph();
#endif
  if (enable_skparagraph) {
    factory = txt::ParagraphBuilder::CreateSkiaBuilder;
  }
#endif  // FLUTTER_ENABLE_SKSHAPER

  m_paragraphBuilder = factory(style, font_collection.GetFontCollection());
}

ParagraphBuilder::~ParagraphBuilder() = default;

void decodeTextShadows(
    Dart_Handle shadows_data,
    std::vector<txt::TextShadow>& decoded_shadows) {  // NOLINT
  decoded_shadows.clear();

  tonic::DartByteData byte_data(shadows_data);
  FML_CHECK(byte_data.length_in_bytes() % kBytesPerShadow == 0);

  const uint32_t* uint_data = static_cast<const uint32_t*>(byte_data.data());
  const float* float_data = static_cast<const float*>(byte_data.data());

  size_t shadow_count = byte_data.length_in_bytes() / kBytesPerShadow;
  size_t shadow_count_offset = 0;
  for (size_t shadow_index = 0; shadow_index < shadow_count; ++shadow_index) {
    shadow_count_offset = shadow_index * kShadowPropertiesCount;
    SkColor color =
        uint_data[shadow_count_offset + kColorOffset] ^ kColorDefault;
    decoded_shadows.emplace_back(
        color,
        SkPoint::Make(float_data[shadow_count_offset + kXOffset],
                      float_data[shadow_count_offset + kYOffset]),
        float_data[shadow_count_offset + kBlurOffset]);
  }
}

void decodeFontFeatures(Dart_Handle font_features_data,
                        txt::FontFeatures& font_features) {  // NOLINT
  tonic::DartByteData byte_data(font_features_data);
  FML_CHECK(byte_data.length_in_bytes() % kBytesPerFontFeature == 0);

  size_t feature_count = byte_data.length_in_bytes() / kBytesPerFontFeature;
  for (size_t feature_index = 0; feature_index < feature_count;
       ++feature_index) {
    size_t feature_offset = feature_index * kBytesPerFontFeature;
    const char* feature_bytes =
        static_cast<const char*>(byte_data.data()) + feature_offset;
    std::string tag(feature_bytes, kFontFeatureTagLength);
    int32_t value = *(reinterpret_cast<const int32_t*>(feature_bytes +
                                                       kFontFeatureTagLength));
    font_features.SetFeature(tag, value);
  }
}

void decodeFontVariations(Dart_Handle font_variations_data,
                          txt::FontVariations& font_variations) {  // NOLINT
  tonic::DartByteData byte_data(font_variations_data);
  FML_CHECK(byte_data.length_in_bytes() % kBytesPerFontVariation == 0);

  size_t variation_count = byte_data.length_in_bytes() / kBytesPerFontVariation;
  for (size_t variation_index = 0; variation_index < variation_count;
       ++variation_index) {
    size_t variation_offset = variation_index * kBytesPerFontVariation;
    const char* variation_bytes =
        static_cast<const char*>(byte_data.data()) + variation_offset;
    std::string tag(variation_bytes, kFontVariationTagLength);
    float value = *(reinterpret_cast<const float*>(variation_bytes +
                                                   kFontVariationTagLength));
    font_variations.SetAxisValue(tag, value);
  }
}

void ParagraphBuilder::pushStyle(tonic::Int32List& encoded,
                                 const std::vector<std::string>& fontFamilies,
                                 double fontSize,
                                 double letterSpacing,
                                 double wordSpacing,
                                 double height,
                                 double decorationThickness,
                                 const std::string& locale,
                                 Dart_Handle background_objects,
                                 Dart_Handle background_data,
                                 Dart_Handle foreground_objects,
                                 Dart_Handle foreground_data,
                                 Dart_Handle shadows_data,
                                 Dart_Handle font_features_data,
                                 Dart_Handle font_variations_data) {
  FML_DCHECK(encoded.num_elements() == 9);

  int32_t mask = encoded[0];

  // Set to use the properties of the previous style if the property is not
  // explicitly given.
  txt::TextStyle style = m_paragraphBuilder->PeekStyle();

  style.half_leading = mask & kTsLeadingDistributionMask;
  // Only change the style property from the previous value if a new explicitly
  // set value is available
  if (mask & kTsColorMask) {
    style.color = encoded[kTsColorIndex];
  }

  if (mask & kTsTextDecorationMask) {
    style.decoration =
        static_cast<txt::TextDecoration>(encoded[kTsTextDecorationIndex]);
  }

  if (mask & kTsTextDecorationColorMask) {
    style.decoration_color = encoded[kTsTextDecorationColorIndex];
  }

  if (mask & kTsTextDecorationStyleMask) {
    style.decoration_style = static_cast<txt::TextDecorationStyle>(
        encoded[kTsTextDecorationStyleIndex]);
  }

  if (mask & kTsTextDecorationThicknessMask) {
    style.decoration_thickness_multiplier = decorationThickness;
  }

  if (mask & kTsTextBaselineMask) {
    // TODO(abarth): Implement TextBaseline. The CSS version of this
    // property wasn't wired up either.
  }

  if (mask & (kTsFontWeightMask | kTsFontStyleMask | kTsFontSizeMask |
              kTsLetterSpacingMask | kTsWordSpacingMask)) {
    if (mask & kTsFontWeightMask) {
      style.font_weight =
          static_cast<txt::FontWeight>(encoded[kTsFontWeightIndex]);
    }

    if (mask & kTsFontStyleMask) {
      style.font_style =
          static_cast<txt::FontStyle>(encoded[kTsFontStyleIndex]);
    }

    if (mask & kTsFontSizeMask) {
      style.font_size = fontSize;
    }

    if (mask & kTsLetterSpacingMask) {
      style.letter_spacing = letterSpacing;
    }

    if (mask & kTsWordSpacingMask) {
      style.word_spacing = wordSpacing;
    }
  }

  if (mask & kTsHeightMask) {
    style.height = height;
    style.has_height_override = true;
  }

  if (mask & kTsLocaleMask) {
    style.locale = locale;
  }

  if (mask & kTsBackgroundMask) {
    Paint background(background_objects, background_data);
    if (background.isNotNull()) {
      SkPaint sk_paint;
      style.has_background = true;
      style.background = *background.paint(sk_paint);
    }
  }

  if (mask & kTsForegroundMask) {
    Paint foreground(foreground_objects, foreground_data);
    if (foreground.isNotNull()) {
      SkPaint sk_paint;
      style.has_foreground = true;
      style.foreground = *foreground.paint(sk_paint);
    }
  }

  if (mask & kTsTextShadowsMask) {
    decodeTextShadows(shadows_data, style.text_shadows);
  }

  if (mask & kTsFontFamilyMask) {
    // The child style's font families override the parent's font families.
    // If the child's fonts are not available, then the font collection will
    // use the system fallback fonts (not the parent's fonts).
    style.font_families = fontFamilies;
  }

  if (mask & kTsFontFeaturesMask) {
    decodeFontFeatures(font_features_data, style.font_features);
  }

  if (mask & kTsFontVariationsMask) {
    decodeFontVariations(font_variations_data, style.font_variations);
  }

  m_paragraphBuilder->PushStyle(style);
}

void ParagraphBuilder::pop() {
  m_paragraphBuilder->Pop();
}

Dart_Handle ParagraphBuilder::addText(const std::u16string& text) {
  if (text.empty()) {
    return Dart_Null();
  }

  // Use ICU to validate the UTF-16 input.  Calling u_strToUTF8 with a null
  // output buffer will return U_BUFFER_OVERFLOW_ERROR if the input is well
  // formed.
  const UChar* text_ptr = reinterpret_cast<const UChar*>(text.data());
  UErrorCode error_code = U_ZERO_ERROR;
  u_strToUTF8(nullptr, 0, nullptr, text_ptr, text.size(), &error_code);
  if (error_code != U_BUFFER_OVERFLOW_ERROR) {
    return tonic::ToDart("string is not well-formed UTF-16");
  }

  m_paragraphBuilder->AddText(text);

  return Dart_Null();
}

Dart_Handle ParagraphBuilder::addPlaceholder(double width,
                                             double height,
                                             unsigned alignment,
                                             double baseline_offset,
                                             unsigned baseline) {
  txt::PlaceholderRun placeholder_run(
      width, height, static_cast<txt::PlaceholderAlignment>(alignment),
      static_cast<txt::TextBaseline>(baseline), baseline_offset);

  m_paragraphBuilder->AddPlaceholder(placeholder_run);

  return Dart_Null();
}

void ParagraphBuilder::build(Dart_Handle paragraph_handle) {
  Paragraph::Create(paragraph_handle, m_paragraphBuilder->Build());
}

}  // namespace flutter
