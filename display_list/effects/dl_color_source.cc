// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_color_source.h"

#include "flutter/display_list/dl_sampling_options.h"
#include "flutter/display_list/effects/dl_runtime_effect.h"
#include "flutter/fml/logging.h"

namespace flutter {

static void DlGradientDeleter(void* p) {
  // Some of our target environments would prefer a sized delete,
  // but other target environments do not have that operator.
  // Use an unsized delete until we get better agreement in the
  // environments.
  // See https://github.com/flutter/flutter/issues/100327
  ::operator delete(p);
}

std::shared_ptr<DlColorSource> DlColorSource::MakeColor(DlColor color) {
  return DlColorColorSource::Make(color);
}

std::shared_ptr<DlColorColorSource> DlColorColorSource::Make(DlColor color) {
  return std::shared_ptr<DlColorColorSource>(new DlColorColorSource(color));
}

std::shared_ptr<DlColorSource> DlColorSource::MakeImage(
    const sk_sp<const DlImage>& image,
    DlTileMode horizontal_tile_mode,
    DlTileMode vertical_tile_mode,
    DlImageSampling sampling,
    const SkMatrix* matrix) {
  return DlImageColorSource::Make(image, horizontal_tile_mode,
                                  vertical_tile_mode, sampling, matrix);
}

std::shared_ptr<DlImageColorSource> DlImageColorSource::Make(
    const sk_sp<const DlImage>& image,
    DlTileMode horizontal_tile_mode,
    DlTileMode vertical_tile_mode,
    DlImageSampling sampling,
    const SkMatrix* matrix) {
  return std::shared_ptr<DlImageColorSource>(new DlImageColorSource(
      image, horizontal_tile_mode, vertical_tile_mode, sampling, matrix));
}

std::shared_ptr<DlColorSource> DlColorSource::MakeLinear(
    const SkPoint start_point,
    const SkPoint end_point,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix) {
  return DlLinearGradientColorSource::Make(start_point, end_point, stop_count,
                                           colors, stops, tile_mode, matrix);
}

std::shared_ptr<DlLinearGradientColorSource> DlLinearGradientColorSource::Make(
    const SkPoint start_point,
    const SkPoint end_point,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix) {
  size_t needed = sizeof(DlLinearGradientColorSource) +
                  (stop_count * (sizeof(uint32_t) + sizeof(float)));

  void* storage = ::operator new(needed);

  return std::shared_ptr<DlLinearGradientColorSource>(  //
      new (storage) DlLinearGradientColorSource(
          start_point, end_point, stop_count, colors, stops, tile_mode, matrix),
      DlGradientDeleter);
}

std::shared_ptr<DlColorSource> DlColorSource::MakeRadial(
    SkPoint center,
    SkScalar radius,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix) {
  return DlRadialGradientColorSource::Make(center, radius, stop_count,  //
                                           colors, stops, tile_mode, matrix);
}

std::shared_ptr<DlRadialGradientColorSource> DlRadialGradientColorSource::Make(
    SkPoint center,
    SkScalar radius,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix) {
  size_t needed = sizeof(DlRadialGradientColorSource) +
                  (stop_count * (sizeof(uint32_t) + sizeof(float)));

  void* storage = ::operator new(needed);

  return std::shared_ptr<DlRadialGradientColorSource>(
      new (storage) DlRadialGradientColorSource(
          center, radius, stop_count, colors, stops, tile_mode, matrix),
      DlGradientDeleter);
}

std::shared_ptr<DlColorSource> DlColorSource::MakeConical(
    SkPoint start_center,
    SkScalar start_radius,
    SkPoint end_center,
    SkScalar end_radius,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix) {
  return DlConicalGradientColorSource::Make(start_center, start_radius,
                                            end_center, end_radius, stop_count,
                                            colors, stops, tile_mode, matrix);
}

std::shared_ptr<DlConicalGradientColorSource>
DlConicalGradientColorSource::Make(SkPoint start_center,
                                   SkScalar start_radius,
                                   SkPoint end_center,
                                   SkScalar end_radius,
                                   uint32_t stop_count,
                                   const DlColor* colors,
                                   const float* stops,
                                   DlTileMode tile_mode,
                                   const SkMatrix* matrix) {
  size_t needed = sizeof(DlConicalGradientColorSource) +
                  (stop_count * (sizeof(uint32_t) + sizeof(float)));

  void* storage = ::operator new(needed);

  return std::shared_ptr<DlConicalGradientColorSource>(
      new (storage) DlConicalGradientColorSource(
          start_center, start_radius, end_center, end_radius,  //
          stop_count, colors, stops, tile_mode, matrix),
      DlGradientDeleter);
}

std::shared_ptr<DlColorSource> DlColorSource::MakeSweep(
    SkPoint center,
    SkScalar start,
    SkScalar end,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix) {
  return DlSweepGradientColorSource::Make(center, start, end, stop_count,
                                          colors, stops, tile_mode, matrix);
}

std::shared_ptr<DlSweepGradientColorSource> DlSweepGradientColorSource::Make(
    SkPoint center,
    SkScalar start,
    SkScalar end,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix) {
  size_t needed = sizeof(DlSweepGradientColorSource) +
                  (stop_count * (sizeof(uint32_t) + sizeof(float)));

  void* storage = ::operator new(needed);

  return std::shared_ptr<DlSweepGradientColorSource>(
      new (storage) DlSweepGradientColorSource(
          center, start, end, stop_count, colors, stops, tile_mode, matrix),
      DlGradientDeleter);
}

std::shared_ptr<DlColorSource> DlColorSource::MakeRuntimeEffect(
    const std::shared_ptr<DlRuntimeEffect>& runtime_effect,
    const std::vector<std::shared_ptr<DlColorSource>>& samplers,
    const std::shared_ptr<std::vector<uint8_t>>& uniform_data) {
  return DlRuntimeEffectColorSource::Make(runtime_effect, samplers,
                                          uniform_data);
}

std::shared_ptr<DlRuntimeEffectColorSource> DlRuntimeEffectColorSource::Make(
    const std::shared_ptr<DlRuntimeEffect>& runtime_effect,
    const std::vector<std::shared_ptr<DlColorSource>>& samplers,
    const std::shared_ptr<std::vector<uint8_t>>& uniform_data) {
  FML_DCHECK(uniform_data != nullptr);
  return std::shared_ptr<DlRuntimeEffectColorSource>(
      new DlRuntimeEffectColorSource(runtime_effect, samplers, uniform_data));
}

#ifdef IMPELLER_ENABLE_3D
std::shared_ptr<DlColorSource> DlColorSource::MakeScene(
    const std::shared_ptr<impeller::scene::Node>& node,
    impeller::Matrix camera_matrix) {
  return DlSceneColorSource::Make(node, camera_matrix);
}

std::shared_ptr<DlSceneColorSource> DlSceneColorSource::Make(
    const std::shared_ptr<impeller::scene::Node>& node,
    impeller::Matrix camera_matrix) {
  return std::shared_ptr<DlSceneColorSource>(
      new DlSceneColorSource(node, camera_matrix));
}
#endif  // IMPELLER_ENABLE_3D

}  // namespace flutter
