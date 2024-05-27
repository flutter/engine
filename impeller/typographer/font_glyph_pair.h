// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TYPOGRAPHER_FONT_GLYPH_PAIR_H_
#define FLUTTER_IMPELLER_TYPOGRAPHER_FONT_GLYPH_PAIR_H_

#include <unordered_map>
#include <unordered_set>

#include "impeller/geometry/color.h"
#include "impeller/typographer/font.h"
#include "impeller/typographer/glyph.h"

namespace impeller {

enum class SubpixelPosition : uint8_t {
  kZero,   // 0
  kOne,    // 0.25
  kTwo,    // 0.5
  kThree,  // 0.75
};

//------------------------------------------------------------------------------
/// @brief      A font and a scale.  Used as a key that represents a typeface
///             within a glyph atlas.
///
struct ScaledFont {
  Font font;
  Scalar scale;
  Color color;
};

//------------------------------------------------------------------------------
/// @brief      A glyph and its subpixel position.
///
struct SubpixelGlyph {
  Glyph glyph;
  SubpixelPosition subpixel;

  SubpixelGlyph(Glyph p_glyph, SubpixelPosition p_subpixel)
      : glyph(p_glyph), subpixel(p_subpixel) {}
};

using FontGlyphMap =
    std::unordered_map<ScaledFont, std::unordered_set<SubpixelGlyph>>;

//------------------------------------------------------------------------------
/// @brief      A font along with a glyph in that font rendered at a particular
///             scale and subpixel position.
///
struct FontGlyphPair {
  FontGlyphPair(const ScaledFont& sf, const SubpixelGlyph& g)
      : scaled_font(sf), glyph(g) {}
  const ScaledFont& scaled_font;
  const SubpixelGlyph& glyph;
};

}  // namespace impeller

template <>
struct std::hash<impeller::ScaledFont> {
  constexpr std::size_t operator()(const impeller::ScaledFont& sf) const {
    return fml::HashCombine(sf.font.GetHash(), sf.scale, sf.color.ToARGB());
  }
};

template <>
struct std::equal_to<impeller::ScaledFont> {
  constexpr bool operator()(const impeller::ScaledFont& lhs,
                            const impeller::ScaledFont& rhs) const {
    return lhs.font.IsEqual(rhs.font) && lhs.scale == rhs.scale &&
           lhs.color == rhs.color;
  }
};

template <>
struct std::hash<impeller::SubpixelGlyph> {
  constexpr std::size_t operator()(const impeller::SubpixelGlyph& sg) const {
    return fml::HashCombine(sg.glyph.index, sg.subpixel);
  }
};

template <>
struct std::equal_to<impeller::SubpixelGlyph> {
  constexpr bool operator()(const impeller::SubpixelGlyph& lhs,
                            const impeller::SubpixelGlyph& rhs) const {
    return lhs.glyph.index == rhs.glyph.index &&
           lhs.glyph.type == rhs.glyph.type && lhs.subpixel == rhs.subpixel;
  }
};

#endif  // FLUTTER_IMPELLER_TYPOGRAPHER_FONT_GLYPH_PAIR_H_
