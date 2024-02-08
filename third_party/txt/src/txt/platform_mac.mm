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
using CTFontPtr =
    std::unique_ptr<std::remove_pointer<CTFontRef>::type, decltype(&CFRelease)>;
using CFMutableDictionaryPtr =
    std::unique_ptr<std::remove_pointer<CFMutableDictionaryRef>::type,
                    decltype(&CFRelease)>;
using CFNumberPtr = std::unique_ptr<std::remove_pointer<CFNumberRef>::type,
                                    decltype(&CFRelease)>;
using CTFontDescriptorPtr =
    std::unique_ptr<std::remove_pointer<CTFontDescriptorRef>::type,
                    decltype(&CFRelease)>;

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
  CTFontPtr ct_font(
      CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr),
      &CFRelease);

  if (desired_weight == kNormalWeightValue) {
    return (CTFontRef)CFRetain(ct_font.get());
  }

  CFMutableDictionaryPtr variations(
      CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks),
      &CFRelease);

  auto add_axis_to_variations = [&variations](const FourCharCode tag,
                                              float desired_value,
                                              float normal_value) {
    if (desired_value != normal_value) {
      CFNumberPtr tag_number(
          CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &tag),
          &CFRelease);
      CFNumberPtr value_number(
          CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType,
                         &desired_value),
          &CFRelease);
      CFDictionarySetValue(variations.get(), tag_number.get(),
                           value_number.get());
    }
  };
  add_axis_to_variations(kWeightTag, desired_weight, kNormalWeightValue);

  CFMutableDictionaryPtr attributes(
      CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks),
      &CFRelease);
  CFDictionarySetValue(attributes.get(), kCTFontVariationAttribute,
                       variations.get());

  CTFontDescriptorPtr var_font_desc(
      CTFontDescriptorCreateWithAttributes(attributes.get()), &CFRelease);

  return CTFontCreateCopyWithAttributes(ct_font.get(), size, nullptr,
                                        var_font_desc.get());
}

void RegisterSystemFonts(const DynamicFontManager& dynamic_font_manager) {
  // iOS loads different system fonts when size is greater than 28 or lower
  // than 17. The "familyName" property returned from CoreText stays the same
  // despite the typeface is different.
  //
  // Below code manually loads and registers the larger font. The existing
  // fallback correctly loads the smaller font. The code also iterates through
  // the possible font weights from 100 - 900 to correctly load all of them, as
  // a CTFont object for the large system font does not include all of the font
  // weights by default.
  //
  // Darwin system fonts from 17 to 28 also have dynamic spacing based on sizes.
  // These two fonts do not match the spacings when sizes are from 17 to 28.
  // The spacing should be handled by the app or the framework.
  //
  // See https://www.wwdcnotes.com/notes/wwdc20/10175/ for Apple's document on
  // this topic.
  auto register_weighted_font = [&dynamic_font_manager](const int weight) {
    sk_sp<SkTypeface> large_system_font_weighted =
        SkMakeTypefaceFromCTFont((CTFontRef)CFAutorelease(
            MatchSystemUIFont(weight, kSFProDisplayBreakPoint)));
    if (large_system_font_weighted) {
      dynamic_font_manager.font_provider().RegisterTypeface(
          large_system_font_weighted, kSFProDisplayName);
    }
  };
  for (int i = 0; i < 8; i++) {
    const int font_weight = i * 100;
    register_weighted_font(font_weight);
  }
  // The value 780 returns a font weight of 800.
  register_weighted_font(780);
  // The value of 810 returns a font weight of 900.
  register_weighted_font(810);
}

}  // namespace txt
