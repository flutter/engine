// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/compositor_context.h"

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
      *this, gr_context, canvas, view_embedder, &damage_context_,
      root_surface_transformation, instrumentation_enabled,
      surface_supports_readback, raster_thread_merger);
}

CompositorContext::ScopedFrame::ScopedFrame(
    CompositorContext& context,
    GrDirectContext* gr_context,
    SkCanvas* canvas,
    ExternalViewEmbedder* view_embedder,
    DamageContext* damage_context,
    const SkMatrix& root_surface_transformation,
    bool instrumentation_enabled,
    bool surface_supports_readback,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger)
    : context_(context),
      gr_context_(gr_context),
      canvas_(canvas),
      view_embedder_(view_embedder),
      damage_context_(damage_context),
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

  std::optional<DamageArea> damage_area;

  if (frame_damage) {
    damage_context()->InitFrame(layer_tree.frame_size(),
                                frame_damage->previous_frame_description);
    layer_tree.Preroll(*this, true);
    auto damage_res = damage_context()->FinishFrame();
    frame_damage->frame_description = std::move(damage_res.frame_description);

    // only bother clipping when less than 80% of screen
    if (damage_res.area.bounds().width() * damage_res.area.bounds().height() <
        0.8 * layer_tree.frame_size().width() *
            layer_tree.frame_size().height()) {
      damage_area = std::move(damage_res.area);
    }
  }

  auto cull_rect =
      damage_area ? SkRect::Make(damage_area->bounds()) : kGiantRect;
  bool root_needs_readback =
      layer_tree.Preroll(*this, ignore_raster_cache, cull_rect);
  bool needs_save_layer = root_needs_readback && !surface_supports_readback();
  PostPrerollResult post_preroll_result = PostPrerollResult::kSuccess;
  if (view_embedder_ && raster_thread_merger_) {
    post_preroll_result =
        view_embedder_->PostPrerollAction(raster_thread_merger_);
  }

  if (post_preroll_result == PostPrerollResult::kResubmitFrame) {
    return RasterStatus::kResubmit;
  }

  if (canvas() && damage_area) {
    canvas()->save();
    canvas()->clipRect(SkRect::Make(damage_area->bounds()));
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
  if (canvas() && damage_area) {
    canvas()->restore();
  }

  if (frame_damage) {
    frame_damage->damage_area = std::move(damage_area);
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
