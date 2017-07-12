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

  void PushPhysicalModel(SkRect bounds, SkColor color);
  void PopPhysicalModel(mozart::client::Material& material);

  void AddLayerToCurrentPaintTask(Layer* layer);
  void FinalizeCurrentPaintTaskIfNeeded(
      mozart::client::ContainerNode& container,
      SkScalar scale_x,
      SkScalar scale_y);

  void ExecutePaintTasks(CompositorContext::ScopedFrame& frame);

 private:
  struct CurrentPaintTask {
    CurrentPaintTask();
    void Clear();

    SkRect bounds;
    std::vector<Layer*> layers;
  };

  struct PaintTask {
    sk_sp<SkSurface> surface;
    SkScalar left;
    SkScalar top;
    SkScalar scale_x;
    SkScalar scale_y;
    SkColor background_color;
    std::vector<Layer*> layers;
  };

  struct PhysicalModel {
    SkRect bounds;
    SkColor color;
    uint32_t image_id = 0u;
    bool finalized = false;
  };

  mozart::client::Session* const session_;
  SurfaceProducer* const surface_producer_;

  CurrentPaintTask current_paint_task_;
  std::vector<PaintTask> paint_tasks_;
  std::vector<PhysicalModel> physical_model_stack_;

  FTL_DISALLOW_COPY_AND_ASSIGN(SceneUpdateContext);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_SCENE_UPDATE_CONTEXT_H_
