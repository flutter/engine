// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_surface_software.h"

#include "flutter/fml/trace_event.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

EmbedderSurfaceSoftware::EmbedderSurfaceSoftware(
    const FlutterSoftwareRendererConfig& render_config,
    SoftwareDispatchTable software_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : software_dispatch_table_(software_dispatch_table),
      external_view_embedder_(external_view_embedder),
      render_config_(render_config) {
  if (!software_dispatch_table_.software_present_backing_store) {
    return;
  }
  valid_ = true;
}

EmbedderSurfaceSoftware::~EmbedderSurfaceSoftware() = default;

// |EmbedderSurface|
bool EmbedderSurfaceSoftware::IsValid() const {
  return valid_;
}

// |EmbedderSurface|
std::unique_ptr<Surface> EmbedderSurfaceSoftware::CreateGPUSurface() {
  if (!IsValid()) {
    return nullptr;
  }
  const bool render_to_surface = !external_view_embedder_;
  auto surface = std::make_unique<GPUSurfaceSoftware>(this, render_to_surface);

  if (!surface->IsValid()) {
    return nullptr;
  }

  return surface;
}

// |EmbedderSurface|
sk_sp<GrDirectContext> EmbedderSurfaceSoftware::CreateResourceContext() const {
  return nullptr;
}

// |GPUSurfaceSoftwareDelegate|
sk_sp<SkSurface> EmbedderSurfaceSoftware::AcquireBackingStore(
    const SkISize& size) {
  TRACE_EVENT0("flutter", "EmbedderSurfaceSoftware::AcquireBackingStore");
  if (!IsValid()) {
    FML_LOG(ERROR)
        << "Could not acquire backing store for the software surface.";
    return nullptr;
  }

  if (sk_surface_ != nullptr &&
      SkISize::Make(sk_surface_->width(), sk_surface_->height()) == size) {
    // The old and new surface sizes are the same. Nothing to do here.
    return sk_surface_;
  }

  SkImageInfo info;
  if (render_config_.color_type >= kSoftAlpha_8_SkColorType &&
      render_config_.color_type <= kSoftRGBA_F16_SkColorType) {
    info =
        SkImageInfo::Make(size.fWidth, size.fHeight,
                          static_cast<SkColorType>(render_config_.color_type),
                          kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
  }

  else {
    info = SkImageInfo::MakeN32(size.fWidth, size.fHeight, kPremul_SkAlphaType,
                                SkColorSpace::MakeSRGB());
  }

  sk_surface_ = SkSurface::MakeRaster(info, nullptr);

  if (sk_surface_ == nullptr) {
    FML_LOG(ERROR) << "Could not create backing store for software rendering.";
    return nullptr;
  }

  return sk_surface_;
}

// |GPUSurfaceSoftwareDelegate|
bool EmbedderSurfaceSoftware::PresentBackingStore(
    sk_sp<SkSurface> backing_store) {
  if (!IsValid()) {
    FML_LOG(ERROR) << "Tried to present an invalid software surface.";
    return false;
  }

  SkPixmap pixmap;
  if (!backing_store->peekPixels(&pixmap)) {
    FML_LOG(ERROR) << "Could not peek the pixels of the backing store.";
    return false;
  }

  int bit_per_pixel = 4;

  if (render_config_.color_type >= kSoftAlpha_8_SkColorType &&
      render_config_.color_type <= kSoftRGBA_F16_SkColorType) {
    switch (render_config_.color_type) {
      case kSoftAlpha_8_SkColorType:  //!< pixel with alpha in 8-bit byte
      case kSoftGray_8_SkColorType:   //!< pixel with grayscale level in 8-bit
                                      //!< byte
        bit_per_pixel = 1;
        break;
      case kSoftRGB_565_SkColorType:  //!< pixel with 5 bits red, 6 bits green,
                                      //!< 5 bits blue, in 16-bit word
      case kSoftARGB_4444_SkColorType:  //!< pixel with 4 bits for alpha, red,
                                        //!< green, blue; in 16-bit word
        bit_per_pixel = 2;
        break;
      case kSoftRGBA_8888_SkColorType:  //!< pixel with 8 bits for red, green,
                                        //!< blue, alpha; in 32-bit word
      case kSoftRGB_888x_SkColorType:   //!< pixel with 8 bits each for red,
                                        //!< green, blue; in 32-bit word
      case kSoftBGRA_8888_SkColorType:  //!< pixel with 8 bits for blue, green,
                                        //!< red, alpha; in 32-bit word
      case kSoftRGBA_1010102_SkColorType:  //!< 10 bits for red, green, blue; 2
                                           //!< bits for alpha; in 32-bit word
      case kSoftBGRA_1010102_SkColorType:  //!< 10 bits for blue, green, red; 2
                                           //!< bits for alpha; in 32-bit word
      case kSoftRGB_101010x_SkColorType:   //!< pixel with 10 bits each for red,
                                           //!< green, blue; in 32-bit word
      case kSoftBGR_101010x_SkColorType:  //!< pixel with 10 bits each for blue,
                                          //!< green, red; in 32-bit word
        bit_per_pixel = 4;
        break;
      case kSoftRGBA_F16_SkColorType:  //!< pixel with half floats for red,
                                       //!< green, blue, alpha; in 64-bit word
        bit_per_pixel = 8;
        break;
      case kSoftUnknown_SkColorType:
      default:
        FML_LOG(ERROR) << "Software surface unsupport  colorType"
                       << render_config_.color_type;
        return false;
    }
  }

  // Some basic sanity checking.
  uint64_t expected_pixmap_data_size =
      pixmap.width() * pixmap.height() * bit_per_pixel;

  const size_t pixmap_size = pixmap.computeByteSize();

  if (expected_pixmap_data_size != pixmap_size) {
    FML_LOG(ERROR) << "Software backing store had unexpected size.";
    return false;
  }

  return software_dispatch_table_.software_present_backing_store(
      pixmap.addr(),      //
      pixmap.rowBytes(),  //
      pixmap.height()     //
  );
}

}  // namespace flutter
