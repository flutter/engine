// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DL_COLOR_H_
#define FLUTTER_DISPLAY_LIST_DL_COLOR_H_

#include <algorithm>
#include "third_party/skia/include/core/SkScalar.h"

namespace flutter {

struct DlColor {
 public:
  constexpr DlColor() : DlColor(0xFF000000) {}
  constexpr explicit DlColor(uint32_t argb)
      : alpha_(((argb >> 24) & 0xFF) / 255.0f),
        red_(((argb >> 16) & 0xFF) / 255.0f),
        green_(((argb >> 8) & 0xFF) / 255.0f),
        blue_(((argb >> 0) & 0xFF) / 255.0f) {}
  constexpr explicit DlColor(SkScalar a, SkScalar r, SkScalar g, SkScalar b)
      : alpha_(a), red_(r), green_(g), blue_(b) {}

  static constexpr uint8_t toAlpha(SkScalar opacity) { return toC(opacity); }
  static constexpr SkScalar toOpacity(uint8_t alpha) { return toF(alpha); }

  // clang-format off
  static constexpr DlColor kTransparent()        {return DlColor(0x00000000);};
  static constexpr DlColor kBlack()              {return DlColor(0xFF000000);};
  static constexpr DlColor kWhite()              {return DlColor(0xFFFFFFFF);};
  static constexpr DlColor kRed()                {return DlColor(0xFFFF0000);};
  static constexpr DlColor kGreen()              {return DlColor(0xFF00FF00);};
  static constexpr DlColor kBlue()               {return DlColor(0xFF0000FF);};
  static constexpr DlColor kCyan()               {return DlColor(0xFF00FFFF);};
  static constexpr DlColor kMagenta()            {return DlColor(0xFFFF00FF);};
  static constexpr DlColor kYellow()             {return DlColor(0xFFFFFF00);};
  static constexpr DlColor kDarkGrey()           {return DlColor(0xFF3F3F3F);};
  static constexpr DlColor kMidGrey()            {return DlColor(0xFF808080);};
  static constexpr DlColor kLightGrey()          {return DlColor(0xFFC0C0C0);};
  static constexpr DlColor kAliceBlue()          {return DlColor(0xFFF0F8FF);};
  static constexpr DlColor kFuchsia()            {return DlColor(0xFFFF00FF);};
  static constexpr DlColor kMaroon()             {return DlColor(0xFF800000);}
  static constexpr DlColor kSkyBlue()            {return DlColor(0xFF87CEEB);}
  // clang-format on

  constexpr bool isOpaque() const { return getAlpha() == 0xFF; }
  constexpr bool isTransparent() const { return getAlpha() == 0; }

  // These getters clamp the value to the range [0, 256).
  constexpr int getAlpha() const { return toC(getAlphaF()); }
  constexpr int getRed() const { return toC(getRedF()); }
  constexpr int getGreen() const { return toC(getGreenF()); }
  constexpr int getBlue() const { return toC(getBlueF()); }

  constexpr float getAlphaF() const { return alpha_; }
  constexpr float getRedF() const { return red_; }
  constexpr float getGreenF() const { return green_; }
  constexpr float getBlueF() const { return blue_; }

  constexpr DlColor withAlpha(uint8_t alpha) const {  //
    return DlColor(alpha / 255.0f, red_, green_, blue_);
  }
  constexpr DlColor withRed(uint8_t red) const {  //
    return DlColor(alpha_, red / 255.0f, green_, blue_);
  }
  constexpr DlColor withGreen(uint8_t green) const {  //
    return DlColor(alpha_, red_, green / 255.0f, blue_);
  }
  constexpr DlColor withBlue(uint8_t blue) const {  //
    return DlColor(alpha_, red_, green_, blue / 255.0f);
  }

  constexpr DlColor modulateOpacity(float opacity) const {
    return opacity <= 0   ? withAlpha(0)
           : opacity >= 1 ? *this
                          : withAlpha(round(getAlpha() * opacity));
  }

  constexpr uint32_t argb() const {
    return (((std::lround(alpha_ * 255.0f) & 0xff) << 24) |
            ((std::lround(red_ * 255.0f) & 0xff) << 16) |
            ((std::lround(green_ * 255.0f) & 0xff) << 8) |
            ((std::lround(blue_ * 255.0f) & 0xff) << 0)) &
           0xFFFFFFFF;
  }

  constexpr uint32_t premultipliedArgb() const {
    if (isOpaque()) {
      return argb();
    }
    float f = getAlphaF();
    return (argb() & 0xFF000000) |      //
           toC(getRedF() * f) << 16 |   //
           toC(getGreenF() * f) << 8 |  //
           toC(getBlueF() * f);
  }

  bool operator==(DlColor const& other) const {
    return SkScalarNearlyEqual(red_, other.red_) &&
           SkScalarNearlyEqual(green_, other.green_) &&
           SkScalarNearlyEqual(blue_, other.blue_) &&
           SkScalarNearlyEqual(alpha_, other.alpha_);
  }
  bool operator!=(DlColor const& other) const { return !(*this == other); }
  bool operator==(uint32_t const& other) const {
    return *this == DlColor(other);
  }
  bool operator!=(uint32_t const& other) const {
    return *this != DlColor(other);
  }

 private:
  SkScalar alpha_ = 0.0;
  SkScalar red_ = 0.0;
  SkScalar green_ = 0.0;
  SkScalar blue_ = 0.0;

  static float toF(uint8_t comp) { return comp * (1.0f / 255); }
  static uint8_t toC(float fComp) { return round(fComp * 255); }
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DL_COLOR_H_
