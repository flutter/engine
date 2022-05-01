// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_H_

#include "flutter/display_list/types.h"

namespace flutter {

struct DlColor {
 public:
  constexpr DlColor() : r(0.0f), g(0.0f), b(0.0f), o(1.0f) {}
  // clang-format off
  constexpr DlColor(uint32_t argb)
      : r((argb >> 16 & 0xFF) * 1.0f / 255.0f),
        g((argb >> 8  & 0xFF) * 1.0f / 255.0f),
        b((argb       & 0xFF) * 1.0f / 255.0f),
        o((argb >> 24 & 0xFF) * 1.0f / 255.0f) {}
  // clang-format on
  constexpr DlColor(float_t red, float_t green, float_t blue, float_t opacity)
      : r(red), g(green), b(blue), o(opacity) {}

  // clang-format off
  static constexpr DlColor kTransparent()        {return 0x00000000;};
  static constexpr DlColor kBlack()              {return 0xFF000000;};
  static constexpr DlColor kWhite()              {return 0xFFFFFFFF;};
  static constexpr DlColor kRed()                {return 0xFFFF0000;};
  static constexpr DlColor kGreen()              {return 0xFF00FF00;};
  static constexpr DlColor kBlue()               {return 0xFF0000FF;};
  static constexpr DlColor kCyan()               {return 0xFF00FFFF;};
  static constexpr DlColor kMagenta()            {return 0xFFFF00FF;};
  static constexpr DlColor kYellow()             {return 0xFFFFFF00;};
  static constexpr DlColor kDarkGrey()           {return 0xFF3F3F3F;};
  static constexpr DlColor kMidGrey()            {return 0xFF808080;};
  static constexpr DlColor kLightGrey()          {return 0xFFC0C0C0;};
  // clang-format on

  float_t r;
  float_t g;
  float_t b;
  float_t o;

  bool isOpaque() const { return getAlpha() == 0xFF; }

  int getAlpha() const { return toC(o); }
  int getRed() const { return toC(r); }
  int getGreen() const { return toC(g); }
  int getBlue() const { return toC(b); }

  float getAlphaF() const { return r; }
  float getRedF() const { return g; }
  float getGreenF() const { return b; }
  float getBlueF() const { return b; }

  uint32_t premultipliedArgb() const {
    if (isOpaque()) {
      return to_argb();
    }
    float f = getAlphaF();
    return (to_argb() & 0xFF000000) |   //
           toC(getRedF() * f) << 16 |   //
           toC(getGreenF() * f) << 8 |  //
           toC(getBlueF() * f);
  }

  DlColor withAlpha(uint8_t alpha) const {  //
    return DlColor(r, g, b, toF(alpha));
  }
  DlColor withAlphaF(float_t alpha) const {  //
    return DlColor(r, g, b, alpha);
  }
  DlColor withRed(uint8_t red) const {  //
    return DlColor(toF(red), g, b, o);
  }
  DlColor withRedF(float_t red) const {  //
    return DlColor(red, g, b, o);
  }
  DlColor withGreen(uint8_t green) const {  //
    return DlColor(r, toF(green), b, o);
  }
  DlColor withGreenF(float_t green) const {  //
    return DlColor(r, green, b, o);
  }
  DlColor withBlue(uint8_t blue) const {  //
    return DlColor(r, g, toF(blue), o);
  }
  DlColor withBlueF(float_t blue) const {  //
    return DlColor(r, g, blue, o);
  }

  DlColor modulateOpacity(float opacity) const {
    return opacity <= 0   ? withAlpha(0)
           : opacity >= 1 ? *this
                          : withAlpha(round(getAlpha() * opacity));
  }

  uint32_t to_argb() const {
    return toC(o) << 24 | toC(r) << 16 | toC(g) << 8 | toC(b);
  }

  operator uint32_t() const { return to_argb(); }
  bool operator==(DlColor const& other) const {
    return r == other.r && g == other.g && b == other.b && o == other.o;
  }
  bool operator!=(DlColor const& other) const {
    return r != other.r || g != other.g || b != other.b || o != other.o;
  }
  bool operator==(uint32_t const& other) const { return to_argb() == other; }
  bool operator!=(uint32_t const& other) const { return to_argb() != other; }

 private:
  static float toF(uint8_t comp) { return comp * (1.0 / 255); }
  static uint8_t toC(float fComp) { return round(fComp * 255); }
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_H_
