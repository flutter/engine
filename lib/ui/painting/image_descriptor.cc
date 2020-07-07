// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_descriptor.h"

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "flutter/lib/ui/painting/codec.h"
#include "flutter/lib/ui/painting/image_decoder.h"
#include "flutter/lib/ui/painting/multi_frame_codec.h"
#include "flutter/lib/ui/painting/single_frame_codec.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, ImageDescriptor);

#define FOR_EACH_BINDING(V)            \
  V(ImageDescriptor, initRaw)          \
  V(ImageDescriptor, instantiateCodec) \
  V(ImageDescriptor, width)            \
  V(ImageDescriptor, height)           \
  V(ImageDescriptor, bytesPerPixel)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void ImageDescriptor::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"ImageDescriptor_initEncoded", ImageDescriptor::initEncoded, 3, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

void ImageDescriptor::initEncoded(Dart_NativeArguments args) {
  Dart_Handle callback_handle = Dart_GetNativeArgument(args, 2);
  if (!Dart_IsClosure(callback_handle)) {
    Dart_SetReturnValue(args, tonic::ToDart("Callback must be a function"));
    return;
  }

  Dart_Handle descriptor_handle = Dart_GetNativeArgument(args, 0);
  ImmutableBuffer* immutable_buffer =
      tonic::DartConverter<ImmutableBuffer*>::FromDart(
          Dart_GetNativeArgument(args, 1));

  if (!immutable_buffer) {
    Dart_SetReturnValue(args,
                        tonic::ToDart("Buffer parameter must not be null"));
    return;
  }

  auto codec = SkCodec::MakeFromData(immutable_buffer->data());
  if (!codec) {
    Dart_SetReturnValue(args, tonic::ToDart("Invalid image data"));
    return;
  }
  auto image_info = codec->getInfo();
  auto descriptor = fml::MakeRefCounted<ImageDescriptor>(
      immutable_buffer->data(), std::move(codec), std::move(image_info));
  descriptor->AssociateWithDartWrapper(descriptor_handle);
  tonic::DartInvoke(callback_handle, {Dart_TypeVoid()});
}

void ImageDescriptor::initRaw(Dart_Handle descriptor_handle,
                              fml::RefPtr<ImmutableBuffer> data,
                              int width,
                              int height,
                              int row_bytes,
                              int pixel_format) {
  SkColorType color_type = kUnknown_SkColorType;
  switch (pixel_format) {
    case PixelFormat::kRGBA8888:
      color_type = kRGBA_8888_SkColorType;
      break;
    case PixelFormat::kBGRA8888:
      color_type = kBGRA_8888_SkColorType;
      break;
  }
  FML_DCHECK(color_type != kUnknown_SkColorType);
  auto image_info =
      SkImageInfo::Make(width, height, color_type, kPremul_SkAlphaType);
  auto descriptor = fml::MakeRefCounted<ImageDescriptor>(
      data->data(), std::move(image_info),
      row_bytes == -1 ? std::nullopt : std::optional<size_t>(row_bytes));
  descriptor->AssociateWithDartWrapper(descriptor_handle);
}

void ImageDescriptor::instantiateCodec(Dart_Handle callback_handle,
                                       int target_width,
                                       int target_height) {
  fml::RefPtr<Codec> ui_codec;
  if (!codec_ || codec_->getFrameCount() == 1) {
    ImageDecoder::ImageDescriptor descriptor;
    descriptor.decompressed_image_info = {
        image_info_,
        row_bytes_.value_or(static_cast<size_t>(image_info_.width() *
                                                image_info_.bytesPerPixel())),
    };

    if (target_width > 0) {
      descriptor.target_width = target_width;
    }
    if (target_height > 0) {
      descriptor.target_height = target_height;
    }
    descriptor.image_upscaling = ImageUpscalingMode::kAllowed;
    descriptor.data = buffer_;

    ui_codec = fml::MakeRefCounted<SingleFrameCodec>(std::move(descriptor));
  } else {
    ui_codec = fml::MakeRefCounted<MultiFrameCodec>(codec_);
  }

  tonic::DartInvoke(callback_handle, {ToDart(ui_codec)});
}
}  // namespace flutter
