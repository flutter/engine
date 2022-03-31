// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/display_list_layer.h"

#include <utility>

#include "flutter/display_list/display_list_builder.h"
#include "flutter/display_list/display_list_flags.h"
#include "flutter/flow/layer_snapshot_store.h"
#include "flutter/flow/layers/offscreen_surface.h"
#include "flutter/flow/raster_cache.h"

namespace flutter {

DisplayListLayer::DisplayListLayer(const SkPoint& offset,
                                   SkiaGPUObject<DisplayList> display_list,
                                   bool is_complex,
                                   bool will_change)
    : offset_(offset),
      display_list_(std::move(display_list)),
      is_complex_(is_complex),
      will_change_(will_change) {
  if (display_list_.skia_object()) {
    set_layer_can_inherit_opacity(
        display_list_.skia_object()->can_apply_group_opacity());
  }
}

bool DisplayListLayer::IsReplacing(DiffContext* context,
                                   const Layer* layer) const {
  // Only return true for identical display lists; This way
  // ContainerLayer::DiffChildren can detect when a display list layer
  // got inserted between other display list layers
  auto old_layer = layer->as_display_list_layer();
  return old_layer != nullptr && offset_ == old_layer->offset_ &&
         Compare(context->statistics(), this, old_layer);
}

void DisplayListLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  if (!context->IsSubtreeDirty()) {
#ifndef NDEBUG
    FML_DCHECK(old_layer);
    auto prev = old_layer->as_display_list_layer();
    DiffContext::Statistics dummy_statistics;
    // IsReplacing has already determined that the display list is same
    FML_DCHECK(prev->offset_ == offset_ &&
               Compare(dummy_statistics, this, prev));
#endif
  }
  context->PushTransform(SkMatrix::Translate(offset_.x(), offset_.y()));
  context->AddLayerBounds(display_list()->bounds());
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

bool DisplayListLayer::Compare(DiffContext::Statistics& statistics,
                               const DisplayListLayer* l1,
                               const DisplayListLayer* l2) {
  const auto& dl1 = l1->display_list_.skia_object();
  const auto& dl2 = l2->display_list_.skia_object();
  if (dl1.get() == dl2.get()) {
    statistics.AddSameInstancePicture();
    return true;
  }
  const auto op_cnt_1 = dl1->op_count();
  const auto op_cnt_2 = dl2->op_count();
  const auto op_bytes_1 = dl1->bytes();
  const auto op_bytes_2 = dl2->bytes();
  if (op_cnt_1 != op_cnt_2 || op_bytes_1 != op_bytes_2 ||
      dl1->bounds() != dl2->bounds()) {
    statistics.AddNewPicture();
    return false;
  }

  if (op_bytes_1 > kMaxBytesToCompare) {
    statistics.AddPictureTooComplexToCompare();
    return false;
  }

  statistics.AddDeepComparePicture();

  auto res = dl1->Equals(*dl2);
  if (res) {
    statistics.AddDifferentInstanceButEqualPicture();
  } else {
    statistics.AddNewPicture();
  }
  return res;
}

void DisplayListLayer::Preroll(PrerollContext* context,
                               const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "DisplayListLayer::Preroll");
  auto cacheable_entry = RasterCacheableEntry::MarkDisplayListCacheable(
      this->display_list(), *context, matrix, offset_, is_complex_,
      will_change_);
  context->raster_cached_entries.push_back(cacheable_entry);

  auto& cache_entry = context->raster_cached_entries.back();
  cache_entry->need_caching = NeedCaching(context, matrix);
}

bool DisplayListLayer::NeedCaching(PrerollContext* context,
                                   const SkMatrix& ctm) {
  DisplayList* disp_list = display_list();
  SkRect bounds = disp_list->bounds().makeOffset(offset_.x(), offset_.y());
  set_paint_bounds(bounds);
  if (auto* cache = context->raster_cache) {
    TRACE_EVENT0("flutter", "DisplayListLayer::RasterCache (Preroll)");
    // For display_list_layer if the context has raster_cache, we will try to
    // collection it when we raster cache it, we will to decision Prepare or
    // Touch cache
    if (context->cull_rect.intersect(bounds) &&
        cache->ShouldBeCached(context, disp_list, is_complex_, will_change_,
                              ctm)) {
      // if current Layer can be cached, we change the
      // subtree_can_inherit_opacity to true
      context->subtree_can_inherit_opacity = true;
    }
    return true;
  }
  return false;
}

void DisplayListLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "DisplayListLayer::Paint");
  FML_DCHECK(display_list_.skia_object());
  FML_DCHECK(needs_painting(context));

  SkAutoCanvasRestore save(context.leaf_nodes_canvas, true);
  context.leaf_nodes_canvas->translate(offset_.x(), offset_.y());

  if (context.raster_cache) {
    AutoCachePaint cache_paint(context);
    if (context.raster_cache->Draw(*display_list(), *context.leaf_nodes_canvas,
                                   cache_paint.paint())) {
      TRACE_EVENT_INSTANT0("flutter", "raster cache hit");
      return;
    }
  }

  if (context.enable_leaf_layer_tracing) {
    const auto canvas_size = context.leaf_nodes_canvas->getBaseLayerSize();
    auto offscreen_surface =
        std::make_unique<OffscreenSurface>(context.gr_context, canvas_size);

    const auto& ctm = context.leaf_nodes_canvas->getTotalMatrix();

    const auto start_time = fml::TimePoint::Now();
    {
      // render display list to offscreen surface.
      auto* canvas = offscreen_surface->GetCanvas();
      SkAutoCanvasRestore save(canvas, true);
      canvas->clear(SK_ColorTRANSPARENT);
      canvas->setMatrix(ctm);
      display_list()->RenderTo(canvas, context.inherited_opacity);
      canvas->flush();
    }
    const fml::TimeDelta offscreen_render_time =
        fml::TimePoint::Now() - start_time;

    const SkRect device_bounds =
        RasterCache::GetDeviceBounds(paint_bounds(), ctm);
    sk_sp<SkData> raster_data = offscreen_surface->GetRasterData(true);
    LayerSnapshotData snapshot_data(unique_id(), offscreen_render_time,
                                    raster_data, device_bounds);
    context.layer_snapshot_store->Add(snapshot_data);
  }

  if (context.leaf_nodes_builder) {
    AutoCachePaint save_paint(context);
    int restore_count = context.leaf_nodes_builder->getSaveCount();
    if (save_paint.paint() != nullptr) {
      DlPaint paint = DlPaint().setAlpha(save_paint.paint()->getAlpha());
      context.leaf_nodes_builder->saveLayer(&paint_bounds(), &paint);
    }
    context.leaf_nodes_builder->drawDisplayList(display_list_.skia_object());
    context.leaf_nodes_builder->restoreToCount(restore_count);
  } else {
    display_list()->RenderTo(context.leaf_nodes_canvas,
                             context.inherited_opacity);
  }
}

}  // namespace flutter
