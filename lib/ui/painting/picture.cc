// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/picture.h"

#include "flutter/lib/ui/painting/canvas.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/window.h"
#include "lib/tonic/converter/dart_converter.h"
#include "lib/tonic/dart_args.h"
#include "lib/tonic/dart_binding_macros.h"
#include "lib/tonic/dart_library_natives.h"
#include "third_party/skia/include/core/SkImage.h"

namespace blink {

IMPLEMENT_WRAPPERTYPEINFO(ui, Picture);

#define FOR_EACH_BINDING(V) \
  V(Picture, toImage)       \
  V(Picture, dispose)

DART_BIND_ALL(Picture, FOR_EACH_BINDING)

fxl::RefPtr<Picture> Picture::Create(flow::SkiaGPUObject<SkPicture> picture,
                                     bool scaled_to_device) {
  return fxl::MakeRefCounted<Picture>(std::move(picture), scaled_to_device);
}

Picture::Picture(flow::SkiaGPUObject<SkPicture> picture, bool scaled_to_device)
    : picture_(std::move(picture)), scaled_to_device_(scaled_to_device) {}

Picture::~Picture() = default;

fxl::RefPtr<CanvasImage> Picture::toImage(int width, int height) {
  SkISize size = SkISize::Make(width, height);
  if (scaled_to_device_) {
    SkScalar dpr =
        UIDartState::Current()->window()->viewport_metrics().device_pixel_ratio;
    size.set(ceil(width * dpr), ceil(height * dpr));
  }
  fxl::RefPtr<CanvasImage> image = CanvasImage::Create();
  image->set_image(UIDartState::CreateGPUObject(SkImage::MakeFromPicture(
      picture_.get(), size, nullptr, nullptr, SkImage::BitDepth::kU8,
      SkColorSpace::MakeSRGB())));
  return image;
}

void Picture::dispose() {
  ClearDartWrapper();
}

size_t Picture::GetAllocationSize() {
  if (auto picture = picture_.get()) {
    return picture->approximateBytesUsed();
  } else {
    return sizeof(Picture);
  }
}

}  // namespace blink
