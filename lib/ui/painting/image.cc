// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image.h"

#include <algorithm>
#include <limits>

#if IMPELLER_SUPPORTS_RENDERING
#include "flutter/impeller/renderer/texture.h"
#endif
#include "flutter/lib/ui/painting/image_encoding.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"

// Must be kept in sync with painting.dart.
enum ColorSpace {
  kSRGB,
  kExtendedSRGB,
};

namespace flutter {

typedef CanvasImage Image;

// Since _Image is a private class, we can't use IMPLEMENT_WRAPPERTYPEINFO
static const tonic::DartWrapperInfo kDartWrapperInfoUIImage("ui", "_Image");
const tonic::DartWrapperInfo& Image::dart_wrapper_info_ =
    kDartWrapperInfoUIImage;

CanvasImage::CanvasImage() = default;

CanvasImage::~CanvasImage() = default;

Dart_Handle CanvasImage::toByteData(int format, Dart_Handle callback) {
  return EncodeImage(this, format, callback);
}

void CanvasImage::dispose() {
  image_.reset();
  ClearDartWrapper();
}

int CanvasImage::colorSpace() {
  if (image_->skia_image()) {
    return ColorSpace::kSRGB;
  } else if (image_->impeller_texture()) {
#if IMPELLER_SUPPORTS_RENDERING
    const impeller::TextureDescriptor& desc =
        image_->impeller_texture()->GetTextureDescriptor();
    switch (desc.format) {
      case impeller::PixelFormat::kB10G10R10XR:  // intentional_fallthrough
      case impeller::PixelFormat::kR16G16B16A16Float:
        return ColorSpace::kExtendedSRGB;
      default:
        return ColorSpace::kSRGB;
    }
#endif  // IMPELLER_SUPPORTS_RENDERING
  }

  return -1;
}

}  // namespace flutter
