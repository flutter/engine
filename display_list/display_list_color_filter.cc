// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_color_filter.h"

namespace flutter {

DlColorFilter::~DlColorFilter() {
  if (type_ == kUnknown) {
    delete static_cast<const DlUnknownColorFilter*>(this);
    type_ = kNone;
  }
}

size_t DlColorFilter::size() const {
  switch (type_) {
    case kNone:
      // This query is used for allocating raw storage within a buffer for
      // storing these objects. Such a technique should not be used for
      // the |kNone| version of these objects.
      FML_DCHECK(false);
      return 0;
    case kBlend:
      return static_cast<const DlBlendColorFilter*>(this)->size();
    case kMatrix:
      return static_cast<const DlMatrixColorFilter*>(this)->size();
    case kSrgbToLinearGamma:
      return static_cast<const DlSrgbToLinearGammaColorFilter*>(this)->size();
    case kLinearToSrgbGamma:
      return static_cast<const DlLinearToSrgbGammaColorFilter*>(this)->size();
    case kUnknown:
      return static_cast<const DlUnknownColorFilter*>(this)->size();
  }
}

bool DlColorFilter::equals(const DlColorFilter* other) const {
  return Equals(this, other);
}

bool DlColorFilter::Equals(const DlColorFilter* a, const DlColorFilter* b) {
  if (a == b) {
    return true;
  }
  if (a == nullptr || b == nullptr) {
    return false;
  }
  if (a->type_ != b->type_) {
    return false;
  }
  switch (a->type_) {
    case kNone:
      return true;
    case kBlend:
      return static_cast<const DlBlendColorFilter*>(a)->equals(
        static_cast<const DlBlendColorFilter*>(b)
      );
    case kMatrix:
      return static_cast<const DlMatrixColorFilter*>(a)->equals(
        static_cast<const DlMatrixColorFilter*>(b)
      );
    case kSrgbToLinearGamma:
    case kLinearToSrgbGamma:
      return true;
    case kUnknown:
      return static_cast<const DlUnknownColorFilter*>(a)->equals(
        static_cast<const DlUnknownColorFilter*>(b)
      );
  }
}

sk_sp<SkColorFilter> DlColorFilter::sk_filter() const {
  switch (type_) {
    case kNone:
      return nullptr;
    case kBlend:
      return static_cast<const DlBlendColorFilter*>(this)->sk_filter();
    case kMatrix:
      return static_cast<const DlMatrixColorFilter*>(this)->sk_filter();
    case kSrgbToLinearGamma:
      return static_cast<const DlSrgbToLinearGammaColorFilter*>(this)->sk_filter();
    case kLinearToSrgbGamma:
      return static_cast<const DlLinearToSrgbGammaColorFilter*>(this)->sk_filter();
    case kUnknown:
      return static_cast<const DlUnknownColorFilter*>(this)->sk_filter();
  }
}

std::shared_ptr<const DlColorFilter> DlColorFilter::shared() const {
  switch (type_) {
    case kNone:
      return std::make_shared<DlNoColorFilter>();
    case kBlend:
      return std::make_shared<DlBlendColorFilter>(static_cast<const DlBlendColorFilter*>(this));
    case kMatrix:
      return std::make_shared<DlMatrixColorFilter>(static_cast<const DlMatrixColorFilter*>(this));
    case kSrgbToLinearGamma:
      return std::make_shared<DlSrgbToLinearGammaColorFilter>(static_cast<const DlSrgbToLinearGammaColorFilter*>(this));
    case kLinearToSrgbGamma:
      return std::make_shared<DlLinearToSrgbGammaColorFilter>(static_cast<const DlLinearToSrgbGammaColorFilter*>(this));
    case kUnknown:
      return std::make_shared<DlUnknownColorFilter>(static_cast<const DlUnknownColorFilter*>(this));
  }
}

bool DlColorFilter::modifies_transparent_black() const {
  switch (type_) {
    case kNone:
    case kSrgbToLinearGamma:
    case kLinearToSrgbGamma:
      return false;
    case kBlend:
      // Look at blend and color to make a faster determination?
    case kMatrix:
      // Look at the matrix to make a faster determination?
      // Basically, are the translation components all 0?
    case kUnknown:
      return sk_filter()->filterColor(SK_ColorTRANSPARENT) != SK_ColorTRANSPARENT;
  }
}

const DlNoColorFilter DlNoColorFilter::instance = DlNoColorFilter();

const DlSrgbToLinearGammaColorFilter DlSrgbToLinearGammaColorFilter::instance =
    DlSrgbToLinearGammaColorFilter();
const sk_sp<SkColorFilter> DlSrgbToLinearGammaColorFilter::sk_filter_ =
    SkColorFilters::SRGBToLinearGamma();

const DlLinearToSrgbGammaColorFilter DlLinearToSrgbGammaColorFilter::instance =
    DlLinearToSrgbGammaColorFilter();
const sk_sp<SkColorFilter> DlLinearToSrgbGammaColorFilter::sk_filter_ =
    SkColorFilters::LinearToSRGBGamma();

}  // namespace flutter
