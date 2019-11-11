// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rect_layer.h"

#include "flutter/fml/trace_event.h"

#if defined(OS_FUCHSIA)
#include "flutter/flow/scene_update_context.h"  //nogncheck
#endif  // defined(OS_FUCHSIA)

namespace flutter {

ClipRectLayer::ClipRectLayer(const SkRect& clip_rect, Clip clip_behavior)
    : clip_rect_(clip_rect), clip_behavior_(clip_behavior) {
  FML_DCHECK(clip_behavior != Clip::none);
}

void ClipRectLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  SkRect previous_cull_rect = context->cull_rect;
  SkRect clip_rect_bounds = clip_rect_;
  if (context->cull_rect.intersect(clip_rect_bounds)) {
    context->mutators_stack.PushClipRect(clip_rect_bounds);
    ContainerLayer::Preroll(context, matrix);

    if (clip_rect_bounds.intersect(paint_bounds())) {
      set_paint_bounds(clip_rect_bounds);
    } else {
      set_paint_bounds(SkRect::MakeEmpty());
    }
    context->mutators_stack.Pop();
  }
  context->cull_rect = previous_cull_rect;
}

#if defined(OS_FUCHSIA)

void ClipRectLayer::UpdateScene(SceneUpdateContext& context) {
  FML_DCHECK(needs_system_composite());

  // TODO(liyuqian): respect clip_behavior_
  SceneUpdateContext::Clip clip(context, clip_rect_);
  ContainerLayer::UpdateScene(context);
}

#endif  // defined(OS_FUCHSIA)

void ClipRectLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "ClipRectLayer::Paint");
  FML_DCHECK(needs_painting());

  SkAutoCanvasRestore save(context.internal_nodes_canvas, true);
  context.internal_nodes_canvas->clipRect(clip_rect_,
                                          clip_behavior_ != Clip::hardEdge);

  if (clip_behavior_ == Clip::antiAliasWithSaveLayer) {
    context.internal_nodes_canvas->saveLayer(clip_rect_, nullptr);
  }
  ContainerLayer::Paint(context);
  if (clip_behavior_ == Clip::antiAliasWithSaveLayer) {
    context.internal_nodes_canvas->restore();
  }
}

}  // namespace flutter
