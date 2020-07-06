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
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

class ImageDescriptor : public RefCountedDartWrappable<ImageDescriptor> {
 public:
  ~ImageDescriptor() override = default;

  static void initEncoded(Dart_NativeArguments args);

  static void initRaw(Dart_Handle descriptor_handle,
                      fml::RefPtr<ImmutableBuffer> data,
                      int width,
                      int height,
                      int row_bytes,
                      int pixel_format);

  void instantiateCodec(Dart_Handle callback,
                        int target_width,
                        int target_height);

  int width() const { return image_info_.width(); }

  int height() const { return image_info_.height(); }

  int bytesPerPixel() const { return image_info_.bytesPerPixel(); }

  void dispose() {
    ClearDartWrapper();
    codec_.reset();
  }

  size_t GetAllocationSize() const override {
    return sizeof(ImageDescriptor) + sizeof(SkImageInfo) + sizeof(SkCodec);
  }

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  // This must be kept in sync with the enum in painting.dart
  enum PixelFormat {
    kRGBA8888,
    kBGRA8888,
  };

  ImageDescriptor(sk_sp<SkData> buffer,
                  const SkImageInfo& image_info,
                  std::optional<size_t> row_bytes)
      : buffer_(std::move(buffer)),
        codec_(nullptr),
        image_info_(std::move(image_info)),
        row_bytes_(row_bytes) {}

  ImageDescriptor(sk_sp<SkData> buffer,
                  std::shared_ptr<SkCodec> codec,
                  const SkImageInfo& image_info)
      : buffer_(std::move(buffer)),
        codec_(std::move(codec)),
        image_info_(std::move(image_info)),
        row_bytes_(std::nullopt) {}

  sk_sp<SkData> buffer_;
  std::shared_ptr<SkCodec> codec_;
  const SkImageInfo image_info_;
  std::optional<size_t> row_bytes_;

  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(ImageDescriptor);
  FML_DISALLOW_COPY_AND_ASSIGN(ImageDescriptor);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_IMAGE_DESCRIPTOR_H_
