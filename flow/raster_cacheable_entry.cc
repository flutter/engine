// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cacheable_entry.h"

#include <utility>

#include "flutter/display_list/display_list.h"
#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/raster_cache_key.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"

namespace flutter {

void CacheableItem::Reset() {
  need_cached_ = true;
  child_entries_ = 0;
  cull_rect_ = kGiantRect;
}

bool CacheableItem::Prepare(PaintContext* context) const {
  return context->raster_cache->Prepare(this, context);
}

bool CacheableItem::Touch(const RasterCache* raster_cache,
                          bool parent_cached) const {
  return raster_cache->Touch(this, parent_cached);
}

bool CacheableItem::Draw(const RasterCache* raster_cache,
                         SkCanvas& canvas,
                         const SkPaint* paint) const {
  return raster_cache->Draw(this, canvas, paint);
}

LayerCacheableItem::LayerCacheableItem(Layer* layer, const SkMatrix& matrix)
    : CacheableItem(matrix), layer_(layer) {}

std::optional<RasterCacheKey> LayerCacheableItem::GetKey(
    SkCanvas* canvas) const {
  if (canvas) {
    return TryToMakeRasterCacheKeyForLayer(strategy_, canvas->getTotalMatrix());
  }
  return TryToMakeRasterCacheKeyForLayer(strategy_, matrix_);
}

std::unique_ptr<RasterCacheResult> LayerCacheableItem::CreateRasterCache(
    PaintContext* paint_context,
    bool checkerboard) const {
  const SkRect& paint_bounds = GetPaintBoundsFromLayer();

  return RasterCache::Rasterize(
      paint_context->gr_context, matrix_, paint_context->dst_color_space,
      checkerboard, paint_bounds, "RasterCacheFlow::Layer",
      [this, paint_context](SkCanvas* canvas) {
        SkISize canvas_size = canvas->getBaseLayerSize();
        SkNWayCanvas internal_nodes_canvas(canvas_size.width(),
                                           canvas_size.height());
        internal_nodes_canvas.setMatrix(canvas->getTotalMatrix());
        internal_nodes_canvas.addCanvas(canvas);
        PaintContext context = {
            // clang-format off
          .internal_nodes_canvas         = static_cast<SkCanvas*>(&internal_nodes_canvas),
          .leaf_nodes_canvas             = canvas,
          .gr_context                    = paint_context->gr_context,
          .dst_color_space               = paint_context->dst_color_space,
          .view_embedder                 = paint_context->view_embedder,
          .raster_time                   = paint_context->raster_time,
          .ui_time                       = paint_context->ui_time,
          .texture_registry              = paint_context->texture_registry,
          .raster_cache                  = paint_context->raster_cache,
          .checkerboard_offscreen_layers = paint_context->checkerboard_offscreen_layers,
          .frame_device_pixel_ratio      = paint_context->frame_device_pixel_ratio,
            // clang-format on
        };

        switch (strategy_) {
          case RasterCacheLayerStrategy::kLayer:
            if (layer_->needs_painting(context)) {
              layer_->Paint(context);
            }
            break;
          case RasterCacheLayerStrategy::kLayerChildren:
            FML_DCHECK(layer_->as_container_layer());
            layer_->as_container_layer()->PaintChildren(context);
            break;
        }
      });
}

std::optional<RasterCacheKey>
LayerCacheableItem::TryToMakeRasterCacheKeyForLayer(
    RasterCacheLayerStrategy strategy,
    const SkMatrix& ctm) const {
  switch (strategy) {
    case RasterCacheLayerStrategy::kLayer:
      return RasterCacheKey(layer_->unique_id(), RasterCacheKeyType::kLayer,
                            ctm);
    case RasterCacheLayerStrategy::kLayerChildren:
      FML_DCHECK(layer_->as_container_layer());
      auto& children_layers = layer_->as_container_layer()->layers();
      auto children_count = children_layers.size();
      if (children_count == 0) {
        return std::nullopt;
      }
      std::vector<uint64_t> ids;
      std::transform(children_layers.begin(), children_layers.end(),
                     std::back_inserter(ids), [](auto& layer) -> uint64_t {
                       return layer->unique_id();
                     });
      return RasterCacheKey(RasterCacheKeyID(std::move(ids)),
                            RasterCacheKeyType::kLayerChildren, ctm);
  }
}

bool LayerCacheableItem::TryToPrepareRasterCache(PaintContext* context) const {
  if (has_platform_view_ || has_texture_layer_ ||
      !SkRect::Intersects(cull_rect_, layer_->paint_bounds())) {
    Touch(context->raster_cache, false);
    return false;
  }

  return Prepare(context);
}

const SkRect& LayerCacheableItem::GetPaintBoundsFromLayer() const {
  switch (strategy_) {
    case RasterCacheLayerStrategy::kLayer:
      return layer_->paint_bounds();
    case RasterCacheLayerStrategy::kLayerChildren:
      FML_DCHECK(layer_->as_container_layer());
      return layer_->as_container_layer()->child_paint_bounds();
  }
}

DisplayListCacheableItem::DisplayListCacheableItem(DisplayList* display_list,
                                                   const SkRect& bounds,
                                                   const SkMatrix& matrix,
                                                   bool is_complex,
                                                   bool will_change)
    : CacheableItem(matrix),
      display_list_(display_list),
      bounds_(bounds),
      is_complex_(is_complex),
      will_change_(will_change) {}

std::optional<RasterCacheKey> DisplayListCacheableItem::GetKey(
    SkCanvas* canvas) const {
  if (canvas) {
    return (RasterCacheKey){display_list_->unique_id(),
                            RasterCacheKeyType::kDisplayList,
                            canvas->getTotalMatrix()};
  }
  return (RasterCacheKey){display_list_->unique_id(),
                          RasterCacheKeyType::kDisplayList, matrix_};
}

bool DisplayListCacheableItem::Prepare(PaintContext* paint_context) const {
  if (!matrix_.invert(nullptr)) {
    // The matrix was singular. No point in going further.
    return false;
  }
  return CacheableItem::Prepare(paint_context);
}

bool DisplayListCacheableItem::ShouldBeCached(
    const RasterCache* raster_cache,
    const GrDirectContext* gr_context) const {
  auto transformation_matrix = matrix_;

  if (!transformation_matrix.invert(nullptr)) {
    // The matrix was singular. No point in going further.
    return false;
  }
  if (!raster_cache->GenerateNewCacheInThisFrame()) {
    return false;
  }

  DisplayListComplexityCalculator* complexity_calculator =
      gr_context ? DisplayListComplexityCalculator::GetForBackend(
                       gr_context->backend())
                 : DisplayListComplexityCalculator::GetForSoftware();

  if (!RasterCache::IsDisplayListWorthRasterizing(
          display_list_, will_change_, is_complex_, complexity_calculator)) {
    // We only deal with display lists that are worthy of rasterization.
    return false;
  }

  RasterCache::Entry& entry = raster_cache->EntryForKey(GetKey().value());
  if (entry.access_count < raster_cache->access_threshold()) {
    // Frame threshold has not yet been reached.
    return false;
  }

  return true;
}

bool DisplayListCacheableItem::TryToPrepareRasterCache(
    PaintContext* context) const {
  auto cull_rect = cull_rect_;
  if (cull_rect.intersect(bounds_)) {
    Prepare(context);
    return true;
  }
  // current bound is not intersect with cull_rect, touch the sk_picture
  Touch(context->raster_cache, false);
  return false;
}

std::unique_ptr<RasterCacheResult> DisplayListCacheableItem::CreateRasterCache(
    PaintContext* paint_context,
    bool checkerboard) const {
  auto transformation_matrix = matrix_;
  // GetIntegralTransCTM effect for matrix which only contains scale,
  // translate, so it won't affect result of matrix decomposition and cache
  // key.
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  transformation_matrix =
      RasterCache::GetIntegralTransCTM(transformation_matrix);
#endif

  return RasterCache::Rasterize(
      paint_context->gr_context, matrix_, paint_context->dst_color_space,
      checkerboard, display_list_->bounds(), "RasterCacheFlow::DisplayList",
      [this](SkCanvas* canvas) { display_list_->RenderTo(canvas); });
}

SkPictureCacheableItem::SkPictureCacheableItem(SkPicture* sk_picture,
                                               const SkRect& bounds,
                                               const SkMatrix& matrix,
                                               bool is_complex,
                                               bool will_change)
    : CacheableItem(matrix),
      sk_picture_(sk_picture),
      bounds_(bounds),
      is_complex_(is_complex),
      will_change_(will_change) {}

std::optional<RasterCacheKey> SkPictureCacheableItem::GetKey(
    SkCanvas* canvas) const {
  if (canvas) {
    return (RasterCacheKey){sk_picture_->uniqueID(),
                            RasterCacheKeyType::kDisplayList,
                            canvas->getTotalMatrix()};
  }

  return (RasterCacheKey){sk_picture_->uniqueID(),
                          RasterCacheKeyType::kDisplayList, matrix_};
}

bool SkPictureCacheableItem::Prepare(PaintContext* paint_context) const {
  if (!matrix_.invert(nullptr)) {
    // The matrix was singular. No point in going further.
    return false;
  }
  return CacheableItem::Prepare(paint_context);
}

bool SkPictureCacheableItem::ShouldBeCached(
    const RasterCache* raster_cache) const {
  if (!raster_cache->GenerateNewCacheInThisFrame()) {
    return false;
  }

  if (!RasterCache::IsPictureWorthRasterizing(sk_picture_, will_change_,
                                              is_complex_)) {
    // We only deal with pictures that are worthy of rasterization.
    return false;
  }

  if (!matrix_.invert(nullptr)) {
    // The matrix was singular. No point in going further.
    return false;
  }

  RasterCache::Entry& entry = raster_cache->EntryForKey(GetKey().value());
  if (entry.access_count < raster_cache->access_threshold()) {
    // Frame threshold has not yet been reached.
    return false;
  }

  return true;
}

bool SkPictureCacheableItem::TryToPrepareRasterCache(
    PaintContext* context) const {
  auto cull_rect = cull_rect_;
  if (cull_rect.intersect(bounds_)) {
    Prepare(context);
    return true;
  }
  // current bound is not intersect with cull_rect, touch the sk_picture
  Touch(context->raster_cache, false);
  return false;
}

std::unique_ptr<RasterCacheResult> SkPictureCacheableItem::CreateRasterCache(
    PaintContext* paint_context,
    bool checkerboard) const {
  auto transformation_matrix = matrix_;
  // GetIntegralTransCTM effect for matrix which only contains scale,
  // translate, so it won't affect result of matrix decomposition and cache
  // key.
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  transformation_matrix =
      RasterCache::GetIntegralTransCTM(transformation_matrix);
#endif

  return RasterCache::Rasterize(
      paint_context->gr_context, matrix_, paint_context->dst_color_space,
      checkerboard, sk_picture_->cullRect(), "RasterCacheFlow::SkPicture",
      [this](SkCanvas* canvas) { canvas->drawPicture(sk_picture_); });
}

}  // namespace flutter
