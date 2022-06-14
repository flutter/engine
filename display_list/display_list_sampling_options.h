// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_SAMPLING_OPTIONS_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_SAMPLING_OPTIONS_H_

#include "include/core/SkColorSpace.h"
#include "include/core/SkSamplingOptions.h"
namespace flutter {

enum class DlFilterMode {
  kNearest,  // single sample point (nearest neighbor)
  kLinear,   // interporate between 2x2 sample points (bilinear interpolation)

  kLast = kLinear,
};

inline DlFilterMode ToDl(const SkFilterMode filter_mode) {
  return static_cast<DlFilterMode>(filter_mode);
}

inline SkFilterMode ToSk(const DlFilterMode filter_mode) {
  return static_cast<SkFilterMode>(filter_mode);
}

enum class DlMipmapMode {
  kNone,     // ignore mipmap levels, sample from the "base"
  kNearest,  // sample from the nearest level
  kLinear,   // interpolate between the two nearest levels

  kLast = kLinear,
};

inline DlMipmapMode ToDl(const SkMipmapMode mip_map) {
  return static_cast<DlMipmapMode>(mip_map);
}

inline SkMipmapMode ToSk(const DlMipmapMode mip_map) {
  return static_cast<SkMipmapMode>(mip_map);
}

struct DlCubicResampler {
  float B, C;

  // Historic default for kHigh_SkFilterQuality
  static constexpr DlCubicResampler Mitchell() { return {1 / 3.0f, 1 / 3.0f}; }
  static constexpr DlCubicResampler CatmullRom() { return {0.0f, 1 / 2.0f}; }
};

struct DlSamplingOptions {
  const int maxAniso = 0;
  const bool useCubic = false;
  const DlCubicResampler cubic = {0, 0};
  const DlFilterMode filter = DlFilterMode::kNearest;
  const DlMipmapMode mipmap = DlMipmapMode::kNone;

  DlSamplingOptions() = default;
  DlSamplingOptions(const DlSamplingOptions&) = default;
  DlSamplingOptions& operator=(const DlSamplingOptions& that) {
    this->~DlSamplingOptions();  // A pedantic no-op.
    new (this) DlSamplingOptions(that);
    return *this;
  }

  static DlSamplingOptions MakeNearestSampling() {
    return DlSamplingOptions(DlFilterMode::kNearest, DlMipmapMode::kNone);
  }

  static const DlSamplingOptions MakeLinearSampling() {
    static DlSamplingOptions opt =
        DlSamplingOptions(DlFilterMode::kLinear, DlMipmapMode::kNone);
    return opt;
  }

  static const DlSamplingOptions MakeMipmapSampling() {
    static DlSamplingOptions opt =
        DlSamplingOptions(DlFilterMode::kLinear, DlMipmapMode::kLinear);
    return opt;
  }

  static const DlSamplingOptions MakeCubicSampling() {
    static DlSamplingOptions opt =
        DlSamplingOptions(DlCubicResampler{1 / 3.0f, 1 / 3.0f});
    return opt;
  }

  explicit DlSamplingOptions(const SkSamplingOptions& skSamplingOptions);

  bool operator==(const DlSamplingOptions& other) const {
    return maxAniso == other.maxAniso && useCubic == other.useCubic &&
           cubic.B == other.cubic.B && cubic.C == other.cubic.C &&
           filter == other.filter && mipmap == other.mipmap;
  }
  bool operator!=(const DlSamplingOptions& other) const {
    return !(*this == other);
  }

 private:
  DlSamplingOptions(DlFilterMode fm, DlMipmapMode mm)
      : filter(fm), mipmap(mm) {}

  explicit DlSamplingOptions(const DlCubicResampler& c)
      : useCubic(true), cubic(c) {}

  friend SkSamplingOptions ToSk(const DlSamplingOptions& dlSamplingOptions);
};

inline SkSamplingOptions ToSk(const DlSamplingOptions& dlSamplingOptions) {
  if (dlSamplingOptions.useCubic) {
    return SkSamplingOptions(SkCubicResampler{1 / 3.0f, 1 / 3.0f});
  }
  return SkSamplingOptions(ToSk(dlSamplingOptions.filter),
                           ToSk(dlSamplingOptions.mipmap));
}

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_SAMPLING_OPTIONS_H_
