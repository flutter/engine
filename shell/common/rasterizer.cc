// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/rasterizer.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "flow/frame_timings.h"
#include "flutter/common/graphics/persistent_cache.h"
#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/time/time_point.h"
#include "flutter/shell/common/serialization_callbacks.h"
#include "fml/make_copyable.h"
#include "third_party/skia/include/core/SkEncodedImageFormat.h"
#include "third_party/skia/include/core/SkImageEncoder.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkSerialProcs.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkSurfaceCharacterization.h"
#include "third_party/skia/include/utils/SkBase64.h"

namespace flutter {

// The rasterizer will tell Skia to purge cached resources that have not been
// used within this interval.
static constexpr std::chrono::milliseconds kSkiaCleanupExpiration(15000);

Rasterizer::Rasterizer(Delegate& delegate)
    : delegate_(delegate),
      compositor_context_(std::make_unique<flutter::CompositorContext>(
          delegate.GetFrameBudget())),
      user_override_resource_cache_bytes_(false),
      weak_factory_(this) {
  FML_DCHECK(compositor_context_);
}

Rasterizer::~Rasterizer() = default;

fml::TaskRunnerAffineWeakPtr<Rasterizer> Rasterizer::GetWeakPtr() const {
  return weak_factory_.GetWeakPtr();
}

fml::TaskRunnerAffineWeakPtr<SnapshotDelegate> Rasterizer::GetSnapshotDelegate()
    const {
  return weak_factory_.GetWeakPtr();
}

void Rasterizer::Setup(std::unique_ptr<Surface> surface) {
  surface_ = std::move(surface);

  if (max_cache_bytes_.has_value()) {
    SetResourceCacheMaxBytes(max_cache_bytes_.value(),
                             user_override_resource_cache_bytes_);
  }

  auto context_switch = surface_->MakeRenderContextCurrent();
  if (context_switch->GetResult()) {
    compositor_context_->OnGrContextCreated();
  }

  if (external_view_embedder_ &&
      external_view_embedder_->SupportsDynamicThreadMerging() &&
      !raster_thread_merger_) {
    const auto platform_id =
        delegate_.GetTaskRunners().GetPlatformTaskRunner()->GetTaskQueueId();
    const auto gpu_id =
        delegate_.GetTaskRunners().GetRasterTaskRunner()->GetTaskQueueId();
    raster_thread_merger_ = fml::RasterThreadMerger::CreateOrShareThreadMerger(
        delegate_.GetParentRasterThreadMerger(), platform_id, gpu_id);
  }
  if (raster_thread_merger_) {
    raster_thread_merger_->SetMergeUnmergeCallback([=]() {
      // Clear the GL context after the thread configuration has changed.
      if (surface_) {
        surface_->ClearRenderContext();
      }
    });
  }
}

void Rasterizer::Teardown() {
  auto context_switch =
      surface_ ? surface_->MakeRenderContextCurrent() : nullptr;
  if (context_switch && context_switch->GetResult()) {
    compositor_context_->OnGrContextDestroyed();
  }

  surface_.reset();
  last_layer_tree_.reset();

  if (raster_thread_merger_.get() != nullptr &&
      raster_thread_merger_.get()->IsMerged()) {
    FML_DCHECK(raster_thread_merger_->IsEnabled());
    raster_thread_merger_->UnMergeNowIfLastOne();
    raster_thread_merger_->SetMergeUnmergeCallback(nullptr);
  }
}

void Rasterizer::EnableThreadMergerIfNeeded() {
  if (raster_thread_merger_) {
    raster_thread_merger_->Enable();
  }
}

void Rasterizer::DisableThreadMergerIfNeeded() {
  if (raster_thread_merger_) {
    raster_thread_merger_->Disable();
  }
}

void Rasterizer::NotifyLowMemoryWarning() const {
  if (!surface_) {
    FML_DLOG(INFO)
        << "Rasterizer::NotifyLowMemoryWarning called with no surface.";
    return;
  }
  auto context = surface_->GetContext();
  if (!context) {
    FML_DLOG(INFO)
        << "Rasterizer::NotifyLowMemoryWarning called with no GrContext.";
    return;
  }
  auto context_switch = surface_->MakeRenderContextCurrent();
  if (!context_switch->GetResult()) {
    return;
  }
  context->performDeferredCleanup(std::chrono::milliseconds(0));
}

flutter::TextureRegistry* Rasterizer::GetTextureRegistry() {
  return &compositor_context_->texture_registry();
}

flutter::LayerTree* Rasterizer::GetLastLayerTree() {
  return last_layer_tree_.get();
}

void Rasterizer::DrawLastLayerTree(
    std::unique_ptr<FrameTimingsRecorder> frame_timings_recorder) {
  if (!last_layer_tree_ || !surface_) {
    return;
  }
  frame_timings_recorder->RecordRasterStart(
      fml::TimePoint::Now(), &compositor_context_->raster_cache());
  DrawToSurface(*frame_timings_recorder, *last_layer_tree_);
}

RasterStatus Rasterizer::Draw(
    std::unique_ptr<FrameTimingsRecorder> frame_timings_recorder,
    std::shared_ptr<Pipeline<flutter::LayerTree>> pipeline,
    LayerTreeDiscardCallback discard_callback) {
  TRACE_EVENT_WITH_FRAME_NUMBER(frame_timings_recorder, "flutter",
                                "GPURasterizer::Draw");
  if (raster_thread_merger_ &&
      !raster_thread_merger_->IsOnRasterizingThread()) {
    // we yield and let this frame be serviced on the right thread.
    return RasterStatus::kYielded;
  }
  FML_DCHECK(delegate_.GetTaskRunners()
                 .GetRasterTaskRunner()
                 ->RunsTasksOnCurrentThread());

  std::unique_ptr<FrameTimingsRecorder> resubmit_recorder =
      frame_timings_recorder->CloneUntil(
          FrameTimingsRecorder::State::kBuildEnd);

  RasterStatus raster_status = RasterStatus::kFailed;
  Pipeline<flutter::LayerTree>::Consumer consumer =
      [&](std::unique_ptr<LayerTree> layer_tree) {
        if (discard_callback(*layer_tree.get())) {
          raster_status = RasterStatus::kDiscarded;
        } else {
          raster_status =
              DoDraw(std::move(frame_timings_recorder), std::move(layer_tree));
        }
      };

  PipelineConsumeResult consume_result = pipeline->Consume(consumer);
  // if the raster status is to resubmit the frame, we push the frame to the
  // front of the queue and also change the consume status to more available.

  auto should_resubmit_frame = raster_status == RasterStatus::kResubmit ||
                               raster_status == RasterStatus::kSkipAndRetry;
  if (should_resubmit_frame) {
    auto front_continuation = pipeline->ProduceIfEmpty();
    bool result =
        front_continuation.Complete(std::move(resubmitted_layer_tree_));
    if (result) {
      consume_result = PipelineConsumeResult::MoreAvailable;
    }
  } else if (raster_status == RasterStatus::kEnqueuePipeline) {
    consume_result = PipelineConsumeResult::MoreAvailable;
  }

  // EndFrame should perform cleanups for the external_view_embedder.
  if (surface_ && external_view_embedder_) {
    external_view_embedder_->EndFrame(should_resubmit_frame,
                                      raster_thread_merger_);
  }

  // Consume as many pipeline items as possible. But yield the event loop
  // between successive tries.
  switch (consume_result) {
    case PipelineConsumeResult::MoreAvailable: {
      delegate_.GetTaskRunners().GetRasterTaskRunner()->PostTask(
          fml::MakeCopyable(
              [weak_this = weak_factory_.GetWeakPtr(), pipeline,
               resubmit_recorder = std::move(resubmit_recorder),
               discard_callback = std::move(discard_callback)]() mutable {
                if (weak_this) {
                  weak_this->Draw(std::move(resubmit_recorder), pipeline,
                                  std::move(discard_callback));
                }
              }));
      break;
    }
    default:
      break;
  }

  return raster_status;
}

namespace {
sk_sp<SkImage> DrawSnapshot(
    sk_sp<SkSurface> surface,
    const std::function<void(SkCanvas*)>& draw_callback) {
  if (surface == nullptr || surface->getCanvas() == nullptr) {
    return nullptr;
  }

  draw_callback(surface->getCanvas());
  surface->getCanvas()->flush();

  sk_sp<SkImage> device_snapshot;
  {
    TRACE_EVENT0("flutter", "MakeDeviceSnpashot");
    device_snapshot = surface->makeImageSnapshot();
  }

  if (device_snapshot == nullptr) {
    return nullptr;
  }

  {
    TRACE_EVENT0("flutter", "DeviceHostTransfer");
    if (auto raster_image = device_snapshot->makeRasterImage()) {
      return raster_image;
    }
  }

  return nullptr;
}
}  // namespace

sk_sp<SkImage> Rasterizer::DoMakeRasterSnapshot(
    SkISize size,
    std::function<void(SkCanvas*)> draw_callback) {
  TRACE_EVENT0("flutter", __FUNCTION__);
  sk_sp<SkImage> result;
  SkImageInfo image_info = SkImageInfo::MakeN32Premul(
      size.width(), size.height(), SkColorSpace::MakeSRGB());

  std::unique_ptr<Surface> pbuffer_surface;
  Surface* snapshot_surface = nullptr;
  if (surface_ && surface_->GetContext()) {
    snapshot_surface = surface_.get();
  } else if (snapshot_surface_producer_) {
    pbuffer_surface = snapshot_surface_producer_->CreateSnapshotSurface();
    if (pbuffer_surface && pbuffer_surface->GetContext())
      snapshot_surface = pbuffer_surface.get();
  }

  if (!snapshot_surface) {
    // Raster surface is fine if there is no on screen surface. This might
    // happen in case of software rendering.
    sk_sp<SkSurface> sk_surface = SkSurface::MakeRaster(image_info);
    result = DrawSnapshot(sk_surface, draw_callback);
  } else {
    delegate_.GetIsGpuDisabledSyncSwitch()->Execute(
        fml::SyncSwitch::Handlers()
            .SetIfTrue([&] {
              sk_sp<SkSurface> surface = SkSurface::MakeRaster(image_info);
              result = DrawSnapshot(surface, draw_callback);
            })
            .SetIfFalse([&] {
              FML_DCHECK(snapshot_surface);
              auto context_switch =
                  snapshot_surface->MakeRenderContextCurrent();
              if (!context_switch->GetResult()) {
                return;
              }

              GrRecordingContext* context = snapshot_surface->GetContext();
              auto max_size = context->maxRenderTargetSize();
              double scale_factor = std::min(
                  1.0, static_cast<double>(max_size) /
                           static_cast<double>(std::max(image_info.width(),
                                                        image_info.height())));

              // Scale down the render target size to the max supported by the
              // GPU if necessary. Exceeding the max would otherwise cause a
              // null result.
              if (scale_factor < 1.0) {
                image_info = image_info.makeWH(
                    static_cast<double>(image_info.width()) * scale_factor,
                    static_cast<double>(image_info.height()) * scale_factor);
              }

              // When there is an on screen surface, we need a render target
              // SkSurface because we want to access texture backed images.
              sk_sp<SkSurface> sk_surface =
                  SkSurface::MakeRenderTarget(context,          // context
                                              SkBudgeted::kNo,  // budgeted
                                              image_info        // image info
                  );
              if (!sk_surface) {
                FML_LOG(ERROR)
                    << "DoMakeRasterSnapshot can not create GPU render target";
                return;
              }

              sk_surface->getCanvas()->scale(scale_factor, scale_factor);
              result = DrawSnapshot(sk_surface, draw_callback);
            }));
  }

  return result;
}

sk_sp<SkImage> Rasterizer::MakeRasterSnapshot(
    std::function<void(SkCanvas*)> draw_callback,
    SkISize picture_size) {
  return DoMakeRasterSnapshot(picture_size, draw_callback);
}

sk_sp<SkImage> Rasterizer::MakeRasterSnapshot(sk_sp<SkPicture> picture,
                                              SkISize picture_size) {
  return DoMakeRasterSnapshot(picture_size,
                              [picture = std::move(picture)](SkCanvas* canvas) {
                                canvas->drawPicture(picture);
                              });
}

sk_sp<SkImage> Rasterizer::ConvertToRasterImage(sk_sp<SkImage> image) {
  TRACE_EVENT0("flutter", __FUNCTION__);

  // If the rasterizer does not have a surface with a GrContext, then it will
  // be unable to render a cross-context SkImage.  The caller will need to
  // create the raster image on the IO thread.
  if (surface_ == nullptr || surface_->GetContext() == nullptr) {
    return nullptr;
  }

  if (image == nullptr) {
    return nullptr;
  }

  SkISize image_size = image->dimensions();
  return DoMakeRasterSnapshot(image_size,
                              [image = std::move(image)](SkCanvas* canvas) {
                                canvas->drawImage(image, 0, 0);
                              });
}

RasterStatus Rasterizer::DoDraw(
    std::unique_ptr<FrameTimingsRecorder> frame_timings_recorder,
    std::unique_ptr<flutter::LayerTree> layer_tree) {
  FML_DCHECK(delegate_.GetTaskRunners()
                 .GetRasterTaskRunner()
                 ->RunsTasksOnCurrentThread());

  if (!layer_tree || !surface_) {
    return RasterStatus::kFailed;
  }

  frame_timings_recorder->RecordRasterStart(
      fml::TimePoint::Now(), &compositor_context_->raster_cache());

  PersistentCache* persistent_cache = PersistentCache::GetCacheForProcess();
  persistent_cache->ResetStoredNewShaders();

  RasterStatus raster_status =
      DrawToSurface(*frame_timings_recorder, *layer_tree);
  if (raster_status == RasterStatus::kSuccess) {
    last_layer_tree_ = std::move(layer_tree);
  } else if (raster_status == RasterStatus::kResubmit ||
             raster_status == RasterStatus::kSkipAndRetry) {
    resubmitted_layer_tree_ = std::move(layer_tree);
    return raster_status;
  } else if (raster_status == RasterStatus::kDiscarded) {
    return raster_status;
  }

  if (persistent_cache->IsDumpingSkp() &&
      persistent_cache->StoredNewShaders()) {
    auto screenshot =
        ScreenshotLastLayerTree(ScreenshotType::SkiaPicture, false);
    persistent_cache->DumpSkp(*screenshot.data);
  }

  // TODO(liyuqian): in Fuchsia, the rasterization doesn't finish when
  // Rasterizer::DoDraw finishes. Future work is needed to adapt the timestamp
  // for Fuchsia to capture SceneUpdateContext::ExecutePaintTasks.
  delegate_.OnFrameRasterized(frame_timings_recorder->GetRecordedTime());

// SceneDisplayLag events are disabled on Fuchsia.
// see: https://github.com/flutter/flutter/issues/56598
#if !defined(OS_FUCHSIA)
  const fml::TimePoint raster_finish_time =
      frame_timings_recorder->GetRasterEndTime();
  fml::TimePoint frame_target_time =
      frame_timings_recorder->GetVsyncTargetTime();
  if (raster_finish_time > frame_target_time) {
    fml::TimePoint latest_frame_target_time =
        delegate_.GetLatestFrameTargetTime();
    const auto frame_budget_millis = delegate_.GetFrameBudget().count();
    if (latest_frame_target_time < raster_finish_time) {
      latest_frame_target_time =
          latest_frame_target_time +
          fml::TimeDelta::FromMillisecondsF(frame_budget_millis);
    }
    const auto frame_lag =
        (latest_frame_target_time - frame_target_time).ToMillisecondsF();
    const int vsync_transitions_missed = round(frame_lag / frame_budget_millis);
    fml::tracing::TraceEventAsyncComplete(
        "flutter",                    // category
        "SceneDisplayLag",            // name
        raster_finish_time,           // begin_time
        latest_frame_target_time,     // end_time
        "frame_target_time",          // arg_key_1
        frame_target_time,            // arg_val_1
        "current_frame_target_time",  // arg_key_2
        latest_frame_target_time,     // arg_val_2
        "vsync_transitions_missed",   // arg_key_3
        vsync_transitions_missed      // arg_val_3
    );
  }
#endif

  // Pipeline pressure is applied from a couple of places:
  // rasterizer: When there are more items as of the time of Consume.
  // animator (via shell): Frame gets produces every vsync.
  // Enqueing here is to account for the following scenario:
  // T = 1
  //  - one item (A) in the pipeline
  //  - rasterizer starts (and merges the threads)
  //  - pipeline consume result says no items to process
  // T = 2
  //  - animator produces (B) to the pipeline
  //  - applies pipeline pressure via platform thread.
  // T = 3
  //   - rasterizes finished (and un-merges the threads)
  //   - |Draw| for B yields as its on the wrong thread.
  // This enqueue ensures that we attempt to consume from the right
  // thread one more time after un-merge.
  if (raster_thread_merger_) {
    if (raster_thread_merger_->DecrementLease() ==
        fml::RasterThreadStatus::kUnmergedNow) {
      return RasterStatus::kEnqueuePipeline;
    }
  }

  return raster_status;
}

RasterStatus Rasterizer::DrawToSurface(
    FrameTimingsRecorder& frame_timings_recorder,
    flutter::LayerTree& layer_tree) {
  TRACE_EVENT0("flutter", "Rasterizer::DrawToSurface");
  FML_DCHECK(surface_);

  RasterStatus raster_status;
  if (surface_->AllowsDrawingWhenGpuDisabled()) {
    raster_status = DrawToSurfaceUnsafe(frame_timings_recorder, layer_tree);
  } else {
    delegate_.GetIsGpuDisabledSyncSwitch()->Execute(
        fml::SyncSwitch::Handlers()
            .SetIfTrue([&] { raster_status = RasterStatus::kDiscarded; })
            .SetIfFalse([&] {
              raster_status =
                  DrawToSurfaceUnsafe(frame_timings_recorder, layer_tree);
            }));
  }

  return raster_status;
}

/// Unsafe because it assumes we have access to the GPU which isn't the case
/// when iOS is backgrounded, for example.
/// \see Rasterizer::DrawToSurface
RasterStatus Rasterizer::DrawToSurfaceUnsafe(
    FrameTimingsRecorder& frame_timings_recorder,
    flutter::LayerTree& layer_tree) {
  TRACE_EVENT0("flutter", "Rasterizer::DrawToSurfaceUnsafe");
  FML_DCHECK(surface_);

  compositor_context_->ui_time().SetLapTime(
      frame_timings_recorder.GetBuildDuration());

  SkCanvas* embedder_root_canvas = nullptr;
  if (external_view_embedder_) {
    external_view_embedder_->BeginFrame(
        layer_tree.frame_size(), surface_->GetContext(),
        layer_tree.device_pixel_ratio(), raster_thread_merger_);
    embedder_root_canvas = external_view_embedder_->GetRootCanvas();
  }

  // On Android, the external view embedder deletes surfaces in `BeginFrame`.
  //
  // Deleting a surface also clears the GL context. Therefore, acquire the
  // frame after calling `BeginFrame` as this operation resets the GL context.
  auto frame = surface_->AcquireFrame(layer_tree.frame_size());
  if (frame == nullptr) {
    return RasterStatus::kFailed;
  }

  // If the external view embedder has specified an optional root surface, the
  // root surface transformation is set by the embedder instead of
  // having to apply it here.
  SkMatrix root_surface_transformation =
      embedder_root_canvas ? SkMatrix{} : surface_->GetRootTransformation();

  auto root_surface_canvas =
      embedder_root_canvas ? embedder_root_canvas : frame->SkiaCanvas();

  auto compositor_frame = compositor_context_->AcquireFrame(
      surface_->GetContext(),         // skia GrContext
      root_surface_canvas,            // root surface canvas
      external_view_embedder_.get(),  // external view embedder
      root_surface_transformation,    // root surface transformation
      true,                           // instrumentation enabled
      frame->framebuffer_info()
          .supports_readback,  // surface supports pixel reads
      raster_thread_merger_    // thread merger
  );
  if (compositor_frame) {
    FrameDamage damage;
    if (!frame->framebuffer_info().existing_damage.empty()) {
      damage.prev_layer_tree = last_layer_tree_.get();
      for (const auto& r : frame->framebuffer_info().existing_damage) {
        damage.additional_damage.join(r);
      }
    }

    RasterStatus raster_status =
        compositor_frame->Raster(layer_tree, false, &damage);
    if (raster_status == RasterStatus::kFailed ||
        raster_status == RasterStatus::kSkipAndRetry) {
      return raster_status;
    }

    SurfaceFrame::SubmitInfo submit_info;
    submit_info.frame_damage.push_back(damage.damage_out.frame_damage);
    submit_info.buffer_damage.push_back(damage.damage_out.buffer_damage);

    frame->set_submit_info(submit_info);

    if (external_view_embedder_ &&
        (!raster_thread_merger_ || raster_thread_merger_->IsMerged())) {
      FML_DCHECK(!frame->IsSubmitted());
      external_view_embedder_->SubmitFrame(surface_->GetContext(),
                                           std::move(frame));
    } else {
      frame->Submit();
    }

    frame_timings_recorder.RecordRasterEnd(
        &compositor_context_->raster_cache());
    FireNextFrameCallbackIfPresent();

    if (surface_->GetContext()) {
      TRACE_EVENT0("flutter", "PerformDeferredSkiaCleanup");
      surface_->GetContext()->performDeferredCleanup(kSkiaCleanupExpiration);
    }

    return raster_status;
  }

  return RasterStatus::kFailed;
}

static sk_sp<SkData> ScreenshotLayerTreeAsPicture(
    flutter::LayerTree* tree,
    flutter::CompositorContext& compositor_context) {
  FML_DCHECK(tree != nullptr);
  SkPictureRecorder recorder;
  recorder.beginRecording(
      SkRect::MakeWH(tree->frame_size().width(), tree->frame_size().height()));

  SkMatrix root_surface_transformation;
  root_surface_transformation.reset();

  // TODO(amirh): figure out how to take a screenshot with embedded UIView.
  // https://github.com/flutter/flutter/issues/23435
  auto frame = compositor_context.AcquireFrame(
      nullptr, recorder.getRecordingCanvas(), nullptr,
      root_surface_transformation, false, true, nullptr);
  frame->Raster(*tree, true, nullptr);

#if defined(OS_FUCHSIA)
  SkSerialProcs procs = {0};
  procs.fImageProc = SerializeImageWithoutData;
  procs.fTypefaceProc = SerializeTypefaceWithoutData;
#else
  SkSerialProcs procs = {0};
  procs.fTypefaceProc = SerializeTypefaceWithData;
#endif

  return recorder.finishRecordingAsPicture()->serialize(&procs);
}

static sk_sp<SkSurface> CreateSnapshotSurface(GrDirectContext* surface_context,
                                              const SkISize& size) {
  const auto image_info = SkImageInfo::MakeN32Premul(
      size.width(), size.height(), SkColorSpace::MakeSRGB());
  if (surface_context) {
    // There is a rendering surface that may contain textures that are going to
    // be referenced in the layer tree about to be drawn.
    return SkSurface::MakeRenderTarget(surface_context,  //
                                       SkBudgeted::kNo,  //
                                       image_info        //
    );
  }

  // There is no rendering surface, assume no GPU textures are present and
  // create a raster surface.
  return SkSurface::MakeRaster(image_info);
}

sk_sp<SkData> Rasterizer::ScreenshotLayerTreeAsImage(
    flutter::LayerTree* tree,
    flutter::CompositorContext& compositor_context,
    GrDirectContext* surface_context,
    bool compressed) {
  // Attempt to create a snapshot surface depending on whether we have access to
  // a valid GPU rendering context.
  auto snapshot_surface =
      CreateSnapshotSurface(surface_context, tree->frame_size());
  if (snapshot_surface == nullptr) {
    FML_LOG(ERROR) << "Screenshot: unable to create snapshot surface";
    return nullptr;
  }

  // Draw the current layer tree into the snapshot surface.
  auto* canvas = snapshot_surface->getCanvas();

  // There is no root surface transformation for the screenshot layer. Reset the
  // matrix to identity.
  SkMatrix root_surface_transformation;
  root_surface_transformation.reset();

  // snapshot_surface->makeImageSnapshot needs the GL context to be set if the
  // render context is GL. frame->Raster() pops the gl context in platforms that
  // gl context switching are used. (For example, older iOS that uses GL) We
  // reset the GL context using the context switch.
  auto context_switch = surface_->MakeRenderContextCurrent();
  if (!context_switch->GetResult()) {
    FML_LOG(ERROR) << "Screenshot: unable to make image screenshot";
    return nullptr;
  }

  auto frame = compositor_context.AcquireFrame(surface_context, canvas, nullptr,
                                               root_surface_transformation,
                                               false, true, nullptr);
  canvas->clear(SK_ColorTRANSPARENT);
  frame->Raster(*tree, true, nullptr);
  canvas->flush();

  // Prepare an image from the surface, this image may potentially be on th GPU.
  auto potentially_gpu_snapshot = snapshot_surface->makeImageSnapshot();
  if (!potentially_gpu_snapshot) {
    FML_LOG(ERROR) << "Screenshot: unable to make image screenshot";
    return nullptr;
  }

  // Copy the GPU image snapshot into CPU memory.
  auto cpu_snapshot = potentially_gpu_snapshot->makeRasterImage();
  if (!cpu_snapshot) {
    FML_LOG(ERROR) << "Screenshot: unable to make raster image";
    return nullptr;
  }

  // If the caller want the pixels to be compressed, there is a Skia utility to
  // compress to PNG. Use that.
  if (compressed) {
    return cpu_snapshot->encodeToData();
  }

  // Copy it into a bitmap and return the same.
  SkPixmap pixmap;
  if (!cpu_snapshot->peekPixels(&pixmap)) {
    FML_LOG(ERROR) << "Screenshot: unable to obtain bitmap pixels";
    return nullptr;
  }
  return SkData::MakeWithCopy(pixmap.addr32(), pixmap.computeByteSize());
}

Rasterizer::Screenshot Rasterizer::ScreenshotLastLayerTree(
    Rasterizer::ScreenshotType type,
    bool base64_encode) {
  auto* layer_tree = GetLastLayerTree();
  if (layer_tree == nullptr) {
    FML_LOG(ERROR) << "Last layer tree was null when screenshotting.";
    return {};
  }

  sk_sp<SkData> data = nullptr;

  GrDirectContext* surface_context =
      surface_ ? surface_->GetContext() : nullptr;

  switch (type) {
    case ScreenshotType::SkiaPicture:
      data = ScreenshotLayerTreeAsPicture(layer_tree, *compositor_context_);
      break;
    case ScreenshotType::UncompressedImage:
      data = ScreenshotLayerTreeAsImage(layer_tree, *compositor_context_,
                                        surface_context, false);
      break;
    case ScreenshotType::CompressedImage:
      data = ScreenshotLayerTreeAsImage(layer_tree, *compositor_context_,
                                        surface_context, true);
      break;
  }

  if (data == nullptr) {
    FML_LOG(ERROR) << "Screenshot data was null.";
    return {};
  }

  if (base64_encode) {
    size_t b64_size = SkBase64::Encode(data->data(), data->size(), nullptr);
    auto b64_data = SkData::MakeUninitialized(b64_size);
    SkBase64::Encode(data->data(), data->size(), b64_data->writable_data());
    return Rasterizer::Screenshot{b64_data, layer_tree->frame_size()};
  }

  return Rasterizer::Screenshot{data, layer_tree->frame_size()};
}

void Rasterizer::SetNextFrameCallback(const fml::closure& callback) {
  next_frame_callback_ = callback;
}

void Rasterizer::SetExternalViewEmbedder(
    const std::shared_ptr<ExternalViewEmbedder>& view_embedder) {
  external_view_embedder_ = view_embedder;
}

void Rasterizer::SetSnapshotSurfaceProducer(
    std::unique_ptr<SnapshotSurfaceProducer> producer) {
  snapshot_surface_producer_ = std::move(producer);
}

fml::RefPtr<fml::RasterThreadMerger> Rasterizer::GetRasterThreadMerger() {
  return raster_thread_merger_;
}

void Rasterizer::FireNextFrameCallbackIfPresent() {
  if (!next_frame_callback_) {
    return;
  }
  // It is safe for the callback to set a new callback.
  auto callback = next_frame_callback_;
  next_frame_callback_ = nullptr;
  callback();
}

void Rasterizer::SetResourceCacheMaxBytes(size_t max_bytes, bool from_user) {
  user_override_resource_cache_bytes_ |= from_user;

  if (!from_user && user_override_resource_cache_bytes_) {
    // We should not update the setting here if a user has explicitly set a
    // value for this over the flutter/skia channel.
    return;
  }

  max_cache_bytes_ = max_bytes;
  if (!surface_) {
    return;
  }

  GrDirectContext* context = surface_->GetContext();
  if (context) {
    auto context_switch = surface_->MakeRenderContextCurrent();
    if (!context_switch->GetResult()) {
      return;
    }

    int max_resources;
    context->getResourceCacheLimits(&max_resources, nullptr);
    context->setResourceCacheLimits(max_resources, max_bytes);
  }
}

std::optional<size_t> Rasterizer::GetResourceCacheMaxBytes() const {
  if (!surface_) {
    return std::nullopt;
  }
  GrDirectContext* context = surface_->GetContext();
  if (context) {
    size_t max_bytes;
    context->getResourceCacheLimits(nullptr, &max_bytes);
    return max_bytes;
  }
  return std::nullopt;
}

Rasterizer::Screenshot::Screenshot() {}

Rasterizer::Screenshot::Screenshot(sk_sp<SkData> p_data, SkISize p_size)
    : data(std::move(p_data)), frame_size(p_size) {}

Rasterizer::Screenshot::Screenshot(const Screenshot& other) = default;

Rasterizer::Screenshot::~Screenshot() = default;

}  // namespace flutter
