// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/scene_update_context.h"

#include "flutter/flow/export_node.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/matrix_decomposition.h"
#include "flutter/glue/trace_event.h"

namespace flow {

SceneUpdateContext::SceneUpdateContext(mozart::client::Session* session,
                                       SurfaceProducer* surface_producer)
    : session_(session), surface_producer_(surface_producer) {}

SceneUpdateContext::~SceneUpdateContext() = default;

void SceneUpdateContext::AddChildScene(ExportNode* export_node,
                                       SkPoint offset,
                                       float device_pixel_ratio,
                                       bool hit_testable) {
  FTL_DCHECK(top_entity_);

  export_node->Bind(*this, top_entity_->entity_node(), offset,
                    1.f / device_pixel_ratio, hit_testable);
}

void SceneUpdateContext::PrepareMaterial(mozart::client::Material& material,
                                         const SkRect& shape_bounds,
                                         SkColor color,
                                         SkScalar scale_x,
                                         SkScalar scale_y,
                                         const SkRect& paint_bounds,
                                         std::vector<Layer*> paint_layers) {
  uint32_t texture_id =
      GenerateTextureIfNeeded(shape_bounds, color, scale_x, scale_y,
                              paint_bounds, std::move(paint_layers));
  if (texture_id != 0u) {
    material.SetTexture(texture_id);
  } else if (color != SK_ColorTRANSPARENT) {
    material.SetColor(SkColorGetR(color), SkColorGetG(color),
                      SkColorGetB(color), SkColorGetA(color));
  }
}

uint32_t SceneUpdateContext::GenerateTextureIfNeeded(
    const SkRect& shape_bounds,
    SkColor color,
    SkScalar scale_x,
    SkScalar scale_y,
    const SkRect& paint_bounds,
    std::vector<Layer*> paint_layers) {
  // Bail if there's nothing to paint within the shape.
  if (paint_layers.empty() || paint_bounds.isEmpty() ||
      !paint_bounds.intersects(shape_bounds))
    return 0u;

  // Bail if the physical bounds are empty.
  SkISize physical_size = SkISize::Make(shape_bounds.width() * scale_x,
                                        shape_bounds.height() * scale_y);
  if (physical_size.isEmpty())
    return 0u;

  // Acquire a surface from the surface producer and register the paint tasks.
  uint32_t texture_id = 0u;
  mx::event acquire, release;
  auto surface = surface_producer_->ProduceSurface(
      physical_size, session_, texture_id, acquire, release);

  // TODO(chinmaygarde): Check that the acquire and release events are valid.
  if (!surface || texture_id == 0 /* || !acquire || !release */) {
    FTL_LOG(ERROR) << "Could not acquire a surface from the surface producer "
                      "of size: "
                   << physical_size.width() << "x" << physical_size.height()
                   << " Surface: " << static_cast<bool>(surface) << ", "
                   << "Texture Id: " << texture_id << ", "
                   << "Acquire Fence: " << static_cast<bool>(acquire) << ", "
                   << "Release Fence: " << static_cast<bool>(release);
    return 0u;
  }

  // Enqueue the acquire and release fences.
  // session_.EnqueueAcquireFence(std::move(acquire));
  // session_.EnqueueReleaseFence(std::move(release));

  // Enqueue the paint task.
  paint_tasks_.push_back({.surface = std::move(surface),
                          .left = shape_bounds.left(),
                          .top = shape_bounds.top(),
                          .scale_x = scale_x,
                          .scale_y = scale_y,
                          .background_color = color,
                          .layers = std::move(paint_layers)});
  return texture_id;
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

SceneUpdateContext::Entity::Entity(SceneUpdateContext& context)
    : context_(context),
      previous_entity_(context.top_entity_),
      entity_node_(context.session()) {
  if (previous_entity_)
    previous_entity_->entity_node_.AddChild(entity_node_);
  context.top_entity_ = this;
}

SceneUpdateContext::Entity::~Entity() {
  FTL_DCHECK(context_.top_entity_ == this);
  context_.top_entity_ = previous_entity_;
}

SceneUpdateContext::Clip::Clip(SceneUpdateContext& context,
                               mozart::client::Shape& shape,
                               const SkRect& shape_bounds)
    : Entity(context) {
  mozart::client::ShapeNode shape_node(context.session());
  shape_node.SetShape(shape);
  shape_node.SetTranslation(shape_bounds.width() * 0.5f + shape_bounds.left(),
                            shape_bounds.height() * 0.5f + shape_bounds.top(),
                            0.f);

  entity_node().AddPart(shape_node);
  entity_node().SetClip(0u, true /* clip to self */);
}

SceneUpdateContext::Clip::~Clip() = default;

SceneUpdateContext::Transform::Transform(SceneUpdateContext& context,
                                         const SkMatrix& transform)
    : Entity(context) {
  // TODO(chinmaygarde): The perspective and shear components in the matrix
  // are not handled correctly.
  MatrixDecomposition decomposition(transform);
  if (decomposition.IsValid()) {
    entity_node().SetTranslation(decomposition.translation().x(),  //
                                 decomposition.translation().y(),  //
                                 decomposition.translation().z()   //
                                 );
    entity_node().SetScale(decomposition.scale().x(),  //
                           decomposition.scale().y(),  //
                           decomposition.scale().z()   //
                           );
    entity_node().SetRotation(decomposition.rotation().fData[0],  //
                              decomposition.rotation().fData[1],  //
                              decomposition.rotation().fData[2],  //
                              decomposition.rotation().fData[3]   //
                              );
  }
}

SceneUpdateContext::Transform::~Transform() = default;

SceneUpdateContext::Frame::Frame(SceneUpdateContext& context,
                                 mozart::client::Shape& shape,
                                 const SkRect& shape_bounds,
                                 SkColor color,
                                 float elevation,
                                 SkScalar scale_x,
                                 SkScalar scale_y)
    : Entity(context),
      shape_bounds_(shape_bounds),
      color_(color),
      scale_x_(scale_x),
      scale_y_(scale_y),
      material_(context.session()),
      paint_bounds_(SkRect::MakeEmpty()) {
  mozart::client::ShapeNode shape_node(context.session());
  shape_node.SetShape(shape);
  shape_node.SetMaterial(material_);
  shape_node.SetTranslation(shape_bounds.width() * 0.5f + shape_bounds.left(),
                            shape_bounds.height() * 0.5f + shape_bounds.top(),
                            0.f);

  entity_node().AddPart(shape_node);
  entity_node().SetClip(0u, true /* clip to self */);
  entity_node().SetTranslation(0.f, 0.f, elevation);
}

SceneUpdateContext::Frame::~Frame() {
  context().PrepareMaterial(material_, shape_bounds_, color_, scale_x_,
                            scale_y_, paint_bounds_, std::move(paint_layers_));
}

void SceneUpdateContext::Frame::AddPaintedLayer(Layer* layer) {
  FTL_DCHECK(layer->needs_painting());
  paint_layers_.push_back(layer);
  paint_bounds_.join(layer->paint_bounds());
}

}  // namespace flow
