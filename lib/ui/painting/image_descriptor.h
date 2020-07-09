// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_IMAGE_DESCRIPTOR_H_
#define FLUTTER_LIB_UI_PAINTING_IMAGE_DESCRIPTOR_H_

#include <cstdint>
#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/immutable_buffer.h"
#include "third_party/skia/include/codec/SkCodec.h"
#include "third_party/skia/include/core/SkImageGenerator.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

class ImageDescriptor : public RefCountedDartWrappable<ImageDescriptor> {
 public:
  ~ImageDescriptor() override = default;

  // This must be kept in sync with the enum in painting.dart
  enum PixelFormat {
    kRGBA8888,
    kBGRA8888,
  };

  static void initEncoded(Dart_NativeArguments args);

  static void initRaw(Dart_Handle descriptor_handle,
                      fml::RefPtr<ImmutableBuffer> data,
                      int width,
                      int height,
                      int row_bytes,
                      PixelFormat pixel_format);

  void instantiateCodec(Dart_Handle callback,
                        int target_width,
                        int target_height);

  int width() const { return image_info_.width(); }

  int height() const { return image_info_.height(); }

  int bytesPerPixel() const { return image_info_.bytesPerPixel(); }

  int row_bytes() const {
    return row_bytes_.value_or(
        static_cast<size_t>(image_info_.width() * image_info_.bytesPerPixel()));
  }

  bool should_resize(int target_width, int target_height) const {
    return target_width != width() || target_height != height();
  }

  sk_sp<SkData> data() const { return buffer_; }

  bool is_compressed() const { return !!codec_; }

  const std::shared_ptr<SkCodec> codec() const { return codec_; }

  const SkImageInfo& image_info() const { return image_info_; }

  bool get_pixels(const SkPixmap& pixmap) const {
    if (image_generator_) {
      return image_generator_->getPixels(pixmap.info(), pixmap.writable_addr(),
                                         pixmap.rowBytes());
    }
    if (codec_) {
      return codec_->getPixels(pixmap.info(), pixmap.writable_addr(),
                               pixmap.rowBytes());
    }
    FML_DCHECK(false) << "get_pixels called without a codec or image generator";
    return false;
  }

  void dispose() {
    ClearDartWrapper();
    codec_ = nullptr;
    codec_.reset();
  }

  size_t GetAllocationSize() const override {
    return sizeof(ImageDescriptor) + sizeof(SkImageInfo) + sizeof(SkCodec);
  }

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  ImageDescriptor(sk_sp<SkData> buffer,
                  const SkImageInfo& image_info,
                  std::optional<size_t> row_bytes);
  ImageDescriptor(sk_sp<SkData> buffer, std::shared_ptr<SkCodec> codec);

  sk_sp<SkData> buffer_;
  std::shared_ptr<SkCodec> codec_;
  // The SkCodec doesn't know how to handle orientation. The image generator
  // does. The image generate takes sole ownership over its codec pointer,
  // but we'll still need access to the codec to ask for scaled dimensions.
  std::unique_ptr<SkImageGenerator> image_generator_;
  const SkImageInfo image_info_;
  std::optional<size_t> row_bytes_;

  const SkImageInfo CreateImageInfo() const;

  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(ImageDescriptor);
  FML_DISALLOW_COPY_AND_ASSIGN(ImageDescriptor);

  friend class ImageDecoderFixtureTest;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_IMAGE_DESCRIPTOR_H_
