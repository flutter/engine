/*
 * Copyright 2017 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "flutter/fml/command_line.h"
#include "flutter/fml/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "third_party/skia/src/core/SkAdvancedTypefaceMetrics.h"
#include "third_party/skia/src/core/SkScalerContext.h"
#include "txt/font_collection.h"
#include "txt_test_utils.h"

namespace txt {

// We don't really need a fixture but a class in a namespace is needed for
// the FRIEND_TEST macro.
class FontCollectionTest : public ::testing::Test {};

class MockSkTypeface : public SkTypeface {
  public:
  MockSkTypeface();
  MOCK_CONST_METHOD1(onMakeClone, sk_sp<SkTypeface>(const SkFontArguments&));
  MOCK_CONST_METHOD2(onCreateScalerContext,
                     SkScalerContext*(const SkScalerContextEffects&,
                                      const SkDescriptor*));
  MOCK_CONST_METHOD1(onFilterRec, void(SkScalerContextRec*));
  MOCK_CONST_METHOD0(onGetAdvancedMetrics,
                     std::unique_ptr<SkAdvancedTypefaceMetrics>());
  MOCK_CONST_METHOD1(getPostScriptGlyphNames, void(SkString*));
  MOCK_CONST_METHOD1(getGlyphToUnicodeMap, void(SkUnichar* dstArray));
  MOCK_CONST_METHOD1(onOpenStream,
                     std::unique_ptr<SkStreamAsset>(int* ttcIndex));
  MOCK_CONST_METHOD2(
      onGetVariationDesignPosition,
      int(SkFontArguments::VariationPosition::Coordinate coordinates[],
          int coordinateCount));
  MOCK_CONST_METHOD2(onGetVariationDesignParameters,
                     int(SkFontParameters::Variation::Axis parameters[],
                         int parameterCount));
  MOCK_CONST_METHOD2(onGetFontDescriptor,
                     void(SkFontDescriptor*, bool* isLocal));
  MOCK_CONST_METHOD3(onCharsToGlyphs,
                     void(const SkUnichar* chars,
                          int count,
                          SkGlyphID glyphs[]));
  MOCK_CONST_METHOD0(onCountGlyphs, int());
  MOCK_CONST_METHOD0(onGetUPEM, int());
  MOCK_CONST_METHOD3(onGetKerningPairAdjustments,
                     bool(const SkGlyphID glyphs[],
                          int count,
                          int32_t adjustments[]));
  MOCK_CONST_METHOD1(onGetFamilyName, void(SkString* familyName));
  MOCK_CONST_METHOD0(onCreateFamilyNameIterator, SkTypeface::LocalizedStrings*());
  MOCK_CONST_METHOD1(onGetTableTags, int(SkFontTableTag tags[]));
  MOCK_CONST_METHOD4(
      onGetTableData,
      size_t(SkFontTableTag, size_t offset, size_t length, void* data));
  MOCK_CONST_METHOD1(onCopyTableData, sk_sp<SkData>(SkFontTableTag));
  MOCK_CONST_METHOD1(onComputeBounds, bool(SkRect*));
  MOCK_CONST_METHOD0(onGetCTFontRef, void*());
};

TEST(FontCollectionTest, CheckSkTypefacesSorting) {
  MockSkTypeface mockTypeface1;

  sk_sp<SkTypeface> typeface1{SkTypeface::MakeFromName(
      "Arial",
      SkFontStyle(SkFontStyle::kThin_Weight, SkFontStyle::kExpanded_Width,
                  SkFontStyle::kItalic_Slant))};
  sk_sp<SkTypeface> typeface2{SkTypeface::MakeFromName(
      "Arial",
      SkFontStyle(SkFontStyle::kLight_Weight, SkFontStyle::kNormal_Width,
                  SkFontStyle::kUpright_Slant))};
  sk_sp<SkTypeface> typeface3{SkTypeface::MakeFromName(
      "Arial",
      SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                  SkFontStyle::kUpright_Slant))};
  std::vector<sk_sp<SkTypeface>> candidateTypefaces{typeface1, typeface2,
                                                    typeface3};

  ASSERT_EQ(candidateTypefaces[0]->fontStyle().weight(),
            SkFontStyle::kThin_Weight);
  ASSERT_EQ(candidateTypefaces[0]->fontStyle().width(),
            SkFontStyle::kExpanded_Width);

  ASSERT_EQ(candidateTypefaces[1]->fontStyle().weight(),
            SkFontStyle::kLight_Weight);
  ASSERT_EQ(candidateTypefaces[1]->fontStyle().width(),
            SkFontStyle::kNormal_Width);

  ASSERT_EQ(candidateTypefaces[2]->fontStyle().weight(),
            SkFontStyle::kNormal_Weight);
  ASSERT_EQ(candidateTypefaces[2]->fontStyle().width(),
            SkFontStyle::kNormal_Width);

  // This sorts the vector in-place.
  txt::FontCollection::SortSkTypefaces(candidateTypefaces);
  ASSERT_EQ(candidateTypefaces[0].get(), typeface1.get());
  ASSERT_EQ(candidateTypefaces[1].get(), typeface2.get());

  // ASSERT_EQ(candidateTypefaces[0]->fontStyle().weight(),
  // SkFontStyle::kLight_Weight);
  ASSERT_EQ(candidateTypefaces[0]->fontStyle().width(),
            SkFontStyle::kNormal_Width);

  ASSERT_EQ(candidateTypefaces[1]->fontStyle().weight(),
            SkFontStyle::kNormal_Weight);
  ASSERT_EQ(candidateTypefaces[1]->fontStyle().width(),
            SkFontStyle::kNormal_Width);

  ASSERT_EQ(candidateTypefaces[2]->fontStyle().weight(),
            SkFontStyle::kThin_Weight);
  ASSERT_EQ(candidateTypefaces[2]->fontStyle().width(),
            SkFontStyle::kExpanded_Width);
}

#if 0

TEST(FontCollection, HasDefaultRegistrations) {
  std::string defaultFamilyName = txt::FontCollection::GetDefaultFamilyName();

  auto collection = txt::FontCollection::GetFontCollection(txt::GetFontDir())
                        .GetMinikinFontCollectionForFamily("");
  ASSERT_EQ(defaultFamilyName,
            txt::FontCollection::GetFontCollection(txt::GetFontDir())
                .ProcessFamilyName(""));
  ASSERT_NE(defaultFamilyName,
            txt::FontCollection::GetFontCollection(txt::GetFontDir())
                .ProcessFamilyName("NotARealFont!"));
  ASSERT_EQ("NotARealFont!",
            txt::FontCollection::GetFontCollection(txt::GetFontDir())
                .ProcessFamilyName("NotARealFont!"));
  ASSERT_NE(collection.get(), nullptr);
}

TEST(FontCollection, GetMinikinFontCollections) {
  std::string defaultFamilyName = txt::FontCollection::GetDefaultFamilyName();

  auto collectionDef = txt::FontCollection::GetFontCollection(txt::GetFontDir())
                           .GetMinikinFontCollectionForFamily("");
  auto collectionRoboto =
      txt::FontCollection::GetFontCollection(txt::GetFontDir())
          .GetMinikinFontCollectionForFamily("Roboto");
  auto collectionHomemadeApple =
      txt::FontCollection::GetFontCollection(txt::GetFontDir())
          .GetMinikinFontCollectionForFamily("Homemade Apple");
  for (size_t base = 0; base < 50; base++) {
    for (size_t variation = 0; variation < 50; variation++) {
      ASSERT_EQ(collectionDef->hasVariationSelector(base, variation),
                collectionRoboto->hasVariationSelector(base, variation));
    }
  }

  ASSERT_NE(collectionDef, collectionHomemadeApple);
  ASSERT_NE(collectionHomemadeApple, collectionRoboto);
  ASSERT_NE(collectionDef.get(), nullptr);
}

TEST(FontCollection, GetFamilyNames) {
  std::set<std::string> names =
      txt::FontCollection::GetFontCollection(txt::GetFontDir())
          .GetFamilyNames();

  ASSERT_TRUE(names.size() >= 19ull);

  ASSERT_EQ(names.count("Roboto"), 1ull);
  ASSERT_EQ(names.count("Homemade Apple"), 1ull);

  ASSERT_EQ(names.count("KoreanFont Test"), 1ull);
  ASSERT_EQ(names.count("JapaneseFont Test"), 1ull);
  ASSERT_EQ(names.count("EmojiFont Test"), 1ull);
  ASSERT_EQ(names.count("ItalicFont Test"), 1ull);
  ASSERT_EQ(names.count("VariationSelector Test"), 1ull);
  ASSERT_EQ(names.count("ColorEmojiFont Test"), 1ull);
  ASSERT_EQ(names.count("TraditionalChinese Test"), 1ull);
  ASSERT_EQ(names.count("Sample Font"), 1ull);
  ASSERT_EQ(names.count("MultiAxisFont Test"), 1ull);
  ASSERT_EQ(names.count("TextEmojiFont Test"), 1ull);
  ASSERT_EQ(names.count("No Cmap Format 14 Subtable Test"), 1ull);
  ASSERT_EQ(names.count("ColorTextMixedEmojiFont Test"), 1ull);
  ASSERT_EQ(names.count("BoldFont Test"), 1ull);
  ASSERT_EQ(names.count("EmptyFont Test"), 1ull);
  ASSERT_EQ(names.count("SimplifiedChinese Test"), 1ull);
  ASSERT_EQ(names.count("BoldItalicFont Test"), 1ull);
  ASSERT_EQ(names.count("RegularFont Test"), 1ull);

  ASSERT_EQ(names.count("Not a real font!"), 0ull);
  ASSERT_EQ(names.count(""), 0ull);
  ASSERT_EQ(names.count("Another Fake Font"), 0ull);
}

#endif  // 0

}  // namespace txt
