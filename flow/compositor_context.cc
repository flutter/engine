// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/compositor_context.h"

#include "flutter/flow/diff_context.h"
#include "flutter/flow/layers/layer_tree.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace flutter {

CompositorContext::CompositorContext(fml::Milliseconds frame_budget)
    : raster_time_(frame_budget), ui_time_(frame_budget) {}

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
    GrDirectContext* gr_context,
    SkCanvas* canvas,
    ExternalViewEmbedder* view_embedder,
    const SkMatrix& root_surface_transformation,
    bool instrumentation_enabled,
    bool surface_supports_readback,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  return std::make_unique<ScopedFrame>(
      *this, gr_context, canvas, view_embedder, root_surface_transformation,
      instrumentation_enabled, surface_supports_readback, raster_thread_merger);
}

CompositorContext::ScopedFrame::ScopedFrame(
    CompositorContext& context,
    GrDirectContext* gr_context,
    SkCanvas* canvas,
    ExternalViewEmbedder* view_embedder,
    const SkMatrix& root_surface_transformation,
    bool instrumentation_enabled,
    bool surface_supports_readback,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger)
    : context_(context),
      gr_context_(gr_context),
      canvas_(canvas),
      view_embedder_(view_embedder),
      root_surface_transformation_(root_surface_transformation),
      instrumentation_enabled_(instrumentation_enabled),
      surface_supports_readback_(surface_supports_readback),
      raster_thread_merger_(raster_thread_merger) {
  context_.BeginFrame(*this, instrumentation_enabled_);
}

CompositorContext::ScopedFrame::~ScopedFrame() {
  context_.EndFrame(*this, instrumentation_enabled_);
}

RasterStatus CompositorContext::ScopedFrame::Raster(
    flutter::LayerTree& layer_tree,
    bool ignore_raster_cache,
    FrameDamage* frame_damage) {
  TRACE_EVENT0("flutter", "CompositorContext::ScopedFrame::Raster");

  SkRect clip_rect = kGiantRect;

  if (frame_damage) {
    DiffContext context(layer_tree.device_pixel_ratio());
    context.PushCullRect(SkRect::MakeIWH(layer_tree.frame_size().width(),
                                         layer_tree.frame_size().height()));

    {
      DiffContext::AutoSubtreeRestore subtree(&context);
      const Layer* prev_root_layer = nullptr;
      if (!frame_damage->prev_layer_tree ||
          frame_damage->prev_layer_tree->frame_size() !=
              layer_tree.frame_size()) {
        context.MarkSubtreeDirty();
      } else {
        prev_root_layer = frame_damage->prev_layer_tree->root_layer();
      }
      layer_tree.root_layer()->Diff(&context, prev_root_layer);
    }

    frame_damage->damage_out =
        context.GetDamage(frame_damage->additional_damage);
    context.statistics().LogStatistics();
    clip_rect = SkRect::Make(frame_damage->damage_out.buffer_damage);
  }

  bool root_needs_readback =
      layer_tree.Preroll(*this, ignore_raster_cache, clip_rect);
  bool needs_save_layer = root_needs_readback && !surface_supports_readback();
  PostPrerollResult post_preroll_result = PostPrerollResult::kSuccess;
  if (view_embedder_ && raster_thread_merger_) {
    post_preroll_result =
        view_embedder_->PostPrerollAction(raster_thread_merger_);
  }

  if (post_preroll_result == PostPrerollResult::kResubmitFrame) {
    return RasterStatus::kResubmit;
  }
  if (post_preroll_result == PostPrerollResult::kSkipAndRetryFrame) {
    return RasterStatus::kSkipAndRetry;
  }

  SkAutoCanvasRestore restore(canvas(), clip_rect != kGiantRect);
  if (clip_rect != kGiantRect) {
    canvas()->clipRect(clip_rect);
  }

  // Clearing canvas after preroll reduces one render target switch when preroll
  // paints some raster cache.
  if (canvas()) {
    if (needs_save_layer) {
      FML_LOG(INFO) << "Using SaveLayer to protect non-readback surface";
      SkRect bounds = SkRect::Make(layer_tree.frame_size());
      SkPaint paint;
      paint.setBlendMode(SkBlendMode::kSrc);
      canvas()->saveLayer(&bounds, &paint);
    }
    canvas()->clear(SK_ColorTRANSPARENT);
  }
  layer_tree.Paint(*this, ignore_raster_cache);
  if (canvas() && needs_save_layer) {
    canvas()->restore();
  }

  return RasterStatus::kSuccess;
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
