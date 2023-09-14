// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/multi_frame_codec.h"

#include <optional>
#include <utility>

#include "flutter/fml/make_copyable.h"
#include "flutter/lib/ui/painting/display_list_image_gpu.h"
#include "flutter/lib/ui/painting/image.h"
#include "third_party/skia/include/core/SkBitmap.h"
#if IMPELLER_SUPPORTS_RENDERING
#include "flutter/lib/ui/painting/image_decoder_impeller.h"
#endif  // IMPELLER_SUPPORTS_RENDERING
#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/skia/include/core/SkPixelRef.h"
#include "third_party/skia/include/gpu/ganesh/SkImageGanesh.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {

MultiFrameCodec::MultiFrameCodec(std::shared_ptr<ImageGenerator> generator)
    : state_(new State(std::move(generator))) {}

MultiFrameCodec::~MultiFrameCodec() = default;

MultiFrameCodec::State::State(std::shared_ptr<ImageGenerator> generator)
    : generator_(std::move(generator)),
      frameCount_(generator_->GetFrameCount()),
      repetitionCount_(generator_->GetPlayCount() ==
                               ImageGenerator::kInfinitePlayCount
                           ? -1
                           : generator_->GetPlayCount() - 1),
      is_impeller_enabled_(UIDartState::Current()->IsImpellerEnabled()),
      nextFrameIndex_(0) {}

static void InvokeNextFrameCallback(
    const fml::RefPtr<CanvasImage>& image,
    int duration,
    const std::string& decode_error,
    std::unique_ptr<DartPersistentValue> callback) {
  std::shared_ptr<tonic::DartState> dart_state = callback->dart_state().lock();
  if (!dart_state) {
    FML_DLOG(ERROR) << "Could not acquire Dart state while attempting to fire "
                       "next frame callback.";
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  tonic::DartInvoke(callback->value(),
                    {tonic::ToDart(image), tonic::ToDart(duration),
                     tonic::ToDart(decode_error)});
}

std::pair<sk_sp<DlImage>, std::string>
MultiFrameCodec::State::GetNextFrameImage(
    fml::WeakPtr<GrDirectContext> resourceContext,
    fml::RefPtr<flutter::SkiaUnrefQueue> unref_queue,
    const std::shared_ptr<const fml::SyncSwitch>& gpu_disable_sync_switch,
    const std::shared_ptr<impeller::Context>& impeller_context,
    SkBitmap bitmap) const {
#if IMPELLER_SUPPORTS_RENDERING
  if (is_impeller_enabled_) {
    // This is safe regardless of whether the GPU is available or not because
    // without mipmap creation there is no command buffer encoding done.
    return ImageDecoderImpeller::UploadTextureToStorage(
        impeller_context, std::make_shared<SkBitmap>(bitmap),
        std::make_shared<fml::SyncSwitch>(),
        impeller::StorageMode::kHostVisible,
        /*create_mips=*/false);
  }
#endif  // IMPELLER_SUPPORTS_RENDERING

  sk_sp<SkImage> skImage;
  gpu_disable_sync_switch->Execute(
      fml::SyncSwitch::Handlers()
          .SetIfTrue([&skImage, &bitmap] {
            // Defer decoding until time of draw later on the raster thread.
            // Can happen when GL operations are currently forbidden such as
            // in the background on iOS.
            skImage = SkImages::RasterFromBitmap(bitmap);
          })
          .SetIfFalse([&skImage, &resourceContext, &bitmap] {
            if (resourceContext) {
              SkPixmap pixmap(bitmap.info(), bitmap.pixelRef()->pixels(),
                              bitmap.pixelRef()->rowBytes());
              skImage = SkImages::CrossContextTextureFromPixmap(
                  resourceContext.get(), pixmap, true);
            } else {
              // Defer decoding until time of draw later on the raster thread.
              // Can happen when GL operations are currently forbidden such as
              // in the background on iOS.
              skImage = SkImages::RasterFromBitmap(bitmap);
            }
          }));

  return std::make_pair(DlImageGPU::Make({skImage, std::move(unref_queue)}),
                        std::string());
}

void MultiFrameCodec::State::OnGetImageAndInvokeCallback(
    const fml::RefPtr<fml::TaskRunner>& ui_task_runner,
    sk_sp<DlImage> dl_image,
    std::string decode_error,
    std::unique_ptr<DartPersistentValue> callback) {
  fml::RefPtr<CanvasImage> image = nullptr;
  int duration = 0;
  if (dl_image) {
    image = CanvasImage::Create();
    image->set_image(std::move(dl_image));
    ImageGenerator::FrameInfo frameInfo =
        generator_->GetFrameInfo(nextFrameIndex_);
    duration = frameInfo.duration;
  }
  nextFrameIndex_ = (nextFrameIndex_ + 1) % frameCount_;

  // The static leak checker gets confused by the use of fml::MakeCopyable.
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  ui_task_runner->PostTask(fml::MakeCopyable(
      [callback = std::move(callback), image = std::move(image),
       decode_error = std::move(decode_error), duration]() mutable {
        InvokeNextFrameCallback(image, duration, decode_error,
                                std::move(callback));
      }));
}

Dart_Handle MultiFrameCodec::getNextFrame(Dart_Handle callback_handle) {
  if (!Dart_IsClosure(callback_handle)) {
    return tonic::ToDart("Callback must be a function");
  }

  auto* dart_state = UIDartState::Current();

  const auto& task_runners = dart_state->GetTaskRunners();
  auto image_decoder = dart_state->GetImageDecoder();

  if (!image_decoder) {
    return tonic::ToDart(
        "Failed to access the internal image decoder "
        "registry on this isolate. Please file a bug on "
        "https://github.com/flutter/flutter/issues.");
  }
  if (state_->frameCount_ == 0) {
    std::string decode_error("Could not provide any frame.");
    FML_LOG(ERROR) << decode_error;
    task_runners.GetUITaskRunner()->PostTask(fml::MakeCopyable(
        [decode_error = std::move(decode_error),
         callback = std::make_unique<DartPersistentValue>(
             tonic::DartState::Current(), callback_handle)]() mutable {
          InvokeNextFrameCallback(nullptr, 0, decode_error,
                                  std::move(callback));
        }));
    return Dart_Null();
  }

  image_decoder->DecodeAnimatedImageFrame(
      state_,
      fml::MakeCopyable([callback = std::make_unique<DartPersistentValue>(
                             tonic::DartState::Current(), callback_handle),
                         weak_state =
                             std::weak_ptr<MultiFrameCodec::State>(state_),
                         ui_task_runner = task_runners.GetUITaskRunner(),
                         io_manager = dart_state->GetIOManager()](
                            std::optional<SkBitmap> bitmap,
                            std::string decode_error) mutable {
        auto state = weak_state.lock();
        if (!state || !io_manager) {
          ui_task_runner->PostTask(fml::MakeCopyable(
              [callback = std::move(callback)]() { callback->Clear(); }));
          return;
        }
        sk_sp<DlImage> dl_image;
        if (bitmap) {
          std::tie(dl_image, decode_error) = state->GetNextFrameImage(
              io_manager->GetResourceContext(), io_manager->GetSkiaUnrefQueue(),
              io_manager->GetIsGpuDisabledSyncSwitch(),
              io_manager->GetImpellerContext(), std::move(*bitmap));
        }

        state->OnGetImageAndInvokeCallback(ui_task_runner, std::move(dl_image),
                                           std::move(decode_error),
                                           std::move(callback));
      }));

  return Dart_Null();
  // The static leak checker gets confused by the control flow, unique
  // pointers and closures in this function.
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
}

int MultiFrameCodec::frameCount() const {
  return state_->frameCount_;
}

int MultiFrameCodec::repetitionCount() const {
  return state_->repetitionCount_;
}

}  // namespace flutter
