// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_generator.h"

namespace flutter {

ImageGenerator::~ImageGenerator() = default;

BuiltinSkiaImageGenerator::~BuiltinSkiaImageGenerator() = default;

BuiltinSkiaImageGenerator::BuiltinSkiaImageGenerator(
    std::unique_ptr<SkImageGenerator> generator)
    : generator_(std::move(generator)) {}

const SkImageInfo& BuiltinSkiaImageGenerator::GetInfo() const {
  return generator_->getInfo();
}

bool BuiltinSkiaImageGenerator::GetPixels(const SkImageInfo& info,
                                          void* pixels,
                                          size_t row_bytes) const {
  return generator_->getPixels(info, pixels, row_bytes);
}

SkISize BuiltinSkiaImageGenerator::GetScaledDimensions(
    float desired_scale) const {
  return generator_->getInfo().dimensions();
}

BuiltinSkiaCodecImageGenerator::~BuiltinSkiaCodecImageGenerator() = default;

std::unique_ptr<ImageGenerator> BuiltinSkiaImageGenerator::MakeFromGenerator(
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

const SkImageInfo& BuiltinSkiaCodecImageGenerator::GetInfo() const {
  return codec_generator_->getInfo();
}

bool BuiltinSkiaCodecImageGenerator::GetPixels(const SkImageInfo& info,
                                               void* pixels,
                                               size_t row_bytes) const {
  return codec_generator_->getPixels(info, pixels, row_bytes);
}

SkISize BuiltinSkiaCodecImageGenerator::GetScaledDimensions(
    float desired_scale) const {
  return codec_generator_->getScaledDimensions(desired_scale);
}

std::unique_ptr<ImageGenerator> BuiltinSkiaCodecImageGenerator::MakeFromData(
    sk_sp<SkData> data) {
  auto codec = SkCodec::MakeFromData(data);
  if (!codec) {
    return nullptr;
  }
  return std::make_unique<BuiltinSkiaCodecImageGenerator>(std::move(codec));
}

}  // namespace flutter
