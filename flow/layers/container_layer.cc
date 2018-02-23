// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layered_paint_context.h"

namespace flow {

ContainerLayer::ContainerLayer() {}

ContainerLayer::~ContainerLayer() = default;

void ContainerLayer::Add(std::unique_ptr<Layer> layer) {
  layer->set_parent(this);
  layers_.push_back(std::move(layer));
}

void ContainerLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "ContainerLayer::Preroll");

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);
  set_paint_bounds(child_paint_bounds);
}

void ContainerLayer::PrerollChildren(PrerollContext* context,
                                     const SkMatrix& child_matrix,
                                     SkRect* child_paint_bounds) {
  for (auto& layer : layers_) {
    PrerollContext child_context = *context;
    layer->Preroll(&child_context, child_matrix);

    if (layer->needs_system_composite()) {
      set_needs_system_composite(true);
    }
    child_paint_bounds->join(layer->paint_bounds());
  }
}

void ContainerLayer::PaintChildren(PaintContext& context) const {
  FXL_DCHECK(needs_painting());


 // SkCanvas *currentCanvas = &context.canvas;
  // Intentionally not tracing here as there should be no self-time
  // and the trace event on this common function has a small overhead.
 // bool lastWasSystemComposite = false;
//  int pushCount = 0;

  for (auto& layer : layers_) {
    if (layer->needs_painting()) {
      layer->Paint(context);
    }
  }

//      if (lastWasSystemComposite && !layer->needs_system_composite()) {
//        context.layers.PushLayer(paint_bounds());
//        pushCount++;
//        lastWasSystemComposite = false;
//        SkPaint paint;
//        paint.setColor(SK_ColorGREEN);
//        context.layers.CurrentCanvas()->drawString("HEJ", 10, 10, paint);
//        currentCanvas = context.layers.CurrentCanvas();
//      } else {
//        PaintContext newContext { (*currentCanvas),
//                                                   context.frame_time,
//                                                   context.engine_time,
//                                                   context.memory_usage,
//                                                   context.texture_registry,
//                                                   context.checkerboard_offscreen_layers,
//                                                   context.layers
//                                                   };
//
//        layer->Paint(newContext);
//      }
//      if (layer->needs_system_composite()) {
//        lastWasSystemComposite = true;
//      }
//      if (this->needs_system_composite()) {
//        context.layers.PushLayer(layer->paint_bounds());
//        //currentCanvas = context.layers.CurrentCanvas();
//        PaintContext newContext { *(context.layers.CurrentCanvas()),
//                                                   context.frame_time,
//                                                   context.engine_time,
//                                                   context.memory_usage,
//                                                   context.texture_registry,
//                                                   context.checkerboard_offscreen_layers,
//                                                   context.layers
//                                                   };
//
//        layer->Paint(newContext);
//        context.layers.PopLayer();
//      }
//    }
//  }
//  for (int i = 0; i < pushCount; i++) {
//    context.layers.PopLayer();
//  }
}


void ContainerLayer::UpdateScene(LayeredPaintContext &context) {
  UpdateSceneChildren(context);
}

void ContainerLayer::UpdateSceneChildren(LayeredPaintContext &context) {
  FXL_DCHECK(needs_system_composite());
  // FXL_DLOG(INFO) << "Container{";

  // for (auto& layer : layers_) {
  //   if (layer->needs_system_composite()) {
  //     layer->UpdateScene(context);
  //   } else {
  //     context.PushLayer(layer->paint_bounds());
  //     context.AddPaintedLayer(layer.get());
  //     context.PopLayer();
  //   }
  // }

  // Paint all of the layers which need to be drawn into the container.
  // These may be flattened down to a containing
  int pushCount = 0;
  std::vector<Layer*> acc;
  SkRect accBounds = SkRect::MakeEmpty();
  SkRect systemLayerBounds = SkRect::MakeEmpty();

  for (auto& layer : layers_) {
    // FXL_DLOG(INFO) << "Composite " << layer->paint_bounds().x() << "x" << layer->paint_bounds().y() << " " << layer->paint_bounds().width() << "x" << layer->paint_bounds().height() << " updatescenechildren child: " << layer->needs_system_composite() << " needspainting" << layer->needs_painting();
    // FXL_DLOG(INFO) << "Composite " << layer->paint_bounds().intersects(systemLayerBounds);
    
   // FXL_DLOG(INFO) << "system layer bounds " << context.SystemCompositedRect().x() << "x" << context.SystemCompositedRect().y() << " " << context.SystemCompositedRect().width() << "x" << context.SystemCompositedRect().height();
    
    if (layer->needs_system_composite()) {
      systemLayerBounds.join(layer->paint_bounds());
      if (!acc.empty()) {
        if (accBounds.intersect(systemLayerBounds)) {
          context.PushLayer(accBounds);
          pushCount++;
        }
      }
      for (Layer *child : acc) {
        context.AddPaintedLayer(child);          
      }
      acc.clear();
      accBounds = SkRect::MakeEmpty();
      // FXL_DLOG(INFO) << "system layer bounds b" << systemLayerBounds.x() << "x" << systemLayerBounds.y() << " " << systemLayerBounds.width() << "x" << systemLayerBounds.height();
      layer->UpdateScene(context);
    } else if (layer->needs_painting()) {
      if (!acc.empty() || layer->paint_bounds().intersects(systemLayerBounds)) {
        // FXL_DLOG(INFO) << "Intersects";
        acc.push_back(layer.get());
        accBounds.join(layer->paint_bounds());
      } else {
        // FXL_DLOG(INFO) << "Doesn't intersect";
        context.AddPaintedLayer(layer.get());
      }
    }
      // FXL_DLOG(INFO) << "sytem layer bounds c" << systemLayerBounds.x() << "x" << systemLayerBounds.y() << " " << systemLayerBounds.width() << "x" << systemLayerBounds.height();
  }
  if (!acc.empty() && accBounds.intersects(systemLayerBounds)) {
    if (accBounds.intersect(paint_bounds())) {
      context.PushLayer(accBounds);
      pushCount++;
    }
  }
  for (Layer *child : acc) {
    context.AddPaintedLayer(child);          
  }
  for (int i = 0; i < pushCount; i++) {
    context.PopLayer();
  }
    // FXL_DLOG(INFO) << "}Container";
}


}  // namespace flow
