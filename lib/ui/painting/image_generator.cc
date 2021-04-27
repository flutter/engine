// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_generator.h"

namespace flutter {

BuiltinSkiaImageGenerator::BuiltinSkiaImageGenerator(
    std::unique_ptr<SkImageGenerator> generator)
    : generator_(std::move(generator)) {}

const SkImageInfo& BuiltinSkiaImageGenerator::getInfo() const {
  return generator_->getInfo();
}

bool BuiltinSkiaImageGenerator::getPixels(const SkImageInfo& info,
                                          void* pixels,
                                          size_t rowBytes) const {
  return generator_->getPixels(info, pixels, rowBytes);
}

SkISize BuiltinSkiaImageGenerator::getScaledDimensions(float desiredScale) {
  return generator_->getInfo().dimensions();
}

std::unique_ptr<ImageGenerator> BuiltinSkiaImageGenerator::makeFromGenerator(
    std::unique_ptr<SkImageGenerator> generator) {
  if (!generator) {
    return nullptr;
  }
  return std::make_unique<BuiltinSkiaImageGenerator>(std::move(generator));
}

BuiltinSkiaCodecImageGenerator::BuiltinSkiaCodecImageGenerator(
    std::unique_ptr<SkCodec> codec)
    : codec_generator_(static_cast<SkCodecImageGenerator*>(
          SkCodecImageGenerator::MakeFromCodec(std::move(codec)).release())) {}

BuiltinSkiaCodecImageGenerator::BuiltinSkiaCodecImageGenerator(
    sk_sp<SkData> buffer)
    : codec_generator_(static_cast<SkCodecImageGenerator*>(
          SkCodecImageGenerator::MakeFromEncodedCodec(buffer).release())) {}

const SkImageInfo& BuiltinSkiaCodecImageGenerator::getInfo() const {
  return codec_generator_->getInfo();
}

bool BuiltinSkiaCodecImageGenerator::getPixels(const SkImageInfo& info,
                                               void* pixels,
                                               size_t rowBytes) const {
  return codec_generator_->getPixels(info, pixels, rowBytes);
}

SkISize BuiltinSkiaCodecImageGenerator::getScaledDimensions(
    float desiredScale) {
  return codec_generator_->getScaledDimensions(desiredScale);
}

std::unique_ptr<ImageGenerator> BuiltinSkiaCodecImageGenerator::makeFromData(
    sk_sp<SkData> data) {
  auto codec = SkCodec::MakeFromData(data);
  if (!codec) {
    return nullptr;
  }
  return std::make_unique<BuiltinSkiaCodecImageGenerator>(std::move(codec));
}

}  // namespace flutter
