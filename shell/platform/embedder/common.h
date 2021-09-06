// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_COMMON_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_COMMON_H_

#include <cstdlib>
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/platform/embedder/embedder.h"

static inline SkColorType getSkColorType(FlutterSoftwarePixelFormat pixfmt) {
  switch (pixfmt) {
    case kGray8:
      return kGray_8_SkColorType;
    case kRGB565:
      return kRGB_565_SkColorType;
    case kRGBA4444:
      return kARGB_4444_SkColorType;
    case kRGBA8888:
      return kRGBA_8888_SkColorType;
    case kRGBX8888:
      return kRGB_888x_SkColorType;
    case kBGRA8888:
      return kBGRA_8888_SkColorType;
    case kNative32:
      return kN32_SkColorType;
    default:
      FML_LOG(ERROR) << "Invalid software rendering pixel format";
      std::abort();
  }
}

static inline SkColorInfo getSkColorInfo(FlutterSoftwarePixelFormat pixfmt) {
  auto ct = getSkColorType(pixfmt);
  auto at =
      SkColorTypeIsAlwaysOpaque(ct) ? kOpaque_SkAlphaType : kPremul_SkAlphaType;

  return SkColorInfo(ct, at, SkColorSpace::MakeSRGB());
}

#endif
