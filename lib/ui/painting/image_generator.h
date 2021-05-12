// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_
#define FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/painting/codec.h"
#include "third_party/skia/include/core/SkImageInfo.h"

namespace flutter {

/// @brief  The minimal interface necessary for defining a decoder that can be
///         used for single-frame image decoding. Image generators can
///         optionally support decoding into a subscaled buffer.
///         Implementers of `ImageGenerator` regularly keep internal state which
///         is not thread safe, and so aliasing and parallel access should never
///         be done with `ImageGenerator`s.
/// @see    `ImageGenerator::GetScaledDimensions`
class ImageGenerator {
 public:
  virtual ~ImageGenerator();

  /// @brief   Returns basic information about the contents of the encoded
  ///          image. This information can almost always be collected by just
  ///          interpreting the header of a decoded image.
  /// @return  Size and color information describing the image.
  /// @note    This method is executed on the UI thread and used for layout
  ///          purposes by the framework, and so this method should not perform
  ///          long synchronous tasks.
  virtual const SkImageInfo& GetInfo() const = 0;

  /// @brief      Decode the image into a given buffer.
  /// @param[in]  info       The desired size and color info of the decoded
  ///                        image to be returned. The implementation of
  ///                        `GetScaledDimensions` determines which sizes are
  ///                        supported by the image decoder.
  /// @param[in]  pixels     The location where the raw decoded image data
  ///                        should be written.
  /// @param[in]  row_bytes  The total number of bytes that should make up a
  ///                        single row of decoded image data
  ///                        (i.e. width * bytes_per_pixel).
  /// @return     True if the image was successfully decoded.
  /// @note       This method performs potentially long synchronous work, and so
  ///             it should never be executed on the UI thread. Image decoders
  ///             do not require GPU acceleration, and so threads without a GPU
  ///             context may also be used.
  /// @see        `GetScaledDimensions`
  virtual bool GetPixels(const SkImageInfo& info,
                         void* pixels,
                         size_t row_bytes) const = 0;

  /// @brief      Given a scale value, find the closest image size that can be
  ///             used for efficiently decoding the image. If subpixel image
  ///             decoding is not supported by the decoder, this method should
  ///             just return the original image size.
  /// @param[in]  scale  The desired scale factor of the image for decoding.
  /// @return     The closest image size that can be used for efficiently
  ///             decoding the image.
  /// @note       This method is called prior to `GetPixels` in order to query
  ///             for supported sizes.
  /// @see        `GetPixels`
  virtual SkISize GetScaledDimensions(float scale) const = 0;
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
                 size_t row_bytes) const override;

  // |ImageGenerator|
  SkISize GetScaledDimensions(float desired_scale) const override;

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
                 size_t row_bytes) const override;

  // |ImageGenerator|
  SkISize GetScaledDimensions(float desired_scale) const override;

  static std::unique_ptr<ImageGenerator> MakeFromData(sk_sp<SkData> data);

 private:
  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(BuiltinSkiaCodecImageGenerator);
  std::unique_ptr<SkCodecImageGenerator> codec_generator_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_H_
