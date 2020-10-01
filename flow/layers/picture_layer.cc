// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/picture_layer.h"

#include "flutter/fml/logging.h"
#include "third_party/skia/include/core/SkSerialProcs.h"

namespace flutter {

PictureLayer::PictureLayer(const SkPoint& offset,
                           SkiaGPUObject<SkPicture> picture,
                           bool is_complex,
                           bool will_change)
    : offset_(offset),
      picture_(std::move(picture)),
      is_complex_(is_complex),
      will_change_(will_change) {}

void PictureLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  if (!context->IsSubtreeDirty()) {
    assert(old_layer);
    auto prev = old_layer->as_picture_layer();
    if (!Compare(this, prev)) {
      context->MarkSubtreeDirty(prev->paint_region());
    }
  }
  context->PushTransform(SkMatrix::Translate(offset_.x(), offset_.y()));
  context->AddPaintRegion(picture()->cullRect());
  set_paint_region(context->CurrentSubtreeRegion());
}

bool PictureLayer::Compare(const PictureLayer* l1, const PictureLayer* l2) {
  const auto& pic1 = l1->picture_.get();
  const auto& pic2 = l2->picture_.get();
  if (pic1.get() == pic2.get()) {
    return true;
  }
  auto op_cnt_1 = pic1->approximateOpCount();
  auto op_cnt_2 = pic2->approximateOpCount();
  if (op_cnt_1 != op_cnt_2 || pic1->cullRect() != pic2->cullRect()) {
    return false;
  }

  if (op_cnt_1 > 10) {
    return false;
  }

  // TODO(knopp) we don't actually need the data; this could be done without
  // allocations by implementing stream that calculates SHA hash and
  // comparing those hashes
  auto d1 = l1->SerializedPicture();
  auto d2 = l2->SerializedPicture();
  auto res = d1->equals(d2.get());
  return res;
}

sk_sp<SkData> PictureLayer::SerializedPicture() const {
  SkSerialProcs procs = {
      nullptr,
      nullptr,
      [](SkImage* i, void* ctx) {
        auto id = i->uniqueID();
        return SkData::MakeWithCopy(&id, sizeof(id));
      },
      nullptr,
      [](SkTypeface* tf, void* ctx) {
        auto id = tf->uniqueID();
        return SkData::MakeWithCopy(&id, sizeof(id));
      },
      nullptr,
  };

  if (!cached_serialized_picture_) {
    cached_serialized_picture_ = picture_.get()->serialize(&procs);
  } else {
  }
  return cached_serialized_picture_;
}

void PictureLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "PictureLayer::Preroll");

#if defined(LEGACY_FUCHSIA_EMBEDDER)
  CheckForChildLayerBelow(context);
#endif

  SkPicture* sk_picture = picture();

  SkRect bounds = sk_picture->cullRect().makeOffset(offset_.x(), offset_.y());

  if (context->cull_rect.intersects(bounds)) {
    set_paint_bounds(bounds);
  } else {
    set_paint_bounds(SkRect::MakeEmpty());
  }

  if (auto* cache = context->raster_cache) {
    TRACE_EVENT0("flutter", "PictureLayer::RasterCache (Preroll)");

    SkMatrix ctm = matrix;
    ctm.preTranslate(offset_.x(), offset_.y());
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
    ctm = RasterCache::GetIntegralTransCTM(ctm);
#endif
    if (needs_painting()) {
      cache->Prepare(context->gr_context, sk_picture, ctm,
                     context->dst_color_space, is_complex_, will_change_);
    } else {
      // Don't evict raster cache entry during partial repaint
      cache->Touch(sk_picture, ctm);
    }
  }
}

void PictureLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "PictureLayer::Paint");
  FML_DCHECK(picture_.get());
  FML_DCHECK(needs_painting());

  SkAutoCanvasRestore save(context.leaf_nodes_canvas, true);
  context.leaf_nodes_canvas->translate(offset_.x(), offset_.y());
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  context.leaf_nodes_canvas->setMatrix(RasterCache::GetIntegralTransCTM(
      context.leaf_nodes_canvas->getTotalMatrix()));
#endif

  if (context.raster_cache &&
      context.raster_cache->Draw(*picture(), *context.leaf_nodes_canvas)) {
    TRACE_EVENT_INSTANT0("flutter", "raster cache hit");
    return;
  }
  picture()->playback(context.leaf_nodes_canvas);
}

}  // namespace flutter
