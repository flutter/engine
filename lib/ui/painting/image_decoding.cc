// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_decoding.h"

#include "flutter/common/threads.h"
#include "flutter/glue/trace_event.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/painting/image.h"
#include "flutter/lib/ui/painting/resource_context.h"
#include "lib/ftl/build_config.h"
#include "lib/ftl/functional/make_copyable.h"
#include "lib/tonic/dart_persistent_value.h"
#include "lib/tonic/dart_state.h"
#include "lib/tonic/logging/dart_invoke.h"
#include "lib/tonic/typed_data/uint8_list.h"
#include "third_party/skia/include/core/SkCrossContextImageData.h"
#include "third_party/skia/include/core/SkImageGenerator.h"
#include "third_party/skia/include/gpu/GrContext.h"

using tonic::DartInvoke;
using tonic::DartPersistentValue;
using tonic::ToDart;

namespace blink {
namespace {

void InvokeImageCallback(sk_sp<SkImage> image,
                         std::unique_ptr<DartPersistentValue> callback) {
  tonic::DartState* dart_state = callback->dart_state().get();
  if (!dart_state)
    return;
  tonic::DartState::Scope scope(dart_state);
  if (!image) {
    DartInvoke(callback->value(), {Dart_Null()});
  } else {
    ftl::RefPtr<CanvasImage> resultImage = CanvasImage::Create();
    resultImage->set_image(std::move(image));
    DartInvoke(callback->value(), {ToDart(resultImage)});
  }
}

void InvokeImageCallbackOnUIThread(
    std::unique_ptr<DartPersistentValue> callback,
    sk_sp<SkImage> image) {
  Threads::UI()->PostTask(
      ftl::MakeCopyable([ callback = std::move(callback), image ]() mutable {
        InvokeImageCallback(image, std::move(callback));
      }));
}

void DecodeImageAndInvokeImageCallback(
    std::unique_ptr<DartPersistentValue> callback,
    sk_sp<SkData> buffer,
    sk_sp<GrContext> rasterizer_grcontext) {
  if (buffer == nullptr || buffer->isEmpty()) {
    InvokeImageCallbackOnUIThread(std::move(callback), nullptr);
    return;
  }

  std::unique_ptr<SkCrossContextImageData> cross_context;
  if (ResourceContext::Get() && rasterizer_grcontext) {
    cross_context = SkCrossContextImageData::MakeFromEncoded(
        ResourceContext::Get(), buffer, nullptr);
  }

  if (!cross_context) {
    // Return a raster image if we are unable to create a texture backed image.
    sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(buffer));
    InvokeImageCallbackOnUIThread(std::move(callback), std::move(image));
    return;
  }

  Threads::Gpu()->PostTask(ftl::MakeCopyable([
    callback = std::move(callback),
    buffer = std::move(buffer),
    cross_context = std::move(cross_context),
    rasterizer_grcontext = std::move(rasterizer_grcontext)
  ]() mutable {
    sk_sp<SkImage> image = SkImage::MakeFromCrossContextImageData(
        rasterizer_grcontext.get(), std::move(cross_context));
    InvokeImageCallbackOnUIThread(std::move(callback), std::move(image));
  }));
}

void DecodeImageFromList(Dart_NativeArguments args) {
  Dart_Handle exception = nullptr;

  tonic::Uint8List list =
      tonic::DartConverter<tonic::Uint8List>::FromArguments(args, 0, exception);
  if (exception) {
    Dart_ThrowException(exception);
    return;
  }

  Dart_Handle callback_handle = Dart_GetNativeArgument(args, 1);
  if (!Dart_IsClosure(callback_handle)) {
    Dart_ThrowException(ToDart("Callback must be a function"));
    return;
  }

  auto buffer = SkData::MakeWithCopy(list.data(), list.num_elements());

  sk_sp<GrContext> rasterizer_grcontext =
      UIDartState::Current()->rasterizer_grcontext();

  Threads::IO()->PostTask(ftl::MakeCopyable([
    callback = std::make_unique<DartPersistentValue>(
        tonic::DartState::Current(), callback_handle),
    buffer = std::move(buffer),
    rasterizer_grcontext = std::move(rasterizer_grcontext)
  ]() mutable {
    DecodeImageAndInvokeImageCallback(std::move(callback),
                                      std::move(buffer),
                                      std::move(rasterizer_grcontext));
  }));
}

}  // namespace

void ImageDecoding::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({
      {"decodeImageFromList", DecodeImageFromList, 2, true},
  });
}

}  // namespace blink
