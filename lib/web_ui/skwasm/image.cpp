// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"

#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkPicture.h"

using namespace SkImages;

SKWASM_EXPORT SkImage* image_createFromPicture(SkPicture* picture,
                                               int32_t width,
                                               int32_t height) {
  picture->ref();
  return DeferredFromPicture(sk_sp<SkPicture>(picture), {width, height},
                             nullptr, nullptr, BitDepth::kU8, nullptr)
      .release();
}

SKWASM_EXPORT void image_dispose(SkImage* image) {
  image->unref();
}

SKWASM_EXPORT int image_getWidth(SkImage* image) {
  return image->width();
}

SKWASM_EXPORT int image_getHeight(SkImage* image) {
  return image->height();
}
