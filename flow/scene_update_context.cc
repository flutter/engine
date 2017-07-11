// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/scene_update_context.h"

#include "flutter/flow/layers/layer.h"
#include "flutter/glue/trace_event.h"

namespace flow {

SceneUpdateContext::CurrentPaintTask::CurrentPaintTask()
    : bounds(SkRect::MakeEmpty()) {}

void SceneUpdateContext::CurrentPaintTask::Clear() {
  bounds = SkRect::MakeEmpty();
  layers.clear();
}

SceneUpdateContext::SceneUpdateContext(mozart::client::Session* session,
                                       SurfaceProducer* surface_producer)
    : session_(session), surface_producer_(surface_producer) {}

SceneUpdateContext::~SceneUpdateContext() = default;

void SceneUpdateContext::AddLayerToCurrentPaintTask(Layer* layer) {
  FTL_DCHECK(!layer->needs_system_composite());

  current_paint_task_.bounds.join(layer->paint_bounds());
  current_paint_task_.layers.push_back(layer);
}

void SceneUpdateContext::FinalizeCurrentPaintTaskIfNeeded(
    mozart::client::ContainerNode& container,
    const SkMatrix& ctm) {
  if (current_paint_task_.layers.empty()) {
    return;
  }

  const SkRect& bounds = current_paint_task_.bounds;

  SkScalar scaleX = ctm.getScaleX();
  SkScalar scaleY = ctm.getScaleY();

  SkISize physical_size =
      SkISize::Make(bounds.width() * scaleX, bounds.height() * scaleY);

  if (physical_size.isEmpty()) {
    current_paint_task_.Clear();
    return;
  }

  // Acquire a surface from the surface producer and register the paint tasks.

  uint32_t session_image_id = 0;
  mx::event acquire, release;
  auto surface = surface_producer_->ProduceSurface(
      physical_size, session_, session_image_id, acquire, release);

  // TODO(chinmaygarde): Check that the acquire and release events are valid.
  if (!surface || session_image_id == 0 /* || !acquire || !release */) {
    FTL_LOG(ERROR) << "Could not acquire a surface from the surface producer "
                      "of size: "
                   << physical_size.width() << "x" << physical_size.height()
                   << " Surface: " << static_cast<bool>(surface) << ", "
                   << "Session Image: " << session_image_id << ", "
                   << "Acquire Fence: " << static_cast<bool>(acquire) << ", "
                   << "Release Fence: " << static_cast<bool>(release);
    return;
  }

  // Enqueue the acquire and release fences.
  // session_.EnqueueAcquireFence(std::move(acquire));
  // session_.EnqueueReleaseFence(std::move(release));

  PaintTask task;
  task.surface = surface;
  task.left = bounds.left();
  task.top = bounds.top();
  task.scaleX = scaleX;
  task.scaleY = scaleY;
  task.layers = std::move(current_paint_task_.layers);

  // Enqueue session ops for the node with the surface as the texture.
  mozart::client::ShapeNode node(session_);

  // The node has a rectangular shape.
  mozart::client::Rectangle rectangle(session_,        //
                                      bounds.width(),  //
                                      bounds.height()  //
                                      );
  node.SetShape(rectangle);

  // The rectangular shape is filled in with a texture that is the content
  // that are rendered into the surface that we just setup.
  mozart::client::Material texture_material(session_);
  texture_material.SetTexture(session_image_id);
  node.SetMaterial(texture_material);
  node.SetTranslation(bounds.width() * 0.5f + bounds.left(),  //
                      bounds.height() * 0.5f + bounds.top(),  //
                      0.0f                                    //
                      );

  // Add the node as a child of the container.
  container.AddChild(node);

  // Ensure that the paint task is registered. We will execute these tasks as
  // the session ops are being flushed.
  paint_tasks_.push_back(task);
}

void SceneUpdateContext::ExecutePaintTasks(
    CompositorContext::ScopedFrame& frame) {
  TRACE_EVENT0("flutter", "SceneUpdateContext::ExecutePaintTasks");

  for (auto& task : paint_tasks_) {
    FTL_DCHECK(task.surface);
    SkCanvas* canvas = task.surface->getCanvas();
    Layer::PaintContext context = {*canvas, frame.context().frame_time(),
                                   frame.context().engine_time(),
                                   frame.context().memory_usage(), false};

    canvas->clear(SK_ColorTRANSPARENT);
    canvas->scale(task.scaleX, task.scaleY);
    canvas->translate(-task.left, -task.top);
    for (Layer* layer : task.layers) {
      layer->Paint(context);
    }
    canvas->flush();
  }

  paint_tasks_.clear();
}

}  // namespace flow
