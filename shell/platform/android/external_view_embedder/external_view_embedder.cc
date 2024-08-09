// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/external_view_embedder/external_view_embedder.h"

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "flow/surface_frame.h"
#include "flow/view_slicer.h"
#include "flutter/common/constants.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/trace_event.h"
#include "fml/synchronization/count_down_latch.h"
#include "shell/platform/android/external_view_embedder/surface_pool.h"

namespace flutter {

AndroidExternalViewEmbedder::AndroidExternalViewEmbedder(
    const AndroidContext& android_context,
    std::shared_ptr<PlatformViewAndroidJNI> jni_facade,
    std::shared_ptr<AndroidSurfaceFactory> surface_factory,
    const TaskRunners& task_runners)
    : ExternalViewEmbedder(),
      android_context_(android_context),
      jni_facade_(std::move(jni_facade)),
      surface_factory_(std::move(surface_factory)),
      surface_pool_(std::make_unique<SurfacePool>()),
      task_runners_(task_runners) {}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::PrerollCompositeEmbeddedView(
    int64_t view_id,
    std::unique_ptr<EmbeddedViewParams> params) {
  TRACE_EVENT0("flutter",
               "AndroidExternalViewEmbedder::PrerollCompositeEmbeddedView");

  SkRect view_bounds = SkRect::Make(frame_size_);
  std::unique_ptr<EmbedderViewSlice> view;
  view = std::make_unique<DisplayListEmbedderViewSlice>(view_bounds);
  slices_.insert_or_assign(view_id, std::move(view));

  composition_order_.push_back(view_id);
  // Update params only if they changed.
  if (view_params_.count(view_id) == 1 &&
      view_params_.at(view_id) == *params.get()) {
    return;
  }
  view_params_.insert_or_assign(view_id, EmbeddedViewParams(*params.get()));
}

// |ExternalViewEmbedder|
DlCanvas* AndroidExternalViewEmbedder::CompositeEmbeddedView(int64_t view_id) {
  if (slices_.count(view_id) == 1) {
    return slices_.at(view_id)->canvas();
  }
  return nullptr;
}

SkRect AndroidExternalViewEmbedder::GetViewRect(int64_t view_id) const {
  const EmbeddedViewParams& params = view_params_.at(view_id);
  // TODO(egarciad): The rect should be computed from the mutator stack.
  // (Clipping is missing)
  // https://github.com/flutter/flutter/issues/59821
  return SkRect::MakeXYWH(params.finalBoundingRect().x(),      //
                          params.finalBoundingRect().y(),      //
                          params.finalBoundingRect().width(),  //
                          params.finalBoundingRect().height()  //
  );
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::SubmitFlutterView(
    int64_t flutter_view_id,
    GrDirectContext* context,
    const std::shared_ptr<impeller::AiksContext>& aiks_context,
    std::unique_ptr<SurfaceFrame> frame) {
  TRACE_EVENT0("flutter", "AndroidExternalViewEmbedder::SubmitFlutterView");
  // TODO(dkwingsmt): This class only supports rendering into the implicit view.
  // Properly support multi-view in the future.
  FML_DCHECK(flutter_view_id == kFlutterImplicitViewId);

  if (!FrameHasPlatformLayers() && !had_platform_views_) {
    frame->Submit();
    return;
  }
  had_platform_views_ = FrameHasPlatformLayers();

  std::unordered_map<int64_t, SkRect> view_rects;
  for (auto platform_id : composition_order_) {
    view_rects[platform_id] = GetViewRect(platform_id);
  }

  std::unordered_map<int64_t, SkRect> overlay_layers =
      SliceViews(frame->Canvas(),     //
                 composition_order_,  //
                 slices_,             //
                 view_rects           //
      );

  // Submit the background canvas frame before switching the GL context to
  // the overlay surfaces.
  //
  // Skip a frame if the embedding is switching surfaces, and indicate in
  // `PostPrerollAction` that this frame must be resubmitted.
  auto should_submit_current_frame = previous_frame_view_count_ > 0;
  if (should_submit_current_frame) {
    frame->Submit();
  }

  bool destroy_all_layers =
      surface_pool_->CheckLayerProperties(context, frame_size_);
  if (destroy_all_layers || surface_pool_->size() < overlay_layers.size()) {
    if (destroy_all_layers) {
      surface_pool_->DestroyLayers(jni_facade_);
    }
    for (auto i = surface_pool_->size(); i < overlay_layers.size(); i++) {
      surface_pool_->CreateLayer(context,            //
                                 android_context_,   //
                                 jni_facade_,        //
                                 surface_factory_);  //
    }
  }

  std::unordered_map<int64_t, std::shared_ptr<OverlayLayer>> layers;
  for (const auto& [view_id, rect] : overlay_layers) {
    auto overlay_layer = surface_pool_->GetNextLayer();
    if (!overlay_layer) {
      continue;
    }

    std::unique_ptr<SurfaceFrame> frame =
        overlay_layer->surface->AcquireFrame(frame_size_);
    if (!frame) {
      continue;
    }

    DlCanvas* overlay_canvas = frame->Canvas();

    // Offset the picture since its absolute position on the scene is
    // determined by the position of the overlay view.
    overlay_canvas->Clear(DlColor::kTransparent());
    overlay_canvas->Translate(-rect.x(), -rect.y());
    slices_[view_id]->render_into(overlay_canvas);

    frame->set_submit_info({.frame_boundary = false});
    frame->Submit();
    layers[view_id] = overlay_layer;
  }

  surface_pool_->RecycleLayers();
  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetPlatformTaskRunner(),
      [&, composition_order = composition_order_, view_params = view_params_,
       overlay_layers = std::move(overlay_layers),
       layers = std::move(layers)]() {
        TRACE_EVENT0("flutter",
                     "AndroidExternalViewEmbedder::RenderNativeViews");
        jni_facade_->FlutterViewBeginFrame();

        for (int64_t view_id : composition_order) {
          const EmbeddedViewParams& params = view_params.at(view_id);
          auto view_rect =
              SkRect::MakeXYWH(params.finalBoundingRect().x(),      //
                               params.finalBoundingRect().y(),      //
                               params.finalBoundingRect().width(),  //
                               params.finalBoundingRect().height()  //
              );

          // Display the platform view. If it's already displayed, then it's
          // just positioned and sized.
          jni_facade_->FlutterViewOnDisplayPlatformView(
              view_id,             //
              view_rect.x(),       //
              view_rect.y(),       //
              view_rect.width(),   //
              view_rect.height(),  //
              params.sizePoints().width() * device_pixel_ratio_,
              params.sizePoints().height() * device_pixel_ratio_,
              params.mutatorsStack()  //
          );

          auto overlay_rect = overlay_layers.find(view_id);
          if (overlay_rect == overlay_layers.end()) {
            continue;
          }
          SkRect rect = overlay_rect->second;
          auto maybe_layer = layers.find(view_id);
          if (maybe_layer == layers.end()) {
            continue;
          }
          jni_facade_->FlutterViewDisplayOverlaySurface(
              maybe_layer->second->id,  //
              rect.x(),                 //
              rect.y(),                 //
              rect.width(),             //
              rect.height()             //
          );
        }

        jni_facade_->FlutterViewEndFrame();
      });
}

// |ExternalViewEmbedder|
PostPrerollResult AndroidExternalViewEmbedder::PostPrerollAction(
    const fml::RefPtr<fml::RasterThreadMerger>& raster_thread_merger) {
  if (!FrameHasPlatformLayers()) {
    return PostPrerollResult::kSuccess;
  }

  if (previous_frame_view_count_ == 0) {
    return PostPrerollResult::kResubmitFrame;
  }
  return PostPrerollResult::kSuccess;
}

bool AndroidExternalViewEmbedder::FrameHasPlatformLayers() {
  return !composition_order_.empty();
}

// |ExternalViewEmbedder|
DlCanvas* AndroidExternalViewEmbedder::GetRootCanvas() {
  // On Android, the root surface is created from the on-screen render target.
  return nullptr;
}

void AndroidExternalViewEmbedder::Reset() {
  previous_frame_view_count_ = composition_order_.size();

  composition_order_.clear();
  slices_.clear();
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::BeginFrame(
    GrDirectContext* context,
    const fml::RefPtr<fml::RasterThreadMerger>& raster_thread_merger) {}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::PrepareFlutterView(
    SkISize frame_size,
    double device_pixel_ratio) {
  Reset();
  frame_size_ = frame_size;
  device_pixel_ratio_ = device_pixel_ratio;
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::CancelFrame() {
  Reset();
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::EndFrame(
    bool should_resubmit_frame,
    const fml::RefPtr<fml::RasterThreadMerger>& raster_thread_merger) {}

// |ExternalViewEmbedder|
bool AndroidExternalViewEmbedder::SupportsDynamicThreadMerging() {
  return false;
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::Teardown() {
  // Post a platform task to ensure that the task runner has flushed all
  // platform view composition.
  auto latch = std::make_shared<fml::CountDownLatch>(1u);
  fml::TaskRunner::RunNowOrPostTask(task_runners_.GetPlatformTaskRunner(),
                                    [&latch]() { latch->CountDown(); });
  latch->Wait();
  DestroySurfaces();
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::DestroySurfaces() {
  if (surface_pool_->size() == 0) {
    return;
  }
  surface_pool_->DestroyLayers(jni_facade_);
}

}  // namespace flutter
