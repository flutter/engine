// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_
#define FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_

#include "flutter/fml/macros.h"
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
  virtual ~ImageGenerator();

  virtual const SkImageInfo& GetInfo() const = 0;

  virtual bool GetPixels(const SkImageInfo& info,
                         void* pixels,
                         size_t rowBytes) const = 0;

  /// Given a scale value, returns the closest image size that can be used for
  /// efficiently decoding the image. If subpixel image decoding is not
  /// supported by the codec, this method just returns the original image size.
  virtual SkISize GetScaledDimensions(float scale) = 0;
};

class BuiltinSkiaImageGenerator : public ImageGenerator {
 public:
  ~BuiltinSkiaImageGenerator();

  BuiltinSkiaImageGenerator(std::unique_ptr<SkImageGenerator> generator);

  // |ImageGenerator|
  const SkImageInfo& GetInfo() const override;

  // |ImageGenerator|
  bool GetPixels(const SkImageInfo& info,
                 void* pixels,
                 size_t rowBytes) const override;

  // |ImageGenerator|
  SkISize GetScaledDimensions(float desiredScale) override;

  static std::unique_ptr<ImageGenerator> MakeFromGenerator(
      std::unique_ptr<SkImageGenerator> generator);

 private:
  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(BuiltinSkiaImageGenerator);
  std::unique_ptr<SkImageGenerator> generator_;
};

class BuiltinSkiaCodecImageGenerator : public ImageGenerator {
 public:
  ~BuiltinSkiaCodecImageGenerator();

  BuiltinSkiaCodecImageGenerator(std::unique_ptr<SkCodec> codec);

  BuiltinSkiaCodecImageGenerator(sk_sp<SkData> buffer);

  // |ImageGenerator|
  const SkImageInfo& GetInfo() const override;

  // |ImageGenerator|
  bool GetPixels(const SkImageInfo& info,
                 void* pixels,
                 size_t rowBytes) const override;

  // |ImageGenerator|
  SkISize GetScaledDimensions(float desiredScale) override;

  static std::unique_ptr<ImageGenerator> MakeFromData(sk_sp<SkData> data);

 private:
  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(BuiltinSkiaCodecImageGenerator);
  std::unique_ptr<SkCodecImageGenerator> codec_generator_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_
