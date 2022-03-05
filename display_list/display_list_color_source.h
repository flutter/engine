// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_SOURCE_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_SOURCE_H_

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/display_list_attributes.h"
#include "flutter/display_list/types.h"
#include "flutter/fml/logging.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/effects/SkGradientShader.h"

namespace flutter {

class DlColorColorSource;
class DlImageColorSource;
class DlLinearGradientColorSource;
class DlRadialGradientColorSource;
class DlConicalGradientColorSource;
class DlSweepGradientColorSource;
class DlUnknownColorSource;

enum class DlColorSourceType {
  kColor,
  kImage,
  kLinearGradient,
  kRadialGradient,
  kConicalGradient,
  kSweepGradient,
  kUnknown
};

enum class DlTileMode {
  // Replicate the edge color if the shader draws outside of its
  // original bounds.
  kClamp,

  // Repeat the shader's image horizontally and vertically.
  kRepeat,

  // Repeat the shader's image horizontally and vertically, alternating
  // mirror images so that adjacent images always seam.
  kMirror,

  // Only draw within the original domain, return transparent-black everywhere
  // else.
  kDecal,
};

class DlColorSource
    : public DlAttribute<DlColorSource, SkShader, DlColorSourceType> {
 public:
  // Return a shared_ptr holding a DlColorSource representing the indicated
  // Skia SkShader pointer.
  //
  // This method can detect each of the 4 recognized types from an analogous
  // SkShader.
  static std::shared_ptr<DlColorSource> From(SkShader* sk_filter);

  // Return a shared_ptr holding a DlColorFilter representing the indicated
  // Skia SkShader pointer.
  //
  // This method can detect each of the 4 recognized types from an analogous
  // SkShader.
  static std::shared_ptr<DlColorSource> From(sk_sp<SkShader> sk_filter) {
    return From(sk_filter.get());
  }

  static std::shared_ptr<DlColorSource> MakeLinear(const SkPoint p0,
                              const SkPoint p1,
                              uint32_t stop_count,
                              const uint32_t* colors,
                              const float* stops,
                              DlTileMode tile_mode,
                              const SkMatrix& matrix);

  virtual bool is_opaque() const = 0;

  virtual std::shared_ptr<DlColorSource> with_sampling(
      SkSamplingOptions& options) const {
    return shared();
  }

  // Return a DlColorColorSource pointer to this object iff it is an Color
  // type of ColorSource, otherwise return nullptr.
  virtual const DlColorColorSource* asColor() const { return nullptr; }

  // Return a DlImageColorSource pointer to this object iff it is an Image
  // type of ColorSource, otherwise return nullptr.
  virtual const DlImageColorSource* asImage() const { return nullptr; }

  // Return a DlLinearGradientColorSource pointer to this object iff it is a
  // Linear Gradient type of ColorSource, otherwise return nullptr.
  virtual const DlLinearGradientColorSource* asLinearGradient() const {
    return nullptr;
  }

  // Return a DlRadialGradientColorSource pointer to this object iff it is a
  // Radial Gradient type of ColorSource, otherwise return nullptr.
  virtual const DlRadialGradientColorSource* asRadialGradient() const {
    return nullptr;
  }

  // Return a DlConicalGradientColorSource pointer to this object iff it is a
  // Conical Gradient type of ColorSource, otherwise return nullptr.
  virtual const DlConicalGradientColorSource* asConicalGradient() const {
    return nullptr;
  }

  // Return a DlSweepGradientColorSource pointer to this object iff it is a
  // Sweep Gradient type of ColorSource, otherwise return nullptr.
  virtual const DlSweepGradientColorSource* asSweepGradient() const {
    return nullptr;
  }
};

class DlColorColorSource final : public DlColorSource {
 public:
  DlColorColorSource(uint32_t color) : color_(color) {}
  DlColorColorSource(const DlColorColorSource& color_source)
      : DlColorColorSource(color_source.color_) {}
  DlColorColorSource(const DlColorColorSource* color_source)
      : DlColorColorSource(color_source->color_) {}

  std::shared_ptr<DlColorSource> shared() const override {
    return std::make_shared<DlColorColorSource>(color_);
  }

  const DlColorColorSource* asColor() const override { return this; }

  DlColorSourceType type() const override { return DlColorSourceType::kColor; }
  size_t size() const override { return sizeof(*this); }

  bool is_opaque() const override { return (color_ >> 24) == 255; }

  uint32_t color() const { return color_; }

  sk_sp<SkShader> skia_object() const override {
    return SkShaders::Color(color_);
  }

 protected:
  bool equals_(DlColorSource const& other) const override {
    FML_DCHECK(other.type() == DlColorSourceType::kColor);
    auto that = static_cast<DlColorColorSource const*>(&other);
    return color_ == that->color_;
  }

 private:
  uint32_t color_;
};

class DlImageColorSource final : public SkRefCnt, public DlColorSource {
 public:
  DlImageColorSource(sk_sp<SkImage> image,
                     const SkMatrix& matrix,
                     DlTileMode horizontal_tile_mode,
                     DlTileMode vertical_tile_mode,
                     SkSamplingOptions sampling = DisplayList::LinearSampling)
      : sk_image_(image),
        matrix_(matrix),
        horizontal_tile_mode_(horizontal_tile_mode),
        vertical_tile_mode_(vertical_tile_mode),
        sampling_(sampling) {}
  DlImageColorSource(const DlImageColorSource& source)
      : DlImageColorSource(source.sk_image_,
                           source.matrix_,
                           source.horizontal_tile_mode_,
                           source.vertical_tile_mode_,
                           source.sampling_) {}
  DlImageColorSource(const DlImageColorSource* source)
      : DlImageColorSource(source->sk_image_,
                           source->matrix_,
                           source->horizontal_tile_mode_,
                           source->vertical_tile_mode_,
                           source->sampling_) {}

  const DlImageColorSource* asImage() const override { return this; }

  std::shared_ptr<DlColorSource> shared() const override {
    return std::make_shared<DlImageColorSource>(this);
  }

  DlColorSourceType type() const override { return DlColorSourceType::kImage; }
  size_t size() const override { return sizeof(*this); }

  bool is_opaque() const override { return sk_image_->isOpaque(); }

  sk_sp<const SkImage> image() const { return sk_image_; }
  const SkMatrix& matrix() const { return matrix_; }
  DlTileMode horizontal_tile_mode() const { return horizontal_tile_mode_; }
  DlTileMode vertical_tile_mode() const { return vertical_tile_mode_; }
  SkSamplingOptions sampling() const { return sampling_; }

  std::shared_ptr<DlColorSource> with_sampling(
      SkSamplingOptions& sampling) const override {
    return std::make_shared<DlImageColorSource>(
        sk_image_, matrix_, horizontal_tile_mode_, vertical_tile_mode_,
        sampling);
  }

  virtual sk_sp<SkShader> skia_object() const override {
    auto h_mode = static_cast<SkTileMode>(horizontal_tile_mode_);
    auto v_mode = static_cast<SkTileMode>(vertical_tile_mode_);
    return sk_image_->makeShader(h_mode, v_mode, sampling_, matrix_);
  }

 protected:
  bool equals_(DlColorSource const& other) const override {
    FML_DCHECK(other.type() == DlColorSourceType::kImage);
    auto that = static_cast<DlImageColorSource const*>(&other);
    return (sk_image_ == that->sk_image_ && matrix_ == that->matrix_ &&
            horizontal_tile_mode_ == that->horizontal_tile_mode_ &&
            vertical_tile_mode_ == that->vertical_tile_mode_);
  }

 private:
  sk_sp<SkImage> sk_image_;
  SkMatrix matrix_;
  DlTileMode horizontal_tile_mode_;
  DlTileMode vertical_tile_mode_;
  SkSamplingOptions sampling_;
};

class DlGradientColorSourceBase : public DlColorSource {
 public:
  DlGradientColorSourceBase(uint32_t stop_count,
                            std::vector<uint32_t> colors,
                            std::vector<float> stops,
                            DlTileMode tile_mode,
                            const SkMatrix& matrix)
      : mode_(tile_mode), matrix_(matrix), stop_count_(stop_count), colors_(colors), stops_(stops) {
    FML_CHECK(stop_count == stops.size() && stop_count == colors.size());
  }
  DlGradientColorSourceBase(uint32_t stop_count,
                            DlTileMode tile_mode,
                            const SkMatrix& matrix)
      : mode_(tile_mode), matrix_(matrix), stop_count_(stop_count) {}

  bool is_opaque() const override {
    if (mode_ == DlTileMode::kDecal) {
      return false;
    }
    for (uint32_t i = 0; i < stop_count_; i++) {
      if ((colors_[i] >> 24) < 255) {
        return false;
      }
    }
    return true;
  }

  DlTileMode tile_mode() const { return mode_; }
  const SkMatrix& matrix() const { return matrix_; }
  const SkMatrix* matrix_ptr() const { return &matrix_; }
  int stop_count() const { return stop_count_; }
  std::vector<uint32_t> colors() const { return colors_; }
  std::vector<float> stops() const { return stops_; }
  const uint32_t* colors_array() const {
    return reinterpret_cast<const uint32_t*>(this + 1);
  }
  const float* stops_array() const {
    return reinterpret_cast<const float*>(colors_array() + stop_count());
  }

 protected:
  uint32_t vector_sizes() const {
    // Until we store this data in the DisplayList pod regions
    // we return 0 because no allocation is needed for this
    // storage.
    // return stop_count_ * (sizeof(uint32_t) + sizeof(float));
    return 0;
  }

  bool base_equals_(DlGradientColorSourceBase const* other_base) const {
    if (mode_ != other_base->mode_ || matrix_ != other_base->matrix_ ||
        stop_count_ != other_base->stop_count_) {
      return false;
    }
    for (uint32_t i = 0; i < stop_count_; i++) {
      if (colors_[i] != other_base->colors_[i] ||
          stops_[i] != other_base->stops_[i]) {
        return false;
      }
    }
    return true;
  }

  uint32_t* unsafe_colors_array() {
    return reinterpret_cast<uint32_t*>(this + 1);
  }
  float* unsafe_stops_array() {
    return reinterpret_cast<float*>(unsafe_colors_array() + stop_count());
  }

 private:
  DlTileMode mode_;
  SkMatrix matrix_;
  uint32_t stop_count_;
  std::vector<uint32_t> colors_;
  std::vector<float> stops_;
};

class DlLinearGradientColorSource final : public DlGradientColorSourceBase {
 public:
  DlLinearGradientColorSource(const SkPoint p0,
                              const SkPoint p1,
                              uint32_t stop_count,
                              std::vector<uint32_t> colors,
                              std::vector<float> stops,
                              DlTileMode tile_mode,
                              const SkMatrix& matrix)
      : DlGradientColorSourceBase(stop_count, colors, stops, tile_mode, matrix),
        p0_(p0),
        p1_(p1) {}
  DlLinearGradientColorSource(const DlLinearGradientColorSource& source)
      : DlLinearGradientColorSource(source.p0(),
                                    source.p1(),
                                    source.stop_count(),
                                    source.colors(),
                                    source.stops(),
                                    source.tile_mode(),
                                    source.matrix()) {}
  DlLinearGradientColorSource(const DlLinearGradientColorSource* source)
      : DlLinearGradientColorSource(source->p0(),
                                    source->p1(),
                                    source->stop_count(),
                                    source->colors(),
                                    source->stops(),
                                    source->tile_mode(),
                                    source->matrix()) {}

  const DlLinearGradientColorSource* asLinearGradient() const override {
    return this;
  }

  DlColorSourceType type() const override {
    return DlColorSourceType::kLinearGradient;
  }
  size_t size() const override { return sizeof(*this) /*+ vector_sizes()*/; }

  std::shared_ptr<DlColorSource> shared() const override {
    return std::make_shared<DlLinearGradientColorSource>(
        p0_, p1_, stop_count(), colors(), stops(), tile_mode(), matrix());
  }

  const SkPoint& p0() const { return p0_; }
  const SkPoint& p1() const { return p1_; }

  sk_sp<SkShader> skia_object() const override {
    auto mode = static_cast<SkTileMode>(tile_mode());
    SkPoint pts[] = {p0_, p1_};
    return SkGradientShader::MakeLinear(pts, colors().data(), stops().data(),
                                        stop_count(), mode, 0, matrix_ptr());
  }

 protected:
  bool equals_(DlColorSource const& other) const override {
    FML_DCHECK(other.type() == DlColorSourceType::kLinearGradient);
    auto that = static_cast<DlLinearGradientColorSource const*>(&other);
    return (p0_ == that->p0_ && p1_ == that->p1_ && base_equals_(that));
  }

 private:
  DlLinearGradientColorSource(const SkPoint p0,
                              const SkPoint p1,
                              uint32_t stop_count,
                              DlTileMode tile_mode,
                              const SkMatrix& matrix)
      : DlGradientColorSourceBase(stop_count, tile_mode, matrix),
        p0_(p0),
        p1_(p1) {}

  SkPoint p0_;
  SkPoint p1_;

  friend class DlColorSource;
};

class DlRadialGradientColorSource final : public DlGradientColorSourceBase {
 public:
  DlRadialGradientColorSource(SkPoint center,
                              SkScalar radius,
                              uint32_t stop_count,
                              std::vector<uint32_t> colors,
                              std::vector<float> stops,
                              DlTileMode tile_mode,
                              const SkMatrix& matrix)
      : DlGradientColorSourceBase(stop_count, colors, stops, tile_mode, matrix),
        center_(center),
        radius_(radius) {}
  DlRadialGradientColorSource(const DlRadialGradientColorSource& source)
      : DlRadialGradientColorSource(source.center(),
                                    source.radius(),
                                    source.stop_count(),
                                    source.colors(),
                                    source.stops(),
                                    source.tile_mode(),
                                    source.matrix()) {}
  DlRadialGradientColorSource(const DlRadialGradientColorSource* source)
      : DlRadialGradientColorSource(source->center(),
                                    source->radius(),
                                    source->stop_count(),
                                    source->colors(),
                                    source->stops(),
                                    source->tile_mode(),
                                    source->matrix()) {}

  const DlRadialGradientColorSource* asRadialGradient() const override {
    return this;
  }

  std::shared_ptr<DlColorSource> shared() const override {
    return std::make_shared<DlRadialGradientColorSource>(
        center_, radius_, stop_count(), colors(), stops(), tile_mode(),
        matrix());
  }

  DlColorSourceType type() const override {
    return DlColorSourceType::kRadialGradient;
  }
  size_t size() const override { return sizeof(*this) + vector_sizes(); }

  SkPoint center() const { return center_; }
  SkScalar radius() const { return radius_; }

  sk_sp<SkShader> skia_object() const override {
    auto mode = static_cast<SkTileMode>(tile_mode());
    return SkGradientShader::MakeRadial(center_, radius_, colors().data(),
                                        stops().data(), stop_count(), mode, 0,
                                        matrix_ptr());
  }

 protected:
  bool equals_(DlColorSource const& other) const override {
    FML_DCHECK(other.type() == DlColorSourceType::kRadialGradient);
    auto that = static_cast<DlRadialGradientColorSource const*>(&other);
    return (center_ == that->center_ && radius_ == that->radius_ &&
            base_equals_(that));
  }

 private:
  SkPoint center_;
  SkScalar radius_;
};

class DlConicalGradientColorSource final : public DlGradientColorSourceBase {
 public:
  DlConicalGradientColorSource(SkPoint start_center,
                               SkScalar start_radius,
                               SkPoint end_center,
                               SkScalar end_radius,
                               uint32_t stop_count,
                               std::vector<uint32_t> colors,
                               std::vector<float> stops,
                               DlTileMode tile_mode,
                               const SkMatrix& matrix)
      : DlGradientColorSourceBase(stop_count, colors, stops, tile_mode, matrix),
        start_center_(start_center),
        start_radius_(start_radius),
        end_center_(end_center),
        end_radius_(end_radius) {}
  DlConicalGradientColorSource(const DlConicalGradientColorSource& source)
      : DlConicalGradientColorSource(source.start_center(),
                                     source.start_radius(),
                                     source.end_center(),
                                     source.end_radius(),
                                     source.stop_count(),
                                     source.colors(),
                                     source.stops(),
                                     source.tile_mode(),
                                     source.matrix()) {}
  DlConicalGradientColorSource(const DlConicalGradientColorSource* source)
      : DlConicalGradientColorSource(source->start_center(),
                                     source->start_radius(),
                                     source->end_center(),
                                     source->end_radius(),
                                     source->stop_count(),
                                     source->colors(),
                                     source->stops(),
                                     source->tile_mode(),
                                     source->matrix()) {}

  const DlConicalGradientColorSource* asConicalGradient() const override {
    return this;
  }

  std::shared_ptr<DlColorSource> shared() const override {
    return std::make_shared<DlConicalGradientColorSource>(
        start_center_, start_radius_, end_center_, end_radius_, stop_count(),
        colors(), stops(), tile_mode(), matrix());
  }

  DlColorSourceType type() const override {
    return DlColorSourceType::kSweepGradient;
  }
  size_t size() const override { return sizeof(*this) + vector_sizes(); }

  SkPoint start_center() const { return start_center_; }
  SkScalar start_radius() const { return start_radius_; }
  SkPoint end_center() const { return end_center_; }
  SkScalar end_radius() const { return end_radius_; }

  sk_sp<SkShader> skia_object() const override {
    auto mode = static_cast<SkTileMode>(tile_mode());
    return SkGradientShader::MakeTwoPointConical(
        start_center_, start_radius_, end_center_, end_radius_, colors().data(),
        stops().data(), stop_count(), mode, 0, matrix_ptr());
  }

 protected:
  bool equals_(DlColorSource const& other) const override {
    FML_DCHECK(other.type() == DlColorSourceType::kConicalGradient);
    auto that = static_cast<DlConicalGradientColorSource const*>(&other);
    return (start_center_ == that->start_center_ &&
            start_radius_ == that->start_radius_ &&
            end_center_ == that->end_center_ &&
            end_radius_ == that->end_radius_ && base_equals_(that));
  }

 private:
  SkPoint start_center_;
  SkScalar start_radius_;
  SkPoint end_center_;
  SkScalar end_radius_;
};

class DlSweepGradientColorSource final : public DlGradientColorSourceBase {
 public:
  DlSweepGradientColorSource(SkPoint center,
                             SkScalar start,
                             SkScalar end,
                             uint32_t stop_count,
                             std::vector<uint32_t> colors,
                             std::vector<float> stops,
                             DlTileMode tile_mode,
                             const SkMatrix& matrix)
      : DlGradientColorSourceBase(stop_count, colors, stops, tile_mode, matrix),
        center_(center),
        start_(start),
        end_(end) {}
  DlSweepGradientColorSource(const DlSweepGradientColorSource& source)
      : DlSweepGradientColorSource(source.center(),
                                   source.start(),
                                   source.end(),
                                   source.stop_count(),
                                   source.colors(),
                                   source.stops(),
                                   source.tile_mode(),
                                   source.matrix()) {}
  DlSweepGradientColorSource(const DlSweepGradientColorSource* source)
      : DlSweepGradientColorSource(source->center(),
                                   source->start(),
                                   source->end(),
                                   source->stop_count(),
                                   source->colors(),
                                   source->stops(),
                                   source->tile_mode(),
                                   source->matrix()) {}

  const DlSweepGradientColorSource* asSweepGradient() const override {
    return this;
  }

  std::shared_ptr<DlColorSource> shared() const override {
    return std::make_shared<DlSweepGradientColorSource>(
        center_, start_, end_, stop_count(), colors(), stops(), tile_mode(),
        matrix());
  }

  DlColorSourceType type() const override {
    return DlColorSourceType::kSweepGradient;
  }
  size_t size() const override { return sizeof(*this) + vector_sizes(); }

  SkPoint center() const { return center_; }
  SkScalar start() const { return start_; }
  SkScalar end() const { return end_; }

  sk_sp<SkShader> skia_object() const override {
    auto mode = static_cast<SkTileMode>(tile_mode());
    return SkGradientShader::MakeSweep(
        center_.x(), center_.y(), colors().data(), stops().data(), stop_count(),
        mode, start_, end_, 0, matrix_ptr());
  }

 protected:
  bool equals_(DlColorSource const& other) const override {
    FML_DCHECK(other.type() == DlColorSourceType::kSweepGradient);
    auto that = static_cast<DlSweepGradientColorSource const*>(&other);
    return (center_ == that->center_ && start_ == that->start_ &&
            end_ == that->end_ && base_equals_(that));
  }

 private:
  SkPoint center_;
  SkScalar start_;
  SkScalar end_;
};

class DlUnknownColorSource final : public DlColorSource {
 public:
  DlUnknownColorSource(sk_sp<SkShader> shader) : sk_shader_(shader) {}
  DlUnknownColorSource(const DlUnknownColorSource& filter)
      : DlUnknownColorSource(filter.sk_shader_) {}
  DlUnknownColorSource(const DlUnknownColorSource* filter)
      : DlUnknownColorSource(filter->sk_shader_) {}

  std::shared_ptr<DlColorSource> shared() const override {
    return std::make_shared<DlUnknownColorSource>(sk_shader_);
  }

  DlColorSourceType type() const override {
    return DlColorSourceType::kUnknown;
  }
  size_t size() const override { return sizeof(*this); }

  bool is_opaque() const override { return sk_shader_->isOpaque(); }

  sk_sp<SkShader> skia_object() const override { return sk_shader_; }

 protected:
  bool equals_(DlColorSource const& other) const override {
    FML_DCHECK(other.type() == DlColorSourceType::kUnknown);
    auto that = static_cast<DlUnknownColorSource const*>(&other);
    return (sk_shader_ == that->sk_shader_);
  }

 private:
  sk_sp<SkShader> sk_shader_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COLOR_SOURCE_H_
