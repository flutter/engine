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
    kBlend,
    kMatrix,
    kSrgbToLinearGamma,
    kLinearToSrgbGamma,
    kUnknown
  };

  static std::shared_ptr<DlColorFilter> From(SkColorFilter* sk_filter);
  static std::shared_ptr<DlColorFilter> From(sk_sp<SkColorFilter> sk_filter) {
    return From(sk_filter.get());
  }

  virtual Type type() const = 0;
  virtual size_t size() const = 0;
  virtual bool modifies_transparent_black() const = 0;

  static std::shared_ptr<DlColorFilter> Shared(const DlColorFilter* filter) {
    return filter == nullptr ? nullptr : filter->shared();
  }
  virtual std::shared_ptr<DlColorFilter> shared() const = 0;
  virtual sk_sp<SkColorFilter> sk_filter() const = 0;

  // asSrgb<->Linear and asUnknown are not needed because they
  // have no properties to query. Their type fully specifies their
  // operation or can be accessed via the common sk_filter() method.
  virtual const DlBlendColorFilter* asBlend() const { return nullptr; }
  virtual const DlMatrixColorFilter* asMatrix() const { return nullptr; }

  bool operator==(DlColorFilter const& other) const {
    return type() == other.type() && equals_(other);
  }
  bool operator!=(DlColorFilter const& other) const {
    return !(*this == other);
  }

  virtual ~DlColorFilter() = default;

 protected:
  virtual bool equals_(DlColorFilter const& other) const = 0;
};

class DlBlendColorFilter final : public DlColorFilter {
 public:
  DlBlendColorFilter(SkColor color, SkBlendMode mode)
      : color_(color), mode_(mode) {}
  DlBlendColorFilter(const DlBlendColorFilter& filter)
      : DlBlendColorFilter(filter.color_, filter.mode_) {}
  DlBlendColorFilter(const DlBlendColorFilter* filter)
      : DlBlendColorFilter(filter->color_, filter->mode_) {}

  Type type() const override { return kBlend; }
  size_t size() const override { return sizeof(*this); }
  bool modifies_transparent_black() const override {
    // Look at blend and color to make a faster determination?
    return sk_filter()->filterColor(SK_ColorTRANSPARENT) != SK_ColorTRANSPARENT;
  }

  std::shared_ptr<DlColorFilter> shared() const override {
    return std::make_shared<DlBlendColorFilter>(this);
  }

  sk_sp<SkColorFilter> sk_filter() const override {
    return SkColorFilters::Blend(color_, mode_);
  }

  const DlBlendColorFilter* asBlend() const override { return this; }

  SkColor color() const { return color_; }
  SkBlendMode mode() const { return mode_; }

 protected:
  bool equals_(DlColorFilter const& other) const override {
    FML_DCHECK(other.type() == kBlend);
    auto that = static_cast<DlBlendColorFilter const&>(other);
    return color_ == that.color_ && mode_ == that.mode_;
  }

 private:
  SkColor color_;
  SkBlendMode mode_;
};

class DlMatrixColorFilter final : public DlColorFilter {
 public:
  DlMatrixColorFilter(const float matrix[20]) {
    memcpy(matrix_, matrix, sizeof(matrix_));
  }
  DlMatrixColorFilter(const DlMatrixColorFilter& filter)
      : DlMatrixColorFilter(filter.matrix_) {}
  DlMatrixColorFilter(const DlMatrixColorFilter* filter)
      : DlMatrixColorFilter(filter->matrix_) {}

  Type type() const override { return kMatrix; }
  size_t size() const override { return sizeof(*this); }
  bool modifies_transparent_black() const override {
    // Look at the matrix to make a faster determination?
    // Basically, are the translation components all 0?
    return sk_filter()->filterColor(SK_ColorTRANSPARENT) != SK_ColorTRANSPARENT;
  }

  std::shared_ptr<DlColorFilter> shared() const override {
    return std::make_shared<DlMatrixColorFilter>(this);
  }

  sk_sp<SkColorFilter> sk_filter() const override {
    return SkColorFilters::Matrix(matrix_);
  }

  const DlMatrixColorFilter* asMatrix() const override { return this; }

  const float& operator[](int index) const { return matrix_[index]; }
  void get_matrix(float matrix[20]) const {
    memcpy(matrix, matrix_, sizeof(matrix_));
  }

 protected:
  bool equals_(const DlColorFilter& other) const override {
    FML_DCHECK(other.type() == kMatrix);
    auto that = static_cast<DlMatrixColorFilter const&>(other);
    return memcmp(matrix_, that.matrix_, sizeof(matrix_)) == 0;
  }

 private:
  float matrix_[20];
};

class DlSrgbToLinearGammaColorFilter final : public DlColorFilter {
 public:
  static const std::shared_ptr<DlSrgbToLinearGammaColorFilter> instance;

  DlSrgbToLinearGammaColorFilter() {}
  DlSrgbToLinearGammaColorFilter(const DlSrgbToLinearGammaColorFilter& filter)
      : DlSrgbToLinearGammaColorFilter() {}
  DlSrgbToLinearGammaColorFilter(const DlSrgbToLinearGammaColorFilter* filter)
      : DlSrgbToLinearGammaColorFilter() {}

  Type type() const override { return kSrgbToLinearGamma; }
  size_t size() const override { return sizeof(*this); }
  bool modifies_transparent_black() const override { return false; }

  std::shared_ptr<DlColorFilter> shared() const override { return instance; }
  sk_sp<SkColorFilter> sk_filter() const override { return sk_filter_; }

 protected:
  bool equals_(const DlColorFilter& other) const override {
    FML_DCHECK(other.type() == kSrgbToLinearGamma);
    return true;
  }

 private:
  static const sk_sp<SkColorFilter> sk_filter_;
  friend class DlColorFilter;
};

class DlLinearToSrgbGammaColorFilter final : public DlColorFilter {
 public:
  static const std::shared_ptr<DlLinearToSrgbGammaColorFilter> instance;

  DlLinearToSrgbGammaColorFilter() {}
  DlLinearToSrgbGammaColorFilter(const DlLinearToSrgbGammaColorFilter& filter)
      : DlLinearToSrgbGammaColorFilter() {}
  DlLinearToSrgbGammaColorFilter(const DlLinearToSrgbGammaColorFilter* filter)
      : DlLinearToSrgbGammaColorFilter() {}

  Type type() const override { return kLinearToSrgbGamma; }
  size_t size() const override { return sizeof(*this); }
  bool modifies_transparent_black() const override { return false; }

  std::shared_ptr<DlColorFilter> shared() const override { return instance; }
  sk_sp<SkColorFilter> sk_filter() const override { return sk_filter_; }

 protected:
  bool equals_(const DlColorFilter& other) const override {
    FML_DCHECK(other.type() == kLinearToSrgbGamma);
    return true;
  }

 private:
  static const sk_sp<SkColorFilter> sk_filter_;
  friend class DlColorFilter;
};

class DlUnknownColorFilter final : public DlColorFilter {
 public:
  DlUnknownColorFilter(sk_sp<SkColorFilter> sk_filter)
      : sk_filter_(std::move(sk_filter)) {}
  DlUnknownColorFilter(const DlUnknownColorFilter& filter)
      : DlUnknownColorFilter(filter.sk_filter_) {}
  DlUnknownColorFilter(const DlUnknownColorFilter* filter)
      : DlUnknownColorFilter(filter->sk_filter_) {}

  Type type() const override { return kUnknown; }
  size_t size() const override { return sizeof(*this); }
  bool modifies_transparent_black() const override {
    return sk_filter()->filterColor(SK_ColorTRANSPARENT) != SK_ColorTRANSPARENT;
  }

  std::shared_ptr<DlColorFilter> shared() const override {
    return std::make_shared<DlUnknownColorFilter>(this);
  }

  sk_sp<SkColorFilter> sk_filter() const override { return sk_filter_; }

  virtual ~DlUnknownColorFilter() = default;

 protected:
  bool equals_(const DlColorFilter& other) const override {
    FML_DCHECK(other.type() == kUnknown);
    auto that = static_cast<DlUnknownColorFilter const&>(other);
    return sk_filter_ == that.sk_filter_;
  }

 private:
  sk_sp<SkColorFilter> sk_filter_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_FILTER_H_
