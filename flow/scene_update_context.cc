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

void SceneUpdateContext::PushPhysicalModel(SkRect bounds, SkColor color) {
  physical_model_stack_.push_back({.bounds = bounds, .color = color});
}

void SceneUpdateContext::PopPhysicalModel(mozart::client::Material& material) {
  FTL_DCHECK(!physical_model_stack_.empty());

  const PhysicalModel& physical_model = physical_model_stack_.back();
  if (physical_model.image_id) {
    material.SetTexture(physical_model.image_id);
  } else {
    material.SetColor(
        SkColorGetR(physical_model.color), SkColorGetG(physical_model.color),
        SkColorGetB(physical_model.color), SkColorGetA(physical_model.color));
  }

  physical_model_stack_.pop_back();
}

void SceneUpdateContext::AddLayerToCurrentPaintTask(Layer* layer) {
  FTL_DCHECK(!layer->needs_system_composite());

  current_paint_task_.bounds.join(layer->paint_bounds());
  current_paint_task_.layers.push_back(layer);
}

void SceneUpdateContext::FinalizeCurrentPaintTaskIfNeeded(
    mozart::client::ContainerNode& container,
    SkScalar scale_x,
    SkScalar scale_y) {
  SkRect bounds = current_paint_task_.bounds;
  std::vector<Layer*> layers = std::move(current_paint_task_.layers);
  current_paint_task_.Clear();

  // Get the containing physical model and mark it finalized.
  // We can only finalize the first set of paint tasks to the physical model.
  // Subsequent paint tasks must be rendered into child nodes.
  SkColor background_color = SK_ColorTRANSPARENT;
  PhysicalModel* physical_model = nullptr;
  if (!physical_model_stack_.empty() &&
      !physical_model_stack_.back().finalized) {
    physical_model = &physical_model_stack_.back();
    physical_model->finalized = true;
    bounds.join(physical_model->bounds);
    background_color = physical_model->color;
  }

  // Bail if there are no paint tasks.
  if (layers.empty())
    return;

  // Bail if the paint bounds are empty.
  SkISize physical_size =
      SkISize::Make(bounds.width() * scale_x, bounds.height() * scale_y);
  if (physical_size.isEmpty())
    return;

  // Acquire a surface from the surface producer and register the paint tasks.
  uint32_t surface_image_id = 0;
  mx::event acquire, release;
  auto surface = surface_producer_->ProduceSurface(
      physical_size, session_, surface_image_id, acquire, release);

  // TODO(chinmaygarde): Check that the acquire and release events are valid.
  if (!surface || surface_image_id == 0 /* || !acquire || !release */) {
    FTL_LOG(ERROR) << "Could not acquire a surface from the surface producer "
                      "of size: "
                   << physical_size.width() << "x" << physical_size.height()
                   << " Surface: " << static_cast<bool>(surface) << ", "
                   << "Image: " << surface_image_id << ", "
                   << "Acquire Fence: " << static_cast<bool>(acquire) << ", "
                   << "Release Fence: " << static_cast<bool>(release);
    return;
  }

  // Enqueue the acquire and release fences.
  // session_.EnqueueAcquireFence(std::move(acquire));
  // session_.EnqueueReleaseFence(std::move(release));

  // Try to rasterize the contents to the containing physical model layer.
  // Otherwise create a node to contain the texture.
  if (physical_model) {
    physical_model->image_id = surface_image_id;
  } else {
    mozart::client::Rectangle surface_rectangle(session_, bounds.width(),
                                                bounds.height());
    mozart::client::Material surface_material(session_);
    surface_material.SetTexture(surface_image_id);
    mozart::client::ShapeNode surface_node(session_);
    surface_node.SetShape(surface_rectangle);
    surface_node.SetMaterial(surface_material);
    surface_node.SetTranslation(bounds.width() * 0.5f + bounds.left(),
                                bounds.height() * 0.5f + bounds.top(), 0.0f);
    container.AddChild(surface_node);
  }

  // Enqueue the paint task.
  paint_tasks_.push_back({.surface = std::move(surface),
                          .left = bounds.left(),
                          .top = bounds.top(),
                          .scale_x = scale_x,
                          .scale_y = scale_y,
                          .background_color = background_color,
                          .layers = std::move(layers)});
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

    canvas->clear(task.background_color);
    canvas->scale(task.scale_x, task.scale_y);
    canvas->translate(-task.left, -task.top);
    for (Layer* layer : task.layers) {
      layer->Paint(context);
    }
    canvas->flush();
  }
  paint_tasks_.clear();
}

}  // namespace flow
