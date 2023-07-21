// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/skia/typeface_skia.h"

#include "flutter/fml/hash_combine.h"

namespace impeller {

TypefaceSkia::TypefaceSkia(SkFont font) : font_(std::move(font)) {}

TypefaceSkia::~TypefaceSkia() = default;

bool TypefaceSkia::IsValid() const {
  return true;
}

std::size_t TypefaceSkia::GetHash() const {
  if (!IsValid()) {
    return 0u;
  }

  size_t flags = fml::HashCombine(
      font_.isForceAutoHinting(), font_.isEmbeddedBitmaps(), font_.isSubpixel(),
      font_.isLinearMetrics(), font_.isEmbolden(), font_.isBaselineSnap());

  return fml::HashCombine(font_.getSize(), font_.getScaleX(), font_.getSkewX(),
                          flags, font_.getEdging(), font_.getHinting());
}

bool TypefaceSkia::IsEqual(const Typeface& other) const {
  auto sk_other = reinterpret_cast<const TypefaceSkia*>(&other);
  return sk_other->font_ == font_;
}

const SkFont& TypefaceSkia::GetSkiaFont() const {
  return font_;
}

}  // namespace impeller
