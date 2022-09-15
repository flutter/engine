// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/skia/typeface_skia.h"

namespace impeller {

TypefaceSkia::TypefaceSkia(sk_sp<SkTypeface> typeface)
    : typeface_(std::move(typeface)) {}

TypefaceSkia::~TypefaceSkia() = default;

bool TypefaceSkia::IsValid() const {
  return !!typeface_;
}

std::size_t TypefaceSkia::GetHash() const {
  if (!IsValid()) {
    return 0u;
  }

  return reinterpret_cast<size_t>(typeface_.get());
}

bool TypefaceSkia::IsEqual(const Typeface& other) const {
  auto sk_other = reinterpret_cast<const TypefaceSkia*>(&other);
  return sk_other->typeface_ == typeface_;
}

Rect TypefaceSkia::GetBoundingBox() const {
  if (!IsValid()) {
    return {};
  }

  const auto bounds = typeface_->getBounds();

  return Rect::MakeLTRB(bounds.left(), bounds.top(), bounds.right(),
                        bounds.bottom());
}

static const SkFontTableTag kCOLRTableTag =
    SkSetFourByteTag('C', 'O', 'L', 'R');

bool TypefaceSkia::HasColor() const {
  auto count = typeface_->countTables();
  if (count == 0) {
    return false;
  }
  std::vector<SkFontTableTag> tags(count);
  typeface_->getTableTags(tags.data());
  for (auto tag : tags) {
    if (tag == kCOLRTableTag) {
      return true;
    }
  }
  return false;
}

const sk_sp<SkTypeface>& TypefaceSkia::GetSkiaTypeface() const {
  return typeface_;
}

}  // namespace impeller
