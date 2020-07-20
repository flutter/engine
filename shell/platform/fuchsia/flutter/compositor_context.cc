// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "compositor_context.h"

#include <vector>

#include "flutter/flow/layers/layer_tree.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter_runner {

class ScopedFrame final : public flutter::CompositorContext::ScopedFrame {
 public:
  ScopedFrame(flutter::CompositorContext& context,
              const SkMatrix& root_surface_transformation,
              flutter::ExternalViewEmbedder* view_embedder,
              bool instrumentation_enabled,
              SessionConnection& session_connection)
      : flutter::CompositorContext::ScopedFrame(
            context,
            session_connection.vulkan_surface_producer()->gr_context(),
            nullptr,
            view_embedder,
            root_surface_transformation,
            instrumentation_enabled,
            true,
            nullptr),
        session_connection_(session_connection) {}

 private:
  SessionConnection& session_connection_;

  flutter::RasterStatus Raster(flutter::LayerTree& layer_tree,
                               bool ignore_raster_cache) override {
    std::vector<flutter::SceneUpdateContext::PaintTask> frame_paint_tasks;
    std::vector<std::unique_ptr<SurfaceProducerSurface>> frame_surfaces;

    {
      // Preroll the Flutter layer tree. This allows Flutter to perform
      // pre-paint optimizations.
      TRACE_EVENT0("flutter", "Preroll");
      layer_tree.Preroll(*this, ignore_raster_cache);
    }

    {
      // Traverse the Flutter layer tree so that the necessary session ops to
      // represent the frame are enqueued in the underlying session.
      TRACE_EVENT0("flutter", "UpdateScene");
      layer_tree.UpdateScene(session_connection_.scene_update_context(),
                             session_connection_.root_node());
      frame_paint_tasks =
          session_connection_.scene_update_context().ResetAndGetPaintTasks();
    }

    {
      // Flush all pending session ops: create surfaces and enqueue session
      // Image ops for the frame's paint tasks, then Present.
      TRACE_EVENT0("flutter", "SessionPresent");
      for (auto& task : frame_paint_tasks) {
        SkISize physical_size =
            SkISize::Make(layer_tree.device_pixel_ratio() * task.scale_x *
                              task.paint_bounds.width(),
                          layer_tree.device_pixel_ratio() * task.scale_y *
                              task.paint_bounds.height());
        std::unique_ptr<SurfaceProducerSurface> surface =
            session_connection_.vulkan_surface_producer()->ProduceSurface(
                physical_size);

        if (!surface) {
          FML_LOG(ERROR)
              << "Could not acquire a surface from the surface producer "
                 "of size: "
              << physical_size.width() << "x" << physical_size.height();
        } else {
          task.material.SetTexture(*(surface->GetImage()));
        }

        frame_surfaces.emplace_back(std::move(surface));
      }

      session_connection_.Present();
    }

    {
      // Execute paint tasks in parallel with Scenic's side of the Present, then
      // signal fences.
      TRACE_EVENT0("flutter", "ExecutePaintTasks");
      size_t surface_index = 0;
      for (auto& task : frame_paint_tasks) {
        std::unique_ptr<SurfaceProducerSurface>& task_surface =
            frame_surfaces[surface_index++];
        if (!task_surface) {
          continue;
        }

        SkCanvas* canvas = task_surface->GetSkiaSurface()->getCanvas();
        flutter::Layer::PaintContext paint_context = {
            canvas,
            canvas,
            gr_context(),
            nullptr,
            context().raster_time(),
            context().ui_time(),
            context().texture_registry(),
            &context().raster_cache(),
            false,
            layer_tree.device_pixel_ratio()};
        canvas->restoreToCount(1);
        canvas->save();
        canvas->clear(task.background_color);
        canvas->scale(layer_tree.device_pixel_ratio() * task.scale_x,
                      layer_tree.device_pixel_ratio() * task.scale_y);
        canvas->translate(-task.paint_bounds.left(), -task.paint_bounds.top());
        for (flutter::Layer* layer : task.layers) {
          layer->Paint(paint_context);
        }
      }

      // Tell the surface producer that a present has occurred so it can perform
      // book-keeping on buffer caches.
      session_connection_.vulkan_surface_producer()->OnSurfacesPresented(
          std::move(frame_surfaces));
    }

    return flutter::RasterStatus::kSuccess;
  }

  FML_DISALLOW_COPY_AND_ASSIGN(ScopedFrame);
};

CompositorContext::CompositorContext(
    std::string debug_label,
    fuchsia::ui::views::ViewToken view_token,
    scenic::ViewRefPair view_ref_pair,
    fidl::InterfaceHandle<fuchsia::ui::scenic::Session> session,
    fml::closure session_error_callback,
    zx_handle_t vsync_event_handle)
    : debug_label_(std::move(debug_label)),
      session_connection_(
          debug_label_,
          std::move(view_token),
          std::move(view_ref_pair),
          std::move(session),
          session_error_callback,
          [](auto) {},
          vsync_event_handle) {}

void CompositorContext::OnWireframeEnabled(bool enabled) {
  session_connection_.set_enable_wireframe(enabled);
}

void CompositorContext::OnCreateView(int64_t view_id,
                                     bool hit_testable,
                                     bool focusable) {
  session_connection_.scene_update_context().CreateView(view_id, hit_testable,
                                                        focusable);
}

void CompositorContext::OnDestroyView(int64_t view_id) {
  session_connection_.scene_update_context().DestroyView(view_id);
}

CompositorContext::~CompositorContext() {
  OnGrContextDestroyed();
}

std::unique_ptr<flutter::CompositorContext::ScopedFrame>
CompositorContext::AcquireFrame(
    GrDirectContext* gr_context,
    SkCanvas* canvas,
    flutter::ExternalViewEmbedder* view_embedder,
    const SkMatrix& root_surface_transformation,
    bool instrumentation_enabled,
    bool surface_supports_readback,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  // TODO: The AcquireFrame interface is too broad and must be refactored to get
  // rid of the context and canvas arguments as those seem to be only used for
  // colorspace correctness purposes on the mobile shells.
  return std::make_unique<flutter_runner::ScopedFrame>(
      *this,                        //
      root_surface_transformation,  //
      view_embedder,
      instrumentation_enabled,  //
      session_connection_       //
  );
}

}  // namespace flutter_runner
