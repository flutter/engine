// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_EFFECTS_DL_IMAGE_FILTER_H_
#define FLUTTER_DISPLAY_LIST_EFFECTS_DL_IMAGE_FILTER_H_

#include "flutter/display_list/dl_attributes.h"
#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/dl_sampling_options.h"
#include "flutter/display_list/dl_tile_mode.h"
#include "flutter/display_list/effects/dl_color_filter.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_rect.h"
#include "flutter/display_list/geometry/dl_transform.h"
#include "flutter/display_list/utils/dl_comparable.h"
#include "flutter/fml/logging.h"

namespace flutter {

// The DisplayList ImageFilter class. This class implements all of the
// facilities and adheres to the design goals of the |DlAttribute| base
// class.
//
// The objects here define operations that can take a location and one or
// more input pixels and produce a color for that output pixel

// An enumerated type for the supported ImageFilter operations.
enum class DlImageFilterType {
  kBlur,
  kDilate,
  kErode,
  kMatrix,
  kCompose,
  kColorFilter,
  kLocalMatrix,
};

class DlBlurImageFilter;
class DlDilateImageFilter;
class DlErodeImageFilter;
class DlMatrixImageFilter;
class DlLocalMatrixImageFilter;
class DlComposeImageFilter;
class DlColorFilterImageFilter;

class DlImageFilter : public DlAttribute<DlImageFilter, DlImageFilterType> {
 public:
  enum class MatrixCapability {
    kTranslate,
    kScaleTranslate,
    kComplex,
  };

  // Return a DlBlurImageFilter pointer to this object iff it is a Blur
  // type of ImageFilter, otherwise return nullptr.
  virtual const DlBlurImageFilter* asBlur() const { return nullptr; }

  // Return a DlDilateImageFilter pointer to this object iff it is a Dilate
  // type of ImageFilter, otherwise return nullptr.
  virtual const DlDilateImageFilter* asDilate() const { return nullptr; }

  // Return a DlErodeImageFilter pointer to this object iff it is an Erode
  // type of ImageFilter, otherwise return nullptr.
  virtual const DlErodeImageFilter* asErode() const { return nullptr; }

  // Return a DlMatrixImageFilter pointer to this object iff it is a Matrix
  // type of ImageFilter, otherwise return nullptr.
  virtual const DlMatrixImageFilter* asMatrix() const { return nullptr; }

  virtual const DlLocalMatrixImageFilter* asLocalMatrix() const {
    return nullptr;
  }

  virtual std::shared_ptr<DlImageFilter> makeWithLocalMatrix(
      const DlTransform& matrix) const;

  // Return a DlComposeImageFilter pointer to this object iff it is a Compose
  // type of ImageFilter, otherwise return nullptr.
  virtual const DlComposeImageFilter* asCompose() const { return nullptr; }

  // Return a DlColorFilterImageFilter pointer to this object iff it is a
  // ColorFilter type of ImageFilter, otherwise return nullptr.
  virtual const DlColorFilterImageFilter* asColorFilter() const {
    return nullptr;
  }

  // Return a boolean indicating whether the image filtering operation will
  // modify transparent black. This is typically used to determine if applying
  // the ImageFilter to a temporary saveLayer buffer will turn the surrounding
  // pixels non-transparent and therefore expand the bounds.
  virtual bool modifies_transparent_black() const = 0;

  // Return the bounds of the output for this image filtering operation
  // based on the supplied input bounds where both are measured in the local
  // (untransformed) coordinate space.
  //
  // The method will return a pointer to the output_bounds parameter if it
  // can successfully compute the output bounds of the filter, otherwise the
  // method will return a nullptr and the output_bounds will be filled with
  // a best guess for the answer, even if just a copy of the input_bounds.
  virtual DlFRect* map_local_bounds(const DlFRect& input_bounds,
                                    DlFRect& output_bounds) const = 0;

  // Return the device bounds of the output for this image filtering operation
  // based on the supplied input device bounds where both are measured in the
  // pixel coordinate space and relative to the given rendering ctm. The
  // transform matrix is used to adjust the filter parameters for when it
  // is used in a rendering operation (for example, the blur radius of a
  // Blur filter will expand based on the ctm).
  //
  // The method will return a pointer to the output_bounds parameter if it
  // can successfully compute the output bounds of the filter, otherwise the
  // method will return a nullptr and the output_bounds will be filled with
  // a best guess for the answer, even if just a copy of the input_bounds.
  virtual DlIRect* map_device_bounds(const DlIRect& input_bounds,
                                     const DlTransform& ctm,
                                     DlIRect& output_bounds) const = 0;

  // Return the input bounds that will be needed in order for the filter to
  // properly fill the indicated output_bounds under the specified
  // transformation matrix. Both output_bounds and input_bounds are taken to
  // be relative to the transformed coordinate space of the provided |ctm|.
  //
  // The method will return a pointer to the input_bounds parameter if it
  // can successfully compute the required input bounds, otherwise the
  // method will return a nullptr and the input_bounds will be filled with
  // a best guess for the answer, even if just a copy of the output_bounds.
  virtual DlIRect* get_input_device_bounds(const DlIRect& output_bounds,
                                           const DlTransform& ctm,
                                           DlIRect& input_bounds) const = 0;

  virtual MatrixCapability matrix_capability() const {
    return MatrixCapability::kScaleTranslate;
  }

 protected:
  static DlIRect* inset_device_bounds(const DlIRect& input_bounds,
                                      DlScalar radius_x,
                                      DlScalar radius_y,
                                      const DlTransform& ctm,
                                      DlIRect& output_bounds) {
    if (ctm.IsFinite()) {
      auto expansion = ctm.ComputeTransformedExpansion(radius_x, radius_y);
      if (expansion.has_value()) {
        output_bounds = input_bounds.Expand(-Floor(*expansion));
        return &output_bounds;
      }
      auto inverse = ctm.Inverse();
      if (inverse.has_value()) {
        DlFRect local_bounds =
            inverse->TransformRect(input_bounds).Expand(-radius_x, -radius_y);
        output_bounds =
            DlIRect::MakeRoundedOut(ctm.TransformRect(local_bounds));
        return &output_bounds;
      }
    }
    output_bounds = input_bounds;
    return nullptr;
  }

  static DlIRect* outset_device_bounds(const DlIRect& input_bounds,
                                       DlScalar radius_x,
                                       DlScalar radius_y,
                                       const DlTransform& ctm,
                                       DlIRect& output_bounds) {
    if (ctm.IsFinite()) {
      auto expansion = ctm.ComputeTransformedExpansion(radius_x, radius_y);
      if (expansion.has_value()) {
        output_bounds = input_bounds.Expand(Ceil(*expansion));
        return &output_bounds;
      }
      auto inverse = ctm.Inverse();
      if (inverse.has_value()) {
        DlFRect local_bounds = inverse->TransformRect(input_bounds);
        local_bounds = local_bounds.Expand(radius_x, radius_y);
        output_bounds =
            DlIRect::MakeRoundedOut(ctm.TransformRect(local_bounds));
        return &output_bounds;
      }
    }
    output_bounds = input_bounds;
    return nullptr;
  }
};

class DlBlurImageFilter final : public DlImageFilter {
 public:
  DlBlurImageFilter(DlScalar sigma_x, DlScalar sigma_y, DlTileMode tile_mode)
      : sigma_x_(sigma_x), sigma_y_(sigma_y), tile_mode_(tile_mode) {}
  explicit DlBlurImageFilter(const DlBlurImageFilter* filter)
      : DlBlurImageFilter(filter->sigma_x_,
                          filter->sigma_y_,
                          filter->tile_mode_) {}
  explicit DlBlurImageFilter(const DlBlurImageFilter& filter)
      : DlBlurImageFilter(&filter) {}

  static std::shared_ptr<DlImageFilter> Make(DlScalar sigma_x,
                                             DlScalar sigma_y,
                                             DlTileMode tile_mode) {
    if (!DlScalars_AreFinite(sigma_x, sigma_y)) {
      return nullptr;
    }
    if (sigma_x < kDlScalar_NearlyZero && sigma_y < kDlScalar_NearlyZero) {
      return nullptr;
    }
    sigma_x = (sigma_x < kDlScalar_NearlyZero) ? 0 : sigma_x;
    sigma_y = (sigma_y < kDlScalar_NearlyZero) ? 0 : sigma_y;
    return std::make_shared<DlBlurImageFilter>(sigma_x, sigma_y, tile_mode);
  }

  std::shared_ptr<DlImageFilter> shared() const override {
    return std::make_shared<DlBlurImageFilter>(this);
  }

  DlImageFilterType type() const override { return DlImageFilterType::kBlur; }
  size_t size() const override { return sizeof(*this); }

  const DlBlurImageFilter* asBlur() const override { return this; }

  bool modifies_transparent_black() const override { return false; }

  DlFRect* map_local_bounds(const DlFRect& input_bounds,
                            DlFRect& output_bounds) const override {
    output_bounds = input_bounds.Expand(sigma_x_ * 3.0f, sigma_y_ * 3.0f);
    return &output_bounds;
  }

  DlIRect* map_device_bounds(const DlIRect& input_bounds,
                             const DlTransform& ctm,
                             DlIRect& output_bounds) const override {
    return outset_device_bounds(input_bounds, sigma_x_ * 3.0f, sigma_y_ * 3.0f,
                                ctm, output_bounds);
  }

  DlIRect* get_input_device_bounds(const DlIRect& output_bounds,
                                   const DlTransform& ctm,
                                   DlIRect& input_bounds) const override {
    // Blurs are symmetric in terms of output-for-input and input-for-output
    return map_device_bounds(output_bounds, ctm, input_bounds);
  }

  DlScalar sigma_x() const { return sigma_x_; }
  DlScalar sigma_y() const { return sigma_y_; }
  DlTileMode tile_mode() const { return tile_mode_; }

 protected:
  bool equals_(const DlImageFilter& other) const override {
    FML_DCHECK(other.type() == DlImageFilterType::kBlur);
    auto that = static_cast<const DlBlurImageFilter*>(&other);
    return (sigma_x_ == that->sigma_x_ && sigma_y_ == that->sigma_y_ &&
            tile_mode_ == that->tile_mode_);
  }

 private:
  DlScalar sigma_x_;
  DlScalar sigma_y_;
  DlTileMode tile_mode_;
};

class DlDilateImageFilter final : public DlImageFilter {
 public:
  DlDilateImageFilter(DlScalar radius_x, DlScalar radius_y)
      : radius_x_(radius_x), radius_y_(radius_y) {}
  explicit DlDilateImageFilter(const DlDilateImageFilter* filter)
      : DlDilateImageFilter(filter->radius_x_, filter->radius_y_) {}
  explicit DlDilateImageFilter(const DlDilateImageFilter& filter)
      : DlDilateImageFilter(&filter) {}

  static std::shared_ptr<DlImageFilter> Make(DlScalar radius_x,
                                             DlScalar radius_y) {
    if (DlScalars_AreFinite(radius_x, radius_y) &&
        radius_x > kDlScalar_NearlyZero &&  //
        radius_y > kDlScalar_NearlyZero) {
      return std::make_shared<DlDilateImageFilter>(radius_x, radius_y);
    }
    return nullptr;
  }

  std::shared_ptr<DlImageFilter> shared() const override {
    return std::make_shared<DlDilateImageFilter>(this);
  }

  DlImageFilterType type() const override { return DlImageFilterType::kDilate; }
  size_t size() const override { return sizeof(*this); }

  const DlDilateImageFilter* asDilate() const override { return this; }

  bool modifies_transparent_black() const override { return false; }

  DlFRect* map_local_bounds(const DlFRect& input_bounds,
                            DlFRect& output_bounds) const override {
    output_bounds = input_bounds.Expand(radius_x_, radius_y_);
    return &output_bounds;
  }

  DlIRect* map_device_bounds(const DlIRect& input_bounds,
                             const DlTransform& ctm,
                             DlIRect& output_bounds) const override {
    return outset_device_bounds(input_bounds, radius_x_, radius_y_, ctm,
                                output_bounds);
  }

  DlIRect* get_input_device_bounds(const DlIRect& output_bounds,
                                   const DlTransform& ctm,
                                   DlIRect& input_bounds) const override {
    return inset_device_bounds(output_bounds, radius_x_, radius_y_, ctm,
                               input_bounds);
  }

  DlScalar radius_x() const { return radius_x_; }
  DlScalar radius_y() const { return radius_y_; }

 protected:
  bool equals_(const DlImageFilter& other) const override {
    FML_DCHECK(other.type() == DlImageFilterType::kDilate);
    auto that = static_cast<const DlDilateImageFilter*>(&other);
    return (radius_x_ == that->radius_x_ && radius_y_ == that->radius_y_);
  }

 private:
  DlScalar radius_x_;
  DlScalar radius_y_;
};

class DlErodeImageFilter final : public DlImageFilter {
 public:
  DlErodeImageFilter(DlScalar radius_x, DlScalar radius_y)
      : radius_x_(radius_x), radius_y_(radius_y) {}
  explicit DlErodeImageFilter(const DlErodeImageFilter* filter)
      : DlErodeImageFilter(filter->radius_x_, filter->radius_y_) {}
  explicit DlErodeImageFilter(const DlErodeImageFilter& filter)
      : DlErodeImageFilter(&filter) {}

  static std::shared_ptr<DlImageFilter> Make(DlScalar radius_x,
                                             DlScalar radius_y) {
    if (DlScalars_AreFinite(radius_x, radius_y) &&
        radius_x > kDlScalar_NearlyZero &&  //
        radius_y > kDlScalar_NearlyZero) {
      return std::make_shared<DlErodeImageFilter>(radius_x, radius_y);
    }
    return nullptr;
  }

  std::shared_ptr<DlImageFilter> shared() const override {
    return std::make_shared<DlErodeImageFilter>(this);
  }

  DlImageFilterType type() const override { return DlImageFilterType::kErode; }
  size_t size() const override { return sizeof(*this); }

  const DlErodeImageFilter* asErode() const override { return this; }

  bool modifies_transparent_black() const override { return false; }

  DlFRect* map_local_bounds(const DlFRect& input_bounds,
                            DlFRect& output_bounds) const override {
    output_bounds = input_bounds.Expand(-radius_x_, -radius_y_);
    return &output_bounds;
  }

  DlIRect* map_device_bounds(const DlIRect& input_bounds,
                             const DlTransform& ctm,
                             DlIRect& output_bounds) const override {
    return inset_device_bounds(input_bounds, radius_x_, radius_y_, ctm,
                               output_bounds);
  }

  DlIRect* get_input_device_bounds(const DlIRect& output_bounds,
                                   const DlTransform& ctm,
                                   DlIRect& input_bounds) const override {
    return outset_device_bounds(output_bounds, radius_x_, radius_y_, ctm,
                                input_bounds);
  }

  DlScalar radius_x() const { return radius_x_; }
  DlScalar radius_y() const { return radius_y_; }

 protected:
  bool equals_(const DlImageFilter& other) const override {
    FML_DCHECK(other.type() == DlImageFilterType::kErode);
    auto that = static_cast<const DlErodeImageFilter*>(&other);
    return (radius_x_ == that->radius_x_ && radius_y_ == that->radius_y_);
  }

 private:
  DlScalar radius_x_;
  DlScalar radius_y_;
};

class DlMatrixImageFilter final : public DlImageFilter {
 public:
  DlMatrixImageFilter(const DlTransform& matrix, DlImageSampling sampling)
      : matrix_(matrix), sampling_(sampling) {}
  explicit DlMatrixImageFilter(const DlMatrixImageFilter* filter)
      : DlMatrixImageFilter(filter->matrix_, filter->sampling_) {}
  explicit DlMatrixImageFilter(const DlMatrixImageFilter& filter)
      : DlMatrixImageFilter(&filter) {}

  static std::shared_ptr<DlImageFilter> Make(const DlTransform& matrix,
                                             DlImageSampling sampling) {
    if (matrix.IsFinite() && !matrix.IsIdentity()) {
      return std::make_shared<DlMatrixImageFilter>(matrix, sampling);
    }
    return nullptr;
  }

  std::shared_ptr<DlImageFilter> shared() const override {
    return std::make_shared<DlMatrixImageFilter>(this);
  }

  DlImageFilterType type() const override { return DlImageFilterType::kMatrix; }
  size_t size() const override { return sizeof(*this); }

  const DlTransform& matrix() const { return matrix_; }
  DlImageSampling sampling() const { return sampling_; }

  const DlMatrixImageFilter* asMatrix() const override { return this; }

  bool modifies_transparent_black() const override { return false; }

  DlFRect* map_local_bounds(const DlFRect& input_bounds,
                            DlFRect& output_bounds) const override {
    output_bounds = matrix_.TransformRect(input_bounds);
    return &output_bounds;
  }

  DlIRect* map_device_bounds(const DlIRect& input_bounds,
                             const DlTransform& ctm,
                             DlIRect& output_bounds) const override {
    auto inverse = ctm.Inverse();
    if (!inverse.has_value()) {
      output_bounds = input_bounds;
      return nullptr;
    }
    inverse->ConcatOuter(matrix_);
    inverse->ConcatOuter(ctm);
    output_bounds =
        DlIRect::MakeRoundedOut(inverse->TransformRect(input_bounds));
    return &output_bounds;
  }

  DlIRect* get_input_device_bounds(const DlIRect& output_bounds,
                                   const DlTransform& ctm,
                                   DlIRect& input_bounds) const override {
    DlTransform matrix = DlTransform::MakeConcat(ctm, matrix_);
    auto inverse = matrix.Inverse();
    if (!inverse.has_value()) {
      input_bounds = output_bounds;
      return nullptr;
    }
    inverse->ConcatOuter(ctm);
    input_bounds =
        DlIRect::MakeRoundedOut(inverse->TransformRect(output_bounds));
    return &input_bounds;
  }

 protected:
  bool equals_(const DlImageFilter& other) const override {
    FML_DCHECK(other.type() == DlImageFilterType::kMatrix);
    auto that = static_cast<const DlMatrixImageFilter*>(&other);
    return (matrix_ == that->matrix_ && sampling_ == that->sampling_);
  }

 private:
  DlTransform matrix_;
  DlImageSampling sampling_;
};

class DlComposeImageFilter final : public DlImageFilter {
 public:
  DlComposeImageFilter(std::shared_ptr<const DlImageFilter> outer,
                       std::shared_ptr<const DlImageFilter> inner)
      : outer_(std::move(outer)), inner_(std::move(inner)) {}
  DlComposeImageFilter(const DlImageFilter* outer, const DlImageFilter* inner)
      : outer_(outer->shared()), inner_(inner->shared()) {}
  DlComposeImageFilter(const DlImageFilter& outer, const DlImageFilter& inner)
      : DlComposeImageFilter(&outer, &inner) {}
  explicit DlComposeImageFilter(const DlComposeImageFilter* filter)
      : DlComposeImageFilter(filter->outer_, filter->inner_) {}
  explicit DlComposeImageFilter(const DlComposeImageFilter& filter)
      : DlComposeImageFilter(&filter) {}

  static std::shared_ptr<const DlImageFilter> Make(
      std::shared_ptr<const DlImageFilter> outer,
      std::shared_ptr<const DlImageFilter> inner) {
    if (!outer) {
      return inner;
    }
    if (!inner) {
      return outer;
    }
    return std::make_shared<DlComposeImageFilter>(outer, inner);
  }

  std::shared_ptr<DlImageFilter> shared() const override {
    return std::make_shared<DlComposeImageFilter>(this);
  }

  DlImageFilterType type() const override {
    return DlImageFilterType::kCompose;
  }
  size_t size() const override { return sizeof(*this); }

  std::shared_ptr<const DlImageFilter> outer() const { return outer_; }
  std::shared_ptr<const DlImageFilter> inner() const { return inner_; }

  const DlComposeImageFilter* asCompose() const override { return this; }

  bool modifies_transparent_black() const override {
    if (inner_ && inner_->modifies_transparent_black()) {
      return true;
    }
    if (outer_ && outer_->modifies_transparent_black()) {
      return true;
    }
    return false;
  }

  DlFRect* map_local_bounds(const DlFRect& input_bounds,
                            DlFRect& output_bounds) const override;

  DlIRect* map_device_bounds(const DlIRect& input_bounds,
                             const DlTransform& ctm,
                             DlIRect& output_bounds) const override;

  DlIRect* get_input_device_bounds(const DlIRect& output_bounds,
                                   const DlTransform& ctm,
                                   DlIRect& input_bounds) const override;

  MatrixCapability matrix_capability() const override {
    return std::min(outer_->matrix_capability(), inner_->matrix_capability());
  }

 protected:
  bool equals_(const DlImageFilter& other) const override {
    FML_DCHECK(other.type() == DlImageFilterType::kCompose);
    auto that = static_cast<const DlComposeImageFilter*>(&other);
    return (Equals(outer_, that->outer_) && Equals(inner_, that->inner_));
  }

 private:
  std::shared_ptr<const DlImageFilter> outer_;
  std::shared_ptr<const DlImageFilter> inner_;
};

class DlColorFilterImageFilter final : public DlImageFilter {
 public:
  explicit DlColorFilterImageFilter(std::shared_ptr<const DlColorFilter> filter)
      : color_filter_(std::move(filter)) {}
  explicit DlColorFilterImageFilter(const DlColorFilter* filter)
      : color_filter_(filter->shared()) {}
  explicit DlColorFilterImageFilter(const DlColorFilter& filter)
      : color_filter_(filter.shared()) {}
  explicit DlColorFilterImageFilter(const DlColorFilterImageFilter* filter)
      : DlColorFilterImageFilter(filter->color_filter_) {}
  explicit DlColorFilterImageFilter(const DlColorFilterImageFilter& filter)
      : DlColorFilterImageFilter(&filter) {}

  static std::shared_ptr<DlImageFilter> Make(
      std::shared_ptr<const DlColorFilter> filter) {
    if (filter) {
      return std::make_shared<DlColorFilterImageFilter>(filter);
    }
    return nullptr;
  }

  std::shared_ptr<DlImageFilter> shared() const override {
    return std::make_shared<DlColorFilterImageFilter>(color_filter_);
  }

  DlImageFilterType type() const override {
    return DlImageFilterType::kColorFilter;
  }
  size_t size() const override { return sizeof(*this); }

  const std::shared_ptr<const DlColorFilter> color_filter() const {
    return color_filter_;
  }

  const DlColorFilterImageFilter* asColorFilter() const override {
    return this;
  }

  bool modifies_transparent_black() const override {
    if (color_filter_) {
      return color_filter_->modifies_transparent_black();
    }
    return false;
  }

  DlFRect* map_local_bounds(const DlFRect& input_bounds,
                            DlFRect& output_bounds) const override {
    output_bounds = input_bounds;
    return modifies_transparent_black() ? nullptr : &output_bounds;
  }

  DlIRect* map_device_bounds(const DlIRect& input_bounds,
                             const DlTransform& ctm,
                             DlIRect& output_bounds) const override {
    output_bounds = input_bounds;
    return modifies_transparent_black() ? nullptr : &output_bounds;
  }

  DlIRect* get_input_device_bounds(const DlIRect& output_bounds,
                                   const DlTransform& ctm,
                                   DlIRect& input_bounds) const override {
    return map_device_bounds(output_bounds, ctm, input_bounds);
  }

  MatrixCapability matrix_capability() const override {
    return MatrixCapability::kComplex;
  }

  std::shared_ptr<DlImageFilter> makeWithLocalMatrix(
      const DlTransform& matrix) const override {
    return shared();
  }

 protected:
  bool equals_(const DlImageFilter& other) const override {
    FML_DCHECK(other.type() == DlImageFilterType::kColorFilter);
    auto that = static_cast<const DlColorFilterImageFilter*>(&other);
    return Equals(color_filter_, that->color_filter_);
  }

 private:
  std::shared_ptr<const DlColorFilter> color_filter_;
};

class DlLocalMatrixImageFilter final : public DlImageFilter {
 public:
  explicit DlLocalMatrixImageFilter(const DlTransform& matrix,
                                    std::shared_ptr<DlImageFilter> filter)
      : matrix_(matrix), image_filter_(filter) {}
  explicit DlLocalMatrixImageFilter(const DlLocalMatrixImageFilter* filter)
      : DlLocalMatrixImageFilter(filter->matrix_, filter->image_filter_) {}
  DlLocalMatrixImageFilter(const DlLocalMatrixImageFilter& filter)
      : DlLocalMatrixImageFilter(&filter) {}
  std::shared_ptr<DlImageFilter> shared() const override {
    return std::make_shared<DlLocalMatrixImageFilter>(this);
  }

  DlImageFilterType type() const override {
    return DlImageFilterType::kLocalMatrix;
  }
  size_t size() const override { return sizeof(*this); }

  const DlTransform& matrix() const { return matrix_; }

  const std::shared_ptr<DlImageFilter> image_filter() const {
    return image_filter_;
  }

  const DlLocalMatrixImageFilter* asLocalMatrix() const override {
    return this;
  }

  bool modifies_transparent_black() const override {
    if (!image_filter_) {
      return false;
    }
    return image_filter_->modifies_transparent_black();
  }

  DlFRect* map_local_bounds(const DlFRect& input_bounds,
                            DlFRect& output_bounds) const override {
    if (!image_filter_) {
      return nullptr;
    }
    return image_filter_->map_local_bounds(input_bounds, output_bounds);
  }

  DlIRect* map_device_bounds(const DlIRect& input_bounds,
                             const DlTransform& ctm,
                             DlIRect& output_bounds) const override {
    if (!image_filter_) {
      return nullptr;
    }
    return image_filter_->map_device_bounds(
        input_bounds, DlTransform::MakeConcat(ctm, matrix_), output_bounds);
  }

  DlIRect* get_input_device_bounds(const DlIRect& output_bounds,
                                   const DlTransform& ctm,
                                   DlIRect& input_bounds) const override {
    if (!image_filter_) {
      return nullptr;
    }
    return image_filter_->get_input_device_bounds(
        output_bounds, DlTransform::MakeConcat(ctm, matrix_), input_bounds);
  }

 protected:
  bool equals_(const DlImageFilter& other) const override {
    FML_DCHECK(other.type() == DlImageFilterType::kLocalMatrix);
    auto that = static_cast<const DlLocalMatrixImageFilter*>(&other);
    return (matrix_ == that->matrix_ &&
            Equals(image_filter_, that->image_filter_));
  }

 private:
  DlTransform matrix_;
  std::shared_ptr<DlImageFilter> image_filter_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_EFFECTS_DL_IMAGE_FILTER_H_
