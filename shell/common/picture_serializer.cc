// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/picture_serializer.h"

#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkPixelSerializer.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/encode/SkPngEncoder.h"

namespace shell {

bool PngPixelSerializer::onUseEncodedData(const void*, size_t) {
  return true;
}

SkData* PngPixelSerializer::onEncode(const SkPixmap& pixmap) {
  SkDynamicMemoryWStream stream;

  SkPngEncoder::Options options;
  options.fUnpremulBehavior = SkTransferFunctionBehavior::kRespect;
  bool encode_result = SkPngEncoder::Encode(&stream, pixmap, options);

  return encode_result ? stream.detachAsData().release() : nullptr;
}

void SerializePicture(const std::string& path, SkPicture* picture) {
  SkFILEWStream stream(path.c_str());
  PngPixelSerializer serializer;
  picture->serialize(&stream, &serializer);
}

}  // namespace shell
