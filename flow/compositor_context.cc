// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/compositor_context.h"

#include "flutter/flow/layers/layer_tree.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace flutter {

CompositorContext::CompositorContext() = default;

CompositorContext::~CompositorContext() = default;

void CompositorContext::BeginFrame(ScopedFrame& frame,
                                   bool enable_instrumentation) {
  if (enable_instrumentation) {
    frame_count_.Increment();
    raster_time_.Start();
  }
}

void CompositorContext::EndFrame(ScopedFrame& frame,
                                 bool enable_instrumentation) {
  raster_cache_.SweepAfterFrame();
  if (enable_instrumentation) {
    raster_time_.Stop();
  }
}

std::unique_ptr<CompositorContext::ScopedFrame> CompositorContext::AcquireFrame(
    GrContext* gr_context,
    SkCanvas* canvas,
    ExternalViewEmbedder* view_embedder,
    const SkMatrix& root_surface_transformation,
    bool instrumentation_enabled,
    fml::RefPtr<fml::TaskRunnerMerger> task_runner_merger) {
  return std::make_unique<ScopedFrame>(
      *this, gr_context, canvas, view_embedder, root_surface_transformation,
      instrumentation_enabled, task_runner_merger);
}

CompositorContext::ScopedFrame::ScopedFrame(
    CompositorContext& context,
    GrContext* gr_context,
    SkCanvas* canvas,
    ExternalViewEmbedder* view_embedder,
    const SkMatrix& root_surface_transformation,
    bool instrumentation_enabled,
    fml::RefPtr<fml::TaskRunnerMerger> task_runner_merger)
    : context_(context),
      gr_context_(gr_context),
      canvas_(canvas),
      view_embedder_(view_embedder),
      root_surface_transformation_(root_surface_transformation),
      instrumentation_enabled_(instrumentation_enabled),
      task_runner_merger_(task_runner_merger) {
  context_.BeginFrame(*this, instrumentation_enabled_);
}

CompositorContext::ScopedFrame::~ScopedFrame() {
  context_.EndFrame(*this, instrumentation_enabled_);
}

RasterStatus CompositorContext::ScopedFrame::Raster(
    flutter::LayerTree& layer_tree,
    bool ignore_raster_cache) {
  layer_tree.Preroll(*this, ignore_raster_cache);
  if (MergeTaskRunnersForEmbeddedViews()) {
    return RasterStatus::kResubmit;
  }
  // Clearing canvas after preroll reduces one render target switch when preroll
  // paints some raster cache.
  if (canvas()) {
    canvas()->clear(SK_ColorTRANSPARENT);
  }
  layer_tree.Paint(*this, ignore_raster_cache);
  return RasterStatus::kSuccess;
}

const int CompositorContext::ScopedFrame::kDefaultMergedLeaseDuration;

bool CompositorContext::ScopedFrame::MergeTaskRunnersForEmbeddedViews() {
  if (view_embedder_ && task_runner_merger_) {
    const bool uiviews_mutated = view_embedder_->HasPendingViewOperations();
    if (uiviews_mutated) {
      bool are_merged = task_runner_merger_->AreMerged();
      if (are_merged) {
        task_runner_merger_->ExtendLease(kDefaultMergedLeaseDuration);
      } else {
        view_embedder_->CancelFrame();
        task_runner_merger_->MergeWithLease(kDefaultMergedLeaseDuration);
        return true;
      }
    }
  }
  return false;
}

void CompositorContext::OnGrContextCreated() {
  texture_registry_.OnGrContextCreated();
  raster_cache_.Clear();
}

void CompositorContext::OnGrContextDestroyed() {
  texture_registry_.OnGrContextDestroyed();
  raster_cache_.Clear();
}

}  // namespace flutter
