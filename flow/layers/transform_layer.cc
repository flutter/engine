// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/transform_layer.h"

#include <optional>

namespace flutter {

TransformLayer::TransformLayer(const SkMatrix& transform, const SkMatrix* cache_transform)
    : transform_(transform) {
  // Checks (in some degree) that SkMatrix transform_ is valid and initialized.
  //
  // If transform_ is uninitialized, this assert may look flaky as it doesn't
  // fail all the time, and some rerun may make it pass. But don't ignore it and
  // just rerun the test if this is triggered, since even a flaky failure here
  // may signify a potentially big problem in the code.
  //
  // We have to write this flaky test because there is no reliable way to test
  // whether a variable is initialized or not in C++.
  FML_DCHECK(transform_.isFinite());
  if (!transform_.isFinite()) {
    FML_LOG(ERROR) << "TransformLayer is constructed with an invalid matrix.";
    transform_.setIdentity();
  }
  if (cache_transform) {
    FML_DCHECK(cache_transform->isFinite());
    if (!cache_transform->isFinite() || !cache_transform->invert(nullptr)) {
      FML_LOG(ERROR) << "TransformLayer is constructed with an invalid cache matrix.";
      cache_requested_ = false;
    } else {
      cache_transform_ = *cache_transform;
      cache_requested_ = true;
    }
  } else {
    cache_requested_ = false;
  }
}

void TransformLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "TransformLayer::Preroll");

  SkMatrix& child_transform = cache_requested_ ? cache_transform_ : transform_;
  SkMatrix child_matrix;
  child_matrix.setConcat(matrix, child_transform);
  context->mutators_stack.PushTransform(child_transform);
  SkRect previous_cull_rect = context->cull_rect;
  SkMatrix inverse_transform_;
  // Perspective projections don't produce rectangles that are useful for
  // culling for some reason.
  if (!child_transform.hasPerspective() && child_transform.invert(&inverse_transform_)) {
    inverse_transform_.mapRect(&context->cull_rect);
  } else {
    context->cull_rect = kGiantRect;
  }
  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, child_matrix, &child_paint_bounds);

  transform_.mapRect(&child_paint_bounds);
  set_paint_bounds(child_paint_bounds);

  context->cull_rect = previous_cull_rect;
  context->mutators_stack.Pop();

  if (cache_requested_) {
    TryToPrepareRasterCache(context, GetCacheableChild(), child_matrix);
  }
}

#if defined(LEGACY_FUCHSIA_EMBEDDER)

void TransformLayer::UpdateScene(SceneUpdateContext& context) {
  TRACE_EVENT0("flutter", "TransformLayer::UpdateScene");
  FML_DCHECK(needs_system_composite());

  std::optional<SceneUpdateContext::Transform> transform;
  if (!transform_.isIdentity()) {
    transform.emplace(context, transform_);
  }

  UpdateSceneChildren(context);
}

#endif

void TransformLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "TransformLayer::Paint");
  FML_DCHECK(needs_painting());

  if (cache_requested_ && context.raster_cache &&
      context.raster_cache->Draw(GetCacheableChild(), *context.internal_nodes_canvas, cache_transform_, transform_)) {
    TRACE_EVENT_INSTANT0("flutter", "raster cache hit");
    return;
  }

  SkAutoCanvasRestore save(context.internal_nodes_canvas, true);
  context.internal_nodes_canvas->concat(transform_);

  PaintChildren(context);
}

}  // namespace flutter
