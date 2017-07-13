// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_SCENE_UPDATE_CONTEXT_H_
#define FLUTTER_FLOW_SCENE_UPDATE_CONTEXT_H_

#include <memory>
#include <vector>

#include "apps/mozart/lib/scene/client/resources.h"
#include "flutter/flow/compositor_context.h"
#include "lib/ftl/build_config.h"
#include "lib/ftl/logging.h"
#include "lib/ftl/macros.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace flow {

class Layer;
class ExportNode;

class SceneUpdateContext {
 public:
  class SurfaceProducer {
   public:
    virtual ~SurfaceProducer() {}
    virtual sk_sp<SkSurface> ProduceSurface(SkISize size,
                                            mozart::client::Session* session,
                                            uint32_t& session_image_id,
                                            mx::event& acquire_release,
                                            mx::event& release_fence) = 0;
  };

  SceneUpdateContext(mozart::client::Session* session,
                     SurfaceProducer* surface_producer);

  ~SceneUpdateContext();

  mozart::client::Session* session() { return session_; }

  class Entity {
   public:
    Entity(SceneUpdateContext& context);
    ~Entity();

    SceneUpdateContext& context() { return context_; }
    mozart::client::EntityNode& entity_node() { return entity_node_; }

   private:
    SceneUpdateContext& context_;
    Entity* const previous_entity_;

    mozart::client::EntityNode entity_node_;
  };

  class Clip : public Entity {
   public:
    Clip(SceneUpdateContext& context,
         mozart::client::Shape& shape,
         const SkRect& shape_bounds);
    ~Clip();
  };

  class Transform : public Entity {
   public:
    Transform(SceneUpdateContext& context, const SkMatrix& transform);
    ~Transform();
  };

  class Frame : public Entity {
   public:
    Frame(SceneUpdateContext& context,
          mozart::client::Shape& shape,
          const SkRect& shape_bounds,
          SkColor color,
          float elevation,
          SkScalar scale_x,
          SkScalar scale_y);
    ~Frame();

    void AddPaintedLayer(Layer* layer);

   private:
    const SkRect& shape_bounds_;
    SkColor const color_;
    SkScalar const scale_x_;
    SkScalar const scale_y_;

    mozart::client::Material material_;
    std::vector<Layer*> paint_layers_;
    SkRect paint_bounds_;
  };

  void AddChildScene(ExportNode* export_node,
                     SkPoint offset,
                     float device_pixel_ratio,
                     bool hit_testable);

  void ExecutePaintTasks(CompositorContext::ScopedFrame& frame);

 private:
  struct PaintTask {
    sk_sp<SkSurface> surface;
    SkScalar left;
    SkScalar top;
    SkScalar scale_x;
    SkScalar scale_y;
    SkColor background_color;
    std::vector<Layer*> layers;
  };

  void PrepareMaterial(mozart::client::Material& material,
                       const SkRect& shape_bounds,
                       SkColor color,
                       SkScalar scale_x,
                       SkScalar scale_y,
                       const SkRect& paint_bounds,
                       std::vector<Layer*> paint_layers);
  uint32_t GenerateTextureIfNeeded(const SkRect& shape_bounds,
                                   SkColor color,
                                   SkScalar scale_x,
                                   SkScalar scale_y,
                                   const SkRect& paint_bounds,
                                   std::vector<Layer*> paint_layers);

  Entity* top_entity_ = nullptr;

  mozart::client::Session* const session_;
  SurfaceProducer* const surface_producer_;

  std::vector<PaintTask> paint_tasks_;

  FTL_DISALLOW_COPY_AND_ASSIGN(SceneUpdateContext);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_SCENE_UPDATE_CONTEXT_H_
