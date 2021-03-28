// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/display_list_layer.h"

#include "flutter/flow/display_list_interpreter.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {

DisplayListLayer::DisplayListLayer(const SkPoint& offset,
                                   const SkRect& cull_rect,
                                   const SkRect& draw_rect,
                                   std::shared_ptr<std::vector<uint8_t>> ops,
                                   std::shared_ptr<std::vector<float>> data,
                                   bool is_complex,
                                   bool will_change)
    : offset_(offset),
      cull_rect_(cull_rect),
      draw_rect_(draw_rect),
      ops_vector_(ops),
      data_vector_(data),
      is_complex_(is_complex),
      will_change_(will_change) {}

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

void DisplayListLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const DisplayListLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (offset_ != prev->offset_ || cull_rect_ != prev->cull_rect_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }
  context->PushTransform(SkMatrix::Translate(offset_.x(), offset_.y()));
  SkRect bounds = draw_rect_;
  if (!bounds.intersect(cull_rect_)) {
    bounds.setEmpty();
  }
  context->AddLayerBounds(bounds);
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

// bool DisplayListLayer::Compare(DiffContext::Statistics& statistics,
//                            const PictureLayer* l1,
//                            const PictureLayer* l2) {
//   const auto& pic1 = l1->picture_.get();
//   const auto& pic2 = l2->picture_.get();
//   if (pic1.get() == pic2.get()) {
//     statistics.AddSameInstancePicture();
//     return true;
//   }
//   auto op_cnt_1 = pic1->approximateOpCount();
//   auto op_cnt_2 = pic2->approximateOpCount();
//   if (op_cnt_1 != op_cnt_2 || pic1->cullRect() != pic2->cullRect()) {
//     statistics.AddNewPicture();
//     return false;
//   }

//   if (op_cnt_1 > 10) {
//     statistics.AddPictureTooComplexToCompare();
//     return false;
//   }

//   statistics.AddDeepComparePicture();

//   // TODO(knopp) we don't actually need the data; this could be done without
//   // allocations by implementing stream that calculates SHA hash and
//   // comparing those hashes
//   auto d1 = l1->SerializedPicture();
//   auto d2 = l2->SerializedPicture();
//   auto res = d1->equals(d2.get());
//   if (res) {
//     statistics.AddDifferentInstanceButEqualPicture();
//   } else {
//     statistics.AddNewPicture();
//   }
//   return res;
// }

// sk_sp<SkData> PictureLayer::SerializedPicture() const {
//   if (!cached_serialized_picture_) {
//     SkSerialProcs procs = {
//         nullptr,
//         nullptr,
//         [](SkImage* i, void* ctx) {
//           auto id = i->uniqueID();
//           return SkData::MakeWithCopy(&id, sizeof(id));
//         },
//         nullptr,
//         [](SkTypeface* tf, void* ctx) {
//           auto id = tf->uniqueID();
//           return SkData::MakeWithCopy(&id, sizeof(id));
//         },
//         nullptr,
//     };
//     cached_serialized_picture_ = picture_.get()->serialize(&procs);
//   }
//   return cached_serialized_picture_;
// }

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

void DisplayListLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "DisplayListLayer::Preroll");

#if defined(LEGACY_FUCHSIA_EMBEDDER)
  CheckForChildLayerBelow(context);
#endif

//   if (auto* cache = context->raster_cache) {
//     TRACE_EVENT0("flutter", "DisplayListLayer::RasterCache (Preroll)");

//     SkMatrix ctm = matrix;
//     ctm.preTranslate(offset_.x(), offset_.y());
// #ifndef SUPPORT_FRACTIONAL_TRANSLATION
//     ctm = RasterCache::GetIntegralTransCTM(ctm);
// #endif
//     cache->Prepare(context->gr_context, sk_picture, ctm,
//                    context->dst_color_space, is_complex_, will_change_);
//   }

  // FML_LOG(ERROR) << "display list cull rect is ["
  //   << cull_rect_.left() << ", "
  //   << cull_rect_.top() << ", "
  //   << cull_rect_.right() << ", "
  //   << cull_rect_.bottom() << "]";
  // FML_LOG(ERROR) << "display list draw rect is ["
  //   << draw_rect_.left() << ", "
  //   << draw_rect_.top() << ", "
  //   << draw_rect_.right() << ", "
  //   << draw_rect_.bottom() << "]";
  SkRect bounds = draw_rect_;
  if (true || bounds.intersect(cull_rect_)) {
    bounds.offset(offset_.x(), offset_.y());
  } else {
    bounds.setEmpty();
  }
  // FML_LOG(ERROR) << "display list paint bounds is ["
  //   << bounds.left() << ", "
  //   << bounds.top() << ", "
  //   << bounds.right() << ", "
  //   << bounds.bottom() << "] "
  //   << ops_vector_.size() << " ops";
  // if (bounds.isEmpty()) {
  //   FML_LOG(ERROR) << "Contents of empty display list:";
  //   DisplayListInterpreter interpreter(ops_vector_, data_vector_);
  //   interpreter.Describe();
  // }
  set_paint_bounds(bounds);
}

void DisplayListLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "DisplayListLayer::Paint");
  FML_DCHECK(needs_painting(context));

  SkAutoCanvasRestore save(context.leaf_nodes_canvas, true);
  context.leaf_nodes_canvas->translate(offset_.x(), offset_.y());
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  context.leaf_nodes_canvas->setMatrix(RasterCache::GetIntegralTransCTM(
      context.leaf_nodes_canvas->getTotalMatrix()));
#endif

//   if (context.raster_cache &&
//       context.raster_cache->Draw(*picture(), *context.leaf_nodes_canvas)) {
//     TRACE_EVENT_INSTANT0("flutter", "raster cache hit");
//     return;
//   }

  // FML_LOG(ERROR) << "painting ["
  //   << paint_bounds().left() << ", "
  //   << paint_bounds().top() << ", "
  //   << paint_bounds().right() << ", "
  //   << paint_bounds().bottom() << "] "
  //   << ops_vector_.size() << " ops";
  DisplayListInterpreter interpreter(ops_vector_, data_vector_);
  interpreter.Rasterize(context.leaf_nodes_canvas);

  // SkRect bounds = paint_bounds();
  SkPaint paint;
  paint.setColor(is_complex_
    ? (will_change_ ? SkColors::kRed : SkColors::kYellow)
    : (will_change_ ? SkColors::kBlue : SkColors::kGreen));
//   paint.setAlphaf(0.125f);
//   context.leaf_nodes_canvas->drawRect(bounds, paint);
//   paint.setStyle(SkPaint::Style::kStroke_Style);
// //   paint.setAlphaf(1.0f);
//   paint.setAntiAlias(true);
//   paint.setColor(SkColors::kBlack);
// //   paint.setStrokeWidth(5.0f);
//   context.leaf_nodes_canvas->drawRect(bounds, paint);
//   context.leaf_nodes_canvas->drawLine(
//     SkPoint::Make(bounds.left(), bounds.top()),
//     SkPoint::Make(bounds.right(), bounds.bottom()),
//     paint);
//   context.leaf_nodes_canvas->drawLine(
//     SkPoint::Make(bounds.right(), bounds.top()),
//     SkPoint::Make(bounds.left(), bounds.bottom()),
//     paint);
}

}  // namespace flutter
