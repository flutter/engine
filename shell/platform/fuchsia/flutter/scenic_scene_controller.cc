// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/scenic_scene_controller.h"

#include <lib/ui/scenic/cpp/commands.h>
#include <zircon/types.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/platform/fuchsia/flutter/vsync_recorder.h"
#include "flutter/shell/platform/fuchsia/flutter/vsync_waiter.h"

namespace flutter_runner {
namespace {

void ToggleSignal(zx_handle_t handle, bool set) {
  const auto signal = VsyncWaiter::SessionPresentSignal;
  auto status = zx_object_signal(handle,            // handle
                                 set ? 0 : signal,  // clear mask
                                 set ? signal : 0   // set mask
  );
  if (status != ZX_OK) {
    FML_LOG(ERROR) << "Could not toggle vsync signal: " << set;
  }
}

}  // end namespace

ScenicSceneController::ScenicSceneController(
    std::string debug_label,
    fml::RefPtr<fml::TaskRunner> task_runner,
    fidl::InterfaceHandle<fuchsia::ui::scenic::Session> session,
    fuchsia::ui::views::ViewToken view_token,
    scenic::ViewRefPair view_ref_pair,
    fml::closure error_callback)
    : task_runner_(std::move(task_runner)),
      session_(session.Bind()),
      root_view_(&session_,
                 std::move(view_token),
                 std::move(view_ref_pair.control_ref),
                 std::move(view_ref_pair.view_ref),
                 debug_label),
      root_node_(&session_) {
  session_.set_error_handler([error_callback](zx_status_t status) {
    FML_LOG(ERROR) << "Interface error " << status
                   << " on: fuchsia::ui::scenic::Session";
    error_callback();
  });
  session_.SetDebugName(debug_label);

  root_view_.AddChild(root_node_);
  root_node_.SetEventMask(fuchsia::ui::gfx::kMetricsEventMask);

  // Create VSync event; signal is initially high indicating availability of
  // the session.
  if (zx::event::create(0, &vsync_event_) != ZX_OK) {
    FML_DLOG(ERROR) << "Could not create the vsync event.";
    return;
  }
  ToggleSignal(vsync_event_.get(), true);
  PresentSession();
}

ScenicSceneController::~ScenicSceneController() = default;

bool ScenicSceneController::CreateBackingStore(
    const FlutterBackingStoreConfig* layer_config,
    FlutterBackingStore* backing_store_out) {
  return false;
}

bool ScenicSceneController::CollectBackingStore(
    const FlutterBackingStore* backing_store) {
  return false;
}

bool ScenicSceneController::PresentLayers(const FlutterLayer** layers,
                                          size_t layer_count) {
  return false;
}

void ScenicSceneController::EnableWireframe(bool enable) {
  session_.Enqueue(
      scenic::NewSetEnableDebugViewBoundsCmd(root_view_.id(), enable));
}

void ScenicSceneController::Present() {
  TRACE_EVENT0("gfx", "ScenicCompositor::Present");
  TRACE_FLOW_BEGIN("gfx", "ScenicCompositor::PresentSession",
                   next_present_session_trace_id_);
  next_present_session_trace_id_++;

  // Throttle vsync if presentation callback is already pending. This allows
  // the paint tasks for this frame to execute in parallel with presentation
  // of last frame but still provides back-pressure to prevent us from queuing
  // even more work.
  if (presentation_callback_pending_) {
    present_session_pending_ = true;
    ToggleSignal(vsync_event_.get(), false);
  } else {
    PresentSession();
  }

  // Execute paint tasks and signal fences.
  // auto surfaces_to_submit = scene_update_context_.ExecutePaintTasks(frame);

  // Tell the surface producer that a present has occurred so it can perform
  // book-keeping on buffer caches.
  // surface_producer_->OnSurfacesPresented(std::move(surfaces_to_submit));
}

void ScenicSceneController::PresentSession() {
  TRACE_EVENT0("gfx", "ScenicCompositor::PresentSession");
  while (processed_present_session_trace_id_ < next_present_session_trace_id_) {
    TRACE_FLOW_END("gfx", "ScenicCompositor::PresentSession",
                   processed_present_session_trace_id_);
    processed_present_session_trace_id_++;
  }
  TRACE_FLOW_BEGIN("gfx", "Session::Present", next_present_trace_id_);
  next_present_trace_id_++;

  // Presentation callback is pending as a result of Present() call below.
  presentation_callback_pending_ = true;

  // Flush all session ops. Paint tasks may not yet have executed but those are
  // fenced. The compositor can start processing ops while we finalize paint
  // tasks.
  session_.Present(
      0,  // presentation_time. (placeholder).
      [this, handle = vsync_event_.get()](
          fuchsia::images::PresentationInfo presentation_info) {
        presentation_callback_pending_ = false;
        VsyncRecorder::GetInstance().UpdateVsyncInfo(presentation_info);

        // Process pending PresentSession() calls.
        if (present_session_pending_) {
          present_session_pending_ = false;
          PresentSession();
        }
        ToggleSignal(handle, true);
      }  // callback
  );
}

}  // namespace flutter_runner
