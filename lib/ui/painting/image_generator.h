// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_
#define FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_

#include "flutter/lib/ui/painting/codec.h"
#include "third_party/skia/include/core/SkImageInfo.h"

namespace flutter {

/// The minimal interface necessary for creating decoders that can be used for
/// single-frame image decoding with optional support for decoding into a
/// subscaled buffer.
///
/// See `getScaledDimensions` for details about how subpixel image decoding can
/// be supported.
class ImageGenerator {
 public:
  virtual ~ImageGenerator(){};

  virtual const SkImageInfo& getInfo() const = 0;

  virtual bool getPixels(const SkImageInfo& info,
                         void* pixels,
                         size_t rowBytes) const = 0;

  /// Given a scale value, returns the closest image size that can be used for
  /// efficiently decoding the image. If subpixel image decoding is not
  /// supported by the codec, this method just returns the original image size.
  virtual SkISize getScaledDimensions(float scale) = 0;
};

class BuiltinSkiaImageGenerator : public ImageGenerator {
 public:
  BuiltinSkiaImageGenerator(std::unique_ptr<SkImageGenerator> generator);

  ~BuiltinSkiaImageGenerator() = default;

  const SkImageInfo& getInfo() const override;

  bool getPixels(const SkImageInfo& info,
                 void* pixels,
                 size_t rowBytes) const override;

  SkISize getScaledDimensions(float desiredScale) override;

  static std::unique_ptr<ImageGenerator> makeFromGenerator(
      std::unique_ptr<SkImageGenerator> generator);

 private:
  BuiltinSkiaImageGenerator() = delete;
  std::unique_ptr<SkImageGenerator> generator_;
};

class BuiltinSkiaCodecImageGenerator : public ImageGenerator {
 public:
  BuiltinSkiaCodecImageGenerator(std::unique_ptr<SkCodec> codec);

  BuiltinSkiaCodecImageGenerator(sk_sp<SkData> buffer);

  ~BuiltinSkiaCodecImageGenerator() = default;

  const SkImageInfo& getInfo() const override;

  bool getPixels(const SkImageInfo& info,
                 void* pixels,
                 size_t rowBytes) const override;

  SkISize getScaledDimensions(float desiredScale) override;

  static std::unique_ptr<ImageGenerator> makeFromData(sk_sp<SkData> data);

 private:
  BuiltinSkiaCodecImageGenerator() = delete;
  std::unique_ptr<SkCodecImageGenerator> codec_generator_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_
