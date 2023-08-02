// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/aiks_layer.h"

#include <utility>

#include "flutter/display_list/dl_builder.h"
#include "flutter/flow/layer_snapshot_store.h"
#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/offscreen_surface.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

AiksLayer::AiksLayer(const SkPoint& offset,
                     const std::shared_ptr<const impeller::Picture>& picture,
                     bool is_complex,
                     bool will_change)
    : offset_(offset), picture_(picture) {
  if (picture_) {
    auto bounds = picture_->pass->GetElementsCoverage(std::nullopt)
                      .value_or(impeller::Rect());
    bounds_ = SkRect::MakeXYWH(bounds.origin.x + offset_.x(),
                               bounds.origin.y + offset.y(), bounds.size.width,
                               bounds.size.height);
  }
}

void AiksLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  context->PushTransform(SkMatrix::Translate(offset_.x(), offset_.y()));
  context->AddLayerBounds(bounds_);
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void AiksLayer::Preroll(PrerollContext* frame) {
  // TODO(dnfield): Handle group opacity checks.
  set_paint_bounds(bounds_);
}

void AiksLayer::Paint(PaintContext& context) const {
  FML_DCHECK(needs_painting(context));

  auto mutator = context.state_stack.save();
  mutator.translate(offset_.x(), offset_.y());

  FML_DCHECK(!context.raster_cache);

  SkScalar opacity = context.state_stack.outstanding_opacity();

  if (context.enable_leaf_layer_tracing) {
    // TODO(dnfield): Decide if we need to capture this for Impeller.
    // We can't do this the same way as on the Skia backend, because Impeller
    // does not expose primitives for flushing things down to the GPU without
    // also allocating a texture.
    FML_LOG(ERROR) << "Leaf layer tracing unsupported for Impeller.";
  }

  context.canvas->DrawImpellerPicture(picture_, opacity);
}

}  // namespace flutter
