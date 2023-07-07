// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/font.h"

namespace impeller {

Font::Font(std::shared_ptr<Typeface> typeface, Metrics metrics)
    : typeface_(std::move(typeface)), metrics_(metrics) {
  if (!typeface_) {
    return;
  }
  is_valid_ = true;
}

Font::~Font() = default;

bool Font::IsValid() const {
  return is_valid_;
}

const std::shared_ptr<Typeface>& Font::GetTypeface() const {
  return typeface_;
}

std::size_t Font::GetHash() const {
  return fml::HashCombine(is_valid_, typeface_ ? typeface_->GetHash() : 0u,
                          metrics_);
}

bool Font::IsEqual(const Font& other) const {
  return DeepComparePointer(typeface_, other.typeface_) &&
         is_valid_ == other.is_valid_ && metrics_ == other.metrics_;
}

const Font::Metrics& Font::GetMetrics() const {
  return metrics_;
}

Font::Metrics Font::Metrics::Scaled(Scalar new_scale) const {
  Font::Metrics metrics;
  metrics.scale = scale * new_scale;
  metrics.point_size = point_size;
  metrics.embolden = embolden;
  metrics.skewX = skewX;
  metrics.scaleX = scaleX;  // This is an actual font property, not a canvas
                            // transform property.
  return metrics;
}

Font Font::Scaled(Scalar scale) const {
  return Font(typeface_, metrics_.Scaled(scale));
}

}  // namespace impeller
