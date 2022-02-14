// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_FILTER_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_FILTER_H_

#include "flutter/display_list/types.h"
#include "flutter/fml/logging.h"

namespace flutter {

class DlBlendColorFilter;
class DlMatrixColorFilter;
class DlSrgbToLinearGammaColorFilter;
class DlLinearToSrgbGammaColorFilter;
class DlUnknownColorFilter;

class DlColorFilter {
 public:
  enum Type {
    kNone,
    kBlend,
    kMatrix,
    kSrgbToLinearGamma,
    kLinearToSrgbGamma,
    kUnknown
  };

  Type type() const { return type_; }

  static DlColorFilter From(SkColorFilter* sk_filter);

  size_t size() const;
  bool equals(const DlColorFilter* other) const;
  static bool Equals(const DlColorFilter* a, const DlColorFilter* b);
  sk_sp<SkColorFilter> sk_filter() const;
  std::shared_ptr<const DlColorFilter> shared() const;
  bool modifies_transparent_black() const;

  const DlBlendColorFilter* asABlendFilter() const {
    return type_ == kBlend ? reinterpret_cast<const DlBlendColorFilter*>(this)
                           : nullptr;
  }

  const DlMatrixColorFilter* asAMatrixFilter() const {
    return type_ == kMatrix ? reinterpret_cast<const DlMatrixColorFilter*>(this)
                            : nullptr;
  }

  const DlSrgbToLinearGammaColorFilter* asASrgbToLinearFilter() const {
    return type_ == kSrgbToLinearGamma
               ? reinterpret_cast<const DlSrgbToLinearGammaColorFilter*>(this)
               : nullptr;
  }

  const DlLinearToSrgbGammaColorFilter* asALinearToSrgbFilter() const {
    return type_ == kLinearToSrgbGamma
               ? reinterpret_cast<const DlLinearToSrgbGammaColorFilter*>(this)
               : nullptr;
  }

  ~DlColorFilter();

 protected:
  DlColorFilter(Type type) : type_(type) {
    FML_DCHECK(type >= kNone && type <= kUnknown);
  }

 private:
  Type type_;
};

class DlNoColorFilter final : public DlColorFilter {
 public:
  static const DlNoColorFilter instance;

  DlNoColorFilter() : DlColorFilter(kNone) {}
  DlNoColorFilter(const DlNoColorFilter& filter) : DlColorFilter(kNone) {}
  DlNoColorFilter(const DlNoColorFilter* filter) : DlColorFilter(kNone) {}

  size_t size() const { return sizeof(*this); }
  bool equals(const DlNoColorFilter* other) const { return true; }

  sk_sp<SkColorFilter> sk_filter() const { return nullptr; }
};

class DlBlendColorFilter final : public DlColorFilter {
 public:
  DlBlendColorFilter(SkColor color, SkBlendMode mode)
      : DlColorFilter(kBlend), color_(color), mode_(mode) {}
  DlBlendColorFilter(const DlBlendColorFilter& filter)
      : DlBlendColorFilter(filter.color_, filter.mode_) {}
  DlBlendColorFilter(const DlBlendColorFilter* filter)
      : DlBlendColorFilter(filter->color_, filter->mode_) {}

  size_t size() const { return sizeof(*this); }
  bool equals(const DlBlendColorFilter* other) const {
    return color_ == other->color_ && mode_ == other->mode_;
  }

  SkColor color() const { return color_; }
  SkBlendMode mode() const { return mode_; }

  sk_sp<SkColorFilter> sk_filter() const {
    return SkColorFilters::Blend(color_, mode_);
  }

 private:
  SkColor color_;
  SkBlendMode mode_;
};

class DlMatrixColorFilter final : public DlColorFilter {
 public:
  DlMatrixColorFilter(const float matrix[20]) : DlColorFilter(kMatrix) {
    memcpy(matrix_, &matrix, sizeof(matrix_));
  }
  DlMatrixColorFilter(const DlMatrixColorFilter& filter)
      : DlMatrixColorFilter(filter.matrix_) {}
  DlMatrixColorFilter(const DlMatrixColorFilter* filter)
      : DlMatrixColorFilter(filter->matrix_) {}

  size_t size() const { return sizeof(*this); }
  bool equals(const DlMatrixColorFilter* other) const {
    return memcmp(matrix_, other->matrix_, sizeof(matrix_)) == 0;
  }

  const float& operator[](int index) const { return matrix_[index]; }
  void get_matrix(float matrix[20]) const {
    memcpy(matrix, matrix_, sizeof(matrix_));
  }

  sk_sp<SkColorFilter> sk_filter() const {
    return SkColorFilters::Matrix(matrix_);
  }

 private:
  float matrix_[20];
};

class DlSrgbToLinearGammaColorFilter final : public DlColorFilter {
 public:
  static const DlSrgbToLinearGammaColorFilter instance;

  DlSrgbToLinearGammaColorFilter() : DlColorFilter(kSrgbToLinearGamma) {}
  DlSrgbToLinearGammaColorFilter(const DlSrgbToLinearGammaColorFilter& filter)
      : DlSrgbToLinearGammaColorFilter() {}
  DlSrgbToLinearGammaColorFilter(const DlSrgbToLinearGammaColorFilter* filter)
      : DlSrgbToLinearGammaColorFilter() {}

  size_t size() const { return sizeof(*this); }
  bool equals(const DlSrgbToLinearGammaColorFilter* other) const {
    return true;
  }

  sk_sp<SkColorFilter> sk_filter() const { return sk_filter_; }

 private:
  static const sk_sp<SkColorFilter> sk_filter_;
};

class DlLinearToSrgbGammaColorFilter final : public DlColorFilter {
 public:
  static const DlLinearToSrgbGammaColorFilter instance;

  DlLinearToSrgbGammaColorFilter() : DlColorFilter(kLinearToSrgbGamma) {}
  DlLinearToSrgbGammaColorFilter(const DlLinearToSrgbGammaColorFilter& filter)
      : DlLinearToSrgbGammaColorFilter() {}
  DlLinearToSrgbGammaColorFilter(const DlLinearToSrgbGammaColorFilter* filter)
      : DlLinearToSrgbGammaColorFilter() {}

  size_t size() const { return sizeof(*this); }
  bool equals(const DlLinearToSrgbGammaColorFilter* other) const {
    return true;
  }

  sk_sp<SkColorFilter> sk_filter() const { return sk_filter_; }

 private:
  static const sk_sp<SkColorFilter> sk_filter_;
};

class DlUnknownColorFilter final : public DlColorFilter {
 public:
  DlUnknownColorFilter(sk_sp<SkColorFilter> sk_filter)
      : DlColorFilter(kUnknown), sk_filter_(std::move(sk_filter)) {}
  DlUnknownColorFilter(const DlUnknownColorFilter& filter)
      : DlUnknownColorFilter(filter.sk_filter_) {}
  DlUnknownColorFilter(const DlUnknownColorFilter* filter)
      : DlUnknownColorFilter(filter->sk_filter_) {}

  size_t size() const { return sizeof(*this); }
  bool equals(const DlUnknownColorFilter* other) const {
    return sk_filter_ == other->sk_filter_;
  }

  sk_sp<SkColorFilter> sk_filter() const { return sk_filter_; }

 private:
  sk_sp<SkColorFilter> sk_filter_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_FILTER_H_
