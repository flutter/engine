// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_color_source.h"

#include "flutter/display_list/dl_sampling_options.h"
#include "flutter/display_list/effects/dl_runtime_effect.h"
#include "flutter/fml/logging.h"

namespace flutter {

dl_shared<DlColorSource> DlColorSource::MakeColor(DlColor color) {
  return DlColorColorSource::Make(color);
}

dl_shared<DlColorColorSource> DlColorColorSource::Make(DlColor color) {
  return dl_shared(new DlColorColorSource(color));
}

dl_shared<DlColorSource> DlColorSource::MakeImage(
    sk_sp<const DlImage> image,
    DlTileMode horizontal_tile_mode,
    DlTileMode vertical_tile_mode,
    DlImageSampling sampling,
    const SkMatrix* matrix) {
  return DlImageColorSource::Make(image, horizontal_tile_mode,
                                  vertical_tile_mode, sampling, matrix);
}

dl_shared<DlImageColorSource> DlImageColorSource::Make(
    sk_sp<const DlImage> image,
    DlTileMode horizontal_tile_mode,
    DlTileMode vertical_tile_mode,
    DlImageSampling sampling,
    const SkMatrix* matrix) {
  return dl_shared(new DlImageColorSource(
      image, horizontal_tile_mode, vertical_tile_mode, sampling, matrix));
}

dl_shared<DlColorSource> DlColorSource::MakeLinear(const SkPoint start_point,
                                                   const SkPoint end_point,
                                                   uint32_t stop_count,
                                                   const DlColor* colors,
                                                   const float* stops,
                                                   DlTileMode tile_mode,
                                                   const SkMatrix* matrix) {
  return DlLinearGradientColorSource::Make(start_point, end_point, stop_count,
                                           colors, stops, tile_mode, matrix);
}

dl_shared<DlLinearGradientColorSource> DlLinearGradientColorSource::Make(
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

  return dl_shared(new (storage) DlLinearGradientColorSource(
      start_point, end_point, stop_count, colors, stops, tile_mode, matrix));
}

dl_shared<DlColorSource> DlColorSource::MakeRadial(SkPoint center,
                                                   SkScalar radius,
                                                   uint32_t stop_count,
                                                   const DlColor* colors,
                                                   const float* stops,
                                                   DlTileMode tile_mode,
                                                   const SkMatrix* matrix) {
  return DlRadialGradientColorSource::Make(center, radius, stop_count,  //
                                           colors, stops, tile_mode, matrix);
}

dl_shared<DlRadialGradientColorSource> DlRadialGradientColorSource::Make(
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

  return dl_shared(new (storage) DlRadialGradientColorSource(
      center, radius, stop_count, colors, stops, tile_mode, matrix));
}

dl_shared<DlColorSource> DlColorSource::MakeConical(SkPoint start_center,
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

dl_shared<DlConicalGradientColorSource> DlConicalGradientColorSource::Make(
    SkPoint start_center,
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

  return dl_shared(new (storage) DlConicalGradientColorSource(
      start_center, start_radius, end_center, end_radius,  //
      stop_count, colors, stops, tile_mode, matrix));
}

dl_shared<DlColorSource> DlColorSource::MakeSweep(SkPoint center,
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

dl_shared<DlSweepGradientColorSource> DlSweepGradientColorSource::Make(
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

  return dl_shared(new (storage) DlSweepGradientColorSource(
      center, start, end, stop_count, colors, stops, tile_mode, matrix));
}

dl_shared<DlColorSource> DlColorSource::MakeRuntimeEffect(
    dl_shared<DlRuntimeEffect> runtime_effect,
    std::vector<dl_shared<DlColorSource>> samplers,
    std::shared_ptr<std::vector<uint8_t>> uniform_data) {
  return DlRuntimeEffectColorSource::Make(runtime_effect, samplers,
                                          uniform_data);
}

dl_shared<DlRuntimeEffectColorSource> DlRuntimeEffectColorSource::Make(
    dl_shared<DlRuntimeEffect> runtime_effect,
    std::vector<dl_shared<DlColorSource>> samplers,
    std::shared_ptr<std::vector<uint8_t>> uniform_data) {
  FML_DCHECK(uniform_data != nullptr);
  return dl_shared(new DlRuntimeEffectColorSource(
      std::move(runtime_effect), std::move(samplers), std::move(uniform_data)));
}

#ifdef IMPELLER_ENABLE_3D
dl_shared<DlColorSource> DlColorSource::MakeScene(
    std::shared_ptr<impeller::scene::Node> node,
    impeller::Matrix camera_matrix) {
  return DlSceneColorSource::Make(node, camera_matrix);
}

dl_shared<DlSceneColorSource> DlSceneColorSource::Make(
    std::shared_ptr<impeller::scene::Node> node,
    impeller::Matrix camera_matrix) {
  return dl_shared(new DlSceneColorSource(node, camera_matrix));
}
#endif  // IMPELLER_ENABLE_3D

}  // namespace flutter
