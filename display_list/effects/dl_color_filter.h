// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_EFFECTS_DL_COLOR_FILTER_H_
#define FLUTTER_DISPLAY_LIST_EFFECTS_DL_COLOR_FILTER_H_

#include "flutter/display_list/dl_blend_mode.h"
#include "flutter/display_list/dl_color.h"
#include "flutter/display_list/effects/dl_attributes.h"
#include "flutter/fml/logging.h"

namespace flutter {

class DlBlendColorFilter;
class DlMatrixColorFilter;
class DlSrgbToLinearGammaColorFilter;
class DlLinearToSrgbGammaColorFilter;

// The DisplayList ColorFilter class. This class implements all of the
// facilities and adheres to the design goals of the |DlAttribute| base
// class.

// An enumerated type for the supported ColorFilter operations.
enum class DlColorFilterType {
  kBlend,
  kMatrix,
  kSrgbToLinearGamma,
  kLinearToSrgbGamma,
};

class DlColorFilter : public DlAttribute<DlColorFilter, DlColorFilterType> {
 public:
  // Construct and return a |dl_shared| pointer to a |DlBlendColorFilter|
  // based on the supplied parameters, or nullptr if the parameters specify
  // a NOP operation.
  static dl_shared<DlColorFilter> MakeBlend(DlColor color, DlBlendMode mode);

  // Construct and return a |dl_shared| pointer to a |DlMatrixColorFilter|
  // based on the supplied parameters, or nullptr if the parameters specify
  // a NOP operation.
  static dl_shared<DlColorFilter> MakeMatrix(const float matrix[20]);

  // Return a |dl_shared| pointer to a |DlSrgbToLinearGammaColorFilter|.
  static dl_shared<DlColorFilter> MakeSrgbToLinearGamma();

  // Return a |dl_shared| pointer to a |DlLinearToSrgbGammaColorFilter|.
  static dl_shared<DlColorFilter> MakeLinearToSrgbGamma();

  // Return a boolean indicating whether the color filtering operation will
  // modify transparent black. This is typically used to determine if applying
  // the ColorFilter to a temporary saveLayer buffer will turn the surrounding
  // pixels non-transparent and therefore expand the bounds.
  virtual bool modifies_transparent_black() const = 0;

  // Return a boolean indicating whether the color filtering operation can
  // be applied either before or after modulating the pixels with an opacity
  // value without changing the operation.
  virtual bool can_commute_with_opacity() const { return false; }

  // Return a DlBlendColorFilter pointer to this object iff it is a Blend
  // type of ColorFilter, otherwise return nullptr.
  virtual const DlBlendColorFilter* asBlend() const { return nullptr; }

  // Return a DlMatrixColorFilter pointer to this object iff it is a Matrix
  // type of ColorFilter, otherwise return nullptr.
  virtual const DlMatrixColorFilter* asMatrix() const { return nullptr; }

  // asSrgb<->Linear is not needed because it has no properties to query.
  // Its type fully specifies its operation.
};

// The Blend type of ColorFilter which specifies modifying the
// colors as if the color specified in the Blend filter is the
// source color and the color drawn by the rendering operation
// is the destination color. The mode parameter of the Blend
// filter is then used to combine those colors.
class DlBlendColorFilter final : public DlColorFilter {
 public:
  static dl_shared<DlBlendColorFilter> Make(DlColor color, DlBlendMode mode);

  DlColorFilterType type() const override { return DlColorFilterType::kBlend; }
  size_t size() const override { return sizeof(*this); }

  bool modifies_transparent_black() const override;
  bool can_commute_with_opacity() const override;

  const DlBlendColorFilter* asBlend() const override { return this; }

  DlColor color() const { return color_; }
  DlBlendMode mode() const { return mode_; }

 protected:
  bool equals_(DlColorFilter const& other) const override {
    FML_DCHECK(other.type() == DlColorFilterType::kBlend);
    auto that = static_cast<DlBlendColorFilter const*>(&other);
    return color_ == that->color_ && mode_ == that->mode_;
  }

 private:
  DlBlendColorFilter(DlColor color, DlBlendMode mode)
      : color_(color), mode_(mode) {}

  DlColor color_;
  DlBlendMode mode_;
};

// The Matrix type of ColorFilter which runs every pixel drawn by
// the rendering operation [iR,iG,iB,iA] through a vector/matrix
// multiplication, as in:
//
//  [ oR ]   [ m[ 0] m[ 1] m[ 2] m[ 3] m[ 4] ]   [ iR ]
//  [ oG ]   [ m[ 5] m[ 6] m[ 7] m[ 8] m[ 9] ]   [ iG ]
//  [ oB ] = [ m[10] m[11] m[12] m[13] m[14] ] x [ iB ]
//  [ oA ]   [ m[15] m[16] m[17] m[18] m[19] ]   [ iA ]
//                                               [  1 ]
//
// The resulting color [oR,oG,oB,oA] is then clamped to the range of
// valid pixel components before storing in the output.
//
// The incoming and outgoing [iR,iG,iB,iA] and [oR,oG,oB,oA] are
// considered to be non-premultiplied. When working on premultiplied
// pixel data, the necessary pre<->non-pre conversions must be performed.
class DlMatrixColorFilter final : public DlColorFilter {
 public:
  static dl_shared<DlMatrixColorFilter> Make(const float matrix[20]);

  DlColorFilterType type() const override { return DlColorFilterType::kMatrix; }
  size_t size() const override { return sizeof(*this); }

  bool modifies_transparent_black() const override;
  bool can_commute_with_opacity() const override;

  const DlMatrixColorFilter* asMatrix() const override { return this; }

  const float& operator[](int index) const { return matrix_[index]; }
  void get_matrix(float matrix[20]) const {
    memcpy(matrix, matrix_, sizeof(matrix_));
  }

 protected:
  bool equals_(const DlColorFilter& other) const override {
    FML_DCHECK(other.type() == DlColorFilterType::kMatrix);
    auto that = static_cast<DlMatrixColorFilter const*>(&other);
    return memcmp(matrix_, that->matrix_, sizeof(matrix_)) == 0;
  }

 private:
  DlMatrixColorFilter(const float* matrix) {
    memcpy(matrix_, matrix, sizeof(matrix_));
  }

  float matrix_[20];
};

// The SrgbToLinear type of ColorFilter that applies the inverse of the sRGB
// gamma curve to the rendered pixels.
class DlSrgbToLinearGammaColorFilter final : public DlColorFilter {
 public:
  static dl_shared<DlSrgbToLinearGammaColorFilter> Make() { return instance; }

  DlColorFilterType type() const override {
    return DlColorFilterType::kSrgbToLinearGamma;
  }
  size_t size() const override { return sizeof(*this); }
  bool modifies_transparent_black() const override { return false; }
  bool can_commute_with_opacity() const override { return true; }

 protected:
  bool equals_(const DlColorFilter& other) const override {
    FML_DCHECK(other.type() == DlColorFilterType::kSrgbToLinearGamma);
    return true;
  }

 private:
  static dl_shared<DlSrgbToLinearGammaColorFilter> instance;

  DlSrgbToLinearGammaColorFilter() {}
};

// The LinearToSrgb type of ColorFilter that applies the sRGB gamma curve
// to the rendered pixels.
class DlLinearToSrgbGammaColorFilter final : public DlColorFilter {
 public:
  static dl_shared<DlLinearToSrgbGammaColorFilter> Make() { return instance; }

  DlColorFilterType type() const override {
    return DlColorFilterType::kLinearToSrgbGamma;
  }
  size_t size() const override { return sizeof(*this); }
  bool modifies_transparent_black() const override { return false; }
  bool can_commute_with_opacity() const override { return true; }

 protected:
  bool equals_(const DlColorFilter& other) const override {
    FML_DCHECK(other.type() == DlColorFilterType::kLinearToSrgbGamma);
    return true;
  }

 private:
  static dl_shared<DlLinearToSrgbGammaColorFilter> instance;

  DlLinearToSrgbGammaColorFilter() {}
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_EFFECTS_DL_COLOR_FILTER_H_
