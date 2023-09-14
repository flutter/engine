// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_decoder.h"
#include <functional>
#include <memory>

#include "flutter/lib/ui/painting/image_decoder_skia.h"

#if IMPELLER_SUPPORTS_RENDERING
#include "flutter/lib/ui/painting/image_decoder_impeller.h"
#endif  // IMPELLER_SUPPORTS_RENDERING

namespace flutter {

std::unique_ptr<ImageDecoder> ImageDecoder::Make(
    const Settings& settings,
    const TaskRunners& runners,
    std::shared_ptr<fml::ConcurrentTaskRunner> concurrent_task_runner,
    fml::WeakPtr<IOManager> io_manager,
    const std::shared_ptr<fml::SyncSwitch>& gpu_disabled_switch) {
#if IMPELLER_SUPPORTS_RENDERING
  if (settings.enable_impeller) {
    return std::make_unique<ImageDecoderImpeller>(
        runners,                            //
        std::move(concurrent_task_runner),  //
        std::move(io_manager),              //
        settings.enable_wide_gamut,         //
        gpu_disabled_switch);
  }
#endif  // IMPELLER_SUPPORTS_RENDERING
  return std::make_unique<ImageDecoderSkia>(
      runners,                            //
      std::move(concurrent_task_runner),  //
      std::move(io_manager)               //
  );
}

ImageDecoder::ImageDecoder(
    const TaskRunners& runners,
    std::shared_ptr<fml::ConcurrentTaskRunner> concurrent_task_runner,
    fml::WeakPtr<IOManager> io_manager)
    : runners_(runners),
      concurrent_task_runner_(std::move(concurrent_task_runner)),
      io_manager_(std::move(io_manager)),
      weak_factory_(this) {
  FML_DCHECK(runners_.IsValid());
  FML_DCHECK(runners_.GetUITaskRunner()->RunsTasksOnCurrentThread())
      << "The image decoder must be created & collected on the UI thread.";
}

void ImageDecoder::DecodeAnimatedImageFrame(
    std::weak_ptr<MultiFrameCodec::State> weak_state,
    MultiFrameImageResult result) {
  auto callback_on_io_thread =
      [result = std::move(result), io_task_runner = runners_.GetIOTaskRunner()](
          std::optional<SkBitmap> bitmap, std::string decode_error) mutable {
        io_task_runner->PostTask(
            [result = std::move(result), bitmap = std::move(bitmap),
             decode_error = std::move(decode_error)]() mutable {
              std::invoke(result, std::move(bitmap), std::move(decode_error));
            });
      };

  concurrent_task_runner_->PostTask([callback_on_io_thread =
                                         std::move(callback_on_io_thread),
                                     weak_state =
                                         std::move(weak_state)]() mutable {
    auto state = weak_state.lock();
    if (!state) {
      std::invoke(callback_on_io_thread, std::nullopt, "state is null");
      return;
    }
    SkBitmap bitmap = SkBitmap();
    SkImageInfo info =
        state->generator_->GetInfo().makeColorType(kN32_SkColorType);
    if (info.alphaType() == kUnpremul_SkAlphaType) {
      SkImageInfo updated = info.makeAlphaType(kPremul_SkAlphaType);
      info = updated;
    }

    if (!bitmap.tryAllocPixels(info)) {
      std::ostringstream stream;
      stream << "Failed to allocate memory for bitmap of size "
             << info.computeMinByteSize() << "B";
      std::string decode_error = stream.str();
      FML_LOG(ERROR) << decode_error;

      std::invoke(callback_on_io_thread, std::nullopt, decode_error);
      return;
    }

    ImageGenerator::FrameInfo frameInfo =
        state->generator_->GetFrameInfo(state->nextFrameIndex_);

    const int requiredFrameIndex =
        frameInfo.required_frame.value_or(SkCodec::kNoFrame);

    if (requiredFrameIndex != SkCodec::kNoFrame) {
      // We are here when the frame said |disposal_method| is
      // `DisposalMethod::kKeep` or `DisposalMethod::kRestorePrevious` and
      // |requiredFrameIndex| is set to ex-frame or ex-ex-frame.
      if (!state->lastRequiredFrame_.has_value()) {
        FML_DLOG(INFO) << "Frame " << state->nextFrameIndex_
                       << " depends on frame " << requiredFrameIndex
                       << " and no required frames are cached. Using blank "
                          "slate instead.";
      } else {
        // Copy the previous frame's output buffer into the current frame as
        // the starting point.
        bitmap.writePixels(state->lastRequiredFrame_->pixmap());
        if (state->restoreBGColorRect_.has_value()) {
          bitmap.erase(SK_ColorTRANSPARENT, state->restoreBGColorRect_.value());
        }
      }
    }

    // Write the new frame to the output buffer. The bitmap pixels as
    // supplied are already set in accordance with the previous frame's
    // disposal policy.
    if (!state->generator_->GetPixels(info, bitmap.getPixels(),
                                      bitmap.rowBytes(), state->nextFrameIndex_,
                                      requiredFrameIndex)) {
      std::ostringstream stream;
      stream << "Could not getPixels for frame " << state->nextFrameIndex_;
      std::string decode_error = stream.str();
      FML_LOG(ERROR) << decode_error;
      std::invoke(callback_on_io_thread, std::nullopt, decode_error);
      return;
    }

    const bool keep_current_frame =
        frameInfo.disposal_method == SkCodecAnimation::DisposalMethod::kKeep;
    const bool restore_previous_frame =
        frameInfo.disposal_method ==
        SkCodecAnimation::DisposalMethod::kRestorePrevious;
    const bool previous_frame_available = state->lastRequiredFrame_.has_value();

    // Store the current frame in `lastRequiredFrame_` if the frame's
    // disposal method indicates we should do so.
    // * When the disposal method is "Keep", the stored frame should always
    // be
    //   overwritten with the new frame we just crafted.
    // * When the disposal method is "RestorePrevious", the previously
    // stored
    //   frame should be retained and used as the backdrop for the next
    //   frame again. If there isn't already a stored frame, that means we
    //   haven't rendered any frames yet! When this happens, we just fall
    //   back to "Keep" behavior and store the current frame as the backdrop
    //   of the next frame.

    if (keep_current_frame ||
        (previous_frame_available && !restore_previous_frame)) {
      // Replace the stored frame. The `lastRequiredFrame_` will get used as
      // the starting backdrop for the next frame.
      state->lastRequiredFrame_ = bitmap;
    }

    if (frameInfo.disposal_method ==
        SkCodecAnimation::DisposalMethod::kRestoreBGColor) {
      state->restoreBGColorRect_ = frameInfo.disposal_rect;
    } else {
      state->restoreBGColorRect_.reset();
    }
    std::invoke(callback_on_io_thread, std::move(bitmap), std::string());
  });
}

ImageDecoder::~ImageDecoder() = default;

fml::WeakPtr<ImageDecoder> ImageDecoder::GetWeakPtr() const {
  return weak_factory_.GetWeakPtr();
}

}  // namespace flutter
