// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "flutter/flow/system_compositor_context.h"

#include "flutter/flow/layers/layer_tree.h"

#include "flutter/flow/layers/layer.h"
#include "flutter/glue/trace_event.h"

namespace flow {

LayerTree::LayerTree()
    : frame_size_{},
      rasterizer_tracing_threshold_(0),
      checkerboard_raster_cache_images_(false),
      checkerboard_offscreen_layers_(false) {}

LayerTree::~LayerTree() = default;

void LayerTree::Raster(CompositorContext::ScopedFrame& frame,
#if defined(OS_FUCHSIA)
                       ui::gfx::Metrics* metrics,
#endif
                       bool ignore_raster_cache) {
#if defined(OS_FUCHSIA)
  FXL_DCHECK(metrics);
#endif
  Preroll(frame,
#if defined(OS_FUCHSIA)
          metrics,
#endif
          ignore_raster_cache);
  frame.systemCompositorContext()->texture_registry =
      &(frame.context().texture_registry());
  UpdateScene(*(frame.systemCompositorContext()));
}

void LayerTree::Preroll(CompositorContext::ScopedFrame& frame,
#if defined(OS_FUCHSIA)
                        ui::gfx::Metrics* metrics,
#endif
                        bool ignore_raster_cache) {
#if defined(OS_FUCHSIA)
  FXL_DCHECK(metrics);
#endif
  TRACE_EVENT0("flutter", "LayerTree::Preroll");
  // TODO(sigurdm): get color space from system compositor context.
  SkColorSpace* color_space = nullptr;
  frame.context().raster_cache().SetCheckboardCacheImages(
      checkerboard_raster_cache_images_);

  Layer::PrerollContext context = {
#if defined(OS_FUCHSIA)
    metrics,
#endif
    ignore_raster_cache ? nullptr : &frame.context().raster_cache(),
    frame.systemCompositorContext()->GetGrContext(),
    color_space,
    SkRect::MakeEmpty(),
  };

  root_layer_->Preroll(&context, SkMatrix::I());
}

void LayerTree::UpdateScene(SystemCompositorContext& context) {
  TRACE_EVENT0("flutter", "LayerTree::UpdateScene");
  context.Transform(SkMatrix::MakeScale(1.f / 2, 1.f / 2));

  if (root_layer_->needs_system_composite()) {
    root_layer_->UpdateScene(context);
  } else if (root_layer_->needs_painting()) {
    context.PushLayer(
        SkRect::MakeWH(frame_size_.width(), frame_size_.height()));
    context.AddPaintedLayer(root_layer_.get());
    context.PopLayer();
  }
  context.PopTransform();
}

}  // namespace flow
