// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <TargetConditionals.h>

#include "flutter/fml/platform/darwin/platform_version.h"
#include "third_party/skia/include/ports/SkFontMgr_mac_ct.h"
#include "third_party/skia/include/ports/SkTypeface_mac.h"
#include "txt/platform.h"
#include "txt/platform_mac.h"

#if TARGET_OS_EMBEDDED || TARGET_OS_SIMULATOR
#include <UIKit/UIKit.h>
#define FONT_CLASS UIFont
#else  // TARGET_OS_EMBEDDED
#include <AppKit/AppKit.h>
#define FONT_CLASS NSFont
#endif  // TARGET_OS_EMBEDDED

// Apple system font larger than size 29 returns SFProDisplay typeface.
static const CGFloat kSFProDisplayBreakPoint = 29;
// Font name represents the "SF Pro Display" system font on Apple platforms.
static const std::string kSFProDisplayName = "CupertinoSystemDisplay";
// Font weight representing Regular
float kNormalWeightValue = 400;

namespace txt {

const FourCharCode kWeightTag = 'wght';

std::vector<std::string> GetDefaultFontFamilies() {
  if (fml::IsPlatformVersionAtLeast(9)) {
    return {[FONT_CLASS systemFontOfSize:14].familyName.UTF8String};
  } else {
    return {"Helvetica"};
  }
}

sk_sp<SkFontMgr> GetDefaultFontManager(uint32_t font_initialization_data) {
  static sk_sp<SkFontMgr> mgr = SkFontMgr_New_CoreText(nullptr);
  return mgr;
}

CTFontRef MatchSystemUIFont(float desired_weight, float size) {
  CTFontRef ct_font(
      CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr));

  if (desired_weight == kNormalWeightValue) {
    return ct_font;
  }

  CFMutableDictionaryRef variations(CFDictionaryCreateMutable(
      kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks));

  auto add_axis_to_variations = [&variations](const FourCharCode tag,
                                              float desired_value,
                                              float normal_value) {
    if (desired_value != normal_value) {
      CFNumberRef tag_number(
          CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &tag));
      CFNumberRef value_number(CFNumberCreate(
          kCFAllocatorDefault, kCFNumberFloatType, &desired_value));
      CFDictionarySetValue(variations, tag_number, value_number);
    }
  };
  add_axis_to_variations(kWeightTag, desired_weight, kNormalWeightValue);

  CFMutableDictionaryRef attributes(CFDictionaryCreateMutable(
      kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks));
  CFDictionarySetValue(attributes, kCTFontVariationAttribute, variations);

  CTFontDescriptorRef var_font_desc(
      CTFontDescriptorCreateWithAttributes(attributes));

  return CTFontCreateCopyWithAttributes(ct_font, size, nullptr, var_font_desc);
}

void RegisterSystemFonts(const DynamicFontManager& dynamic_font_manager) {
  // iOS loads different system fonts when size is greater than 28 or lower
  // than 17. The "familyName" property returned from CoreText stays the same
  // despite the typeface is different.
  //
  // Below code manually loads and registers the larger font. The existing fallback
  // correctly loads the smaller font. The code also iterates through the possible
  // font weights from 100 - 900 to correctly load all of them, as a CTFont object
  // for the large system font does not include all of the font weights by default.
  //
  // Darwin system fonts from 17 to 28 also have dynamic spacing based on sizes.
  // These two fonts do not match the spacings when sizes are from 17 to 28.
  // The spacing should be handled by the app or the framework.
  //
  // See https://www.wwdcnotes.com/notes/wwdc20/10175/ for Apple's document on
  // this topic.
  for (int i = 0; i < 8; i++) {
    const int font_weight = i * 100;
    sk_sp<SkTypeface> large_system_font_weighted =
        SkMakeTypefaceFromCTFont((CTFontRef)CFAutorelease(
            MatchSystemUIFont(font_weight, kSFProDisplayBreakPoint)));
    if (large_system_font_weighted) {
      dynamic_font_manager.font_provider().RegisterTypeface(
          large_system_font_weighted, kSFProDisplayName);
    }
  }
  // The value 780 returns a font weight of 800.
  sk_sp<SkTypeface> large_system_font_weighted_800 =
        SkMakeTypefaceFromCTFont((CTFontRef)CFAutorelease(
            MatchSystemUIFont(780, kSFProDisplayBreakPoint)));
    if (large_system_font_weighted_800) {
      dynamic_font_manager.font_provider().RegisterTypeface(
          large_system_font_weighted_800, kSFProDisplayName);
    }
  // The value of 810 returns a font weight of 900.
  sk_sp<SkTypeface> large_system_font_weighted_900 =
        SkMakeTypefaceFromCTFont((CTFontRef)CFAutorelease(
            MatchSystemUIFont(810, kSFProDisplayBreakPoint)));
    if (large_system_font_weighted_900) {
      dynamic_font_manager.font_provider().RegisterTypeface(
          large_system_font_weighted_900, kSFProDisplayName);
    }
  }

}  // namespace txt
