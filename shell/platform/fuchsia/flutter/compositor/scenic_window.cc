// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_window.h"

#include <fuchsia/ui/gfx/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/ui/scenic/cpp/id.h>
#include <lib/ui/scenic/cpp/view_ref_pair.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/trace_event.h"

namespace flutter_runner {
namespace {

// ScenicSession CreateSession(
//     const std::string& debug_label,
//     FlutterEngine& bound_engine,
//     ScenicSession::SessionEventCallback session_event_callback,
//     ScenicWindow::ErrorCallback error_callback,
//     std::shared_ptr<sys::ServiceDirectory> incoming_services) {
//   auto scenic = incoming_services->Connect<fuchsia::ui::scenic::Scenic>();

//   return ScenicSession(
//       debug_label, bound_engine, session_event_callback,
//       [error_callback](zx_status_t status) {
//         FML_LOG(ERROR) << "Interface error " << status
//                        << " on: fuchsia::ui::scenic::Session";
//         error_callback();
//       },
//       scenic.get());
// }

// ScenicInputHandler CreateInputHandler(
//     FlutterEngine& bound_engine,
//     ScenicWindow::ErrorCallback error_callback,
//     fml::RefPtr<fml::TaskRunner> input_runner,
//     fml::RefPtr<fml::TaskRunner> render_runner,
//     std::shared_ptr<sys::ServiceDirectory> incoming_services,
//     ScenicViewController& view_controller) {
//   ScenicInputHandler::RenderDispatchTable render_dispatch_table = {
//       .metrics_changed_callback =
//           [&view_controller,
//            render_runner](fuchsia::ui::gfx::Metrics new_metrics) {
//             fml::TaskRunner::RunNowOrPostTask(
//                 render_runner, [&view_controller, new_metrics]() {
//                   view_controller.OnMetricsChanged(new_metrics);
//                 });
//           },
//       .child_view_connected_callback =
//           [&view_controller, render_runner](scenic::ResourceId view_holder_id) {
//             fml::TaskRunner::RunNowOrPostTask(
//                 render_runner, [&view_controller, view_holder_id]() {
//                   view_controller.OnChildViewConnected(view_holder_id);
//                 });
//           },
//       .child_view_disconnected_callback =
//           [&view_controller, render_runner](scenic::ResourceId view_holder_id) {
//             fml::TaskRunner::RunNowOrPostTask(
//                 render_runner, [&view_controller, view_holder_id]() {
//                   view_controller.OnChildViewDisconnected(view_holder_id);
//                 });
//           },
//       .child_view_state_changed_callback =
//           [&view_controller, render_runner](scenic::ResourceId view_holder_id,
//                                             bool is_rendering) {
//             fml::TaskRunner::RunNowOrPostTask(
//                 render_runner,
//                 [&view_controller, view_holder_id, is_rendering]() {
//                   view_controller.OnChildViewStateChanged(view_holder_id,
//                                                           is_rendering);
//                 });
//           },
//       .view_enable_wireframe_callback =
//           [&view_controller, render_runner](bool enable) {
//             fml::TaskRunner::RunNowOrPostTask(
//                 render_runner, [&view_controller, enable]() {
//                   view_controller.OnEnableWireframe(enable);
//                 });
//           },
//   };

//   auto view_ref = view_controller.view_ref_copy();
//   return ScenicInputHandler(bound_engine, std::move(render_dispatch_table),
//                             std::move(error_callback), std::move(view_ref),
//                             std::move(incoming_services));
// }

}  // end namespace

ScenicWindow::ScenicWindow(const std::string& debug_label,
                           ErrorCallback error_callback,
                           fuchsia::ui::views::ViewToken view_token,
                           std::shared_ptr<sys::ServiceDirectory> incoming_services) {}
//     : session_(CreateSession(debug_label,
//                              flutter_engine_,
//                              std::bind(&ScenicInputHandler::OnScenicEvent,
//                                        &input_handler_,
//                                        std::placeholders::_1),
//                              error_callback,
//                              incoming_services)),
//       view_controller_(session_, std::move(view_token)),
//       input_handler_(CreateInputHandler(flutter_engine_,
//                                         error_callback,
//                                         nullptr,  // TODO input_runner,
//                                         nullptr,  // TODO render_runner,
//                                         incoming_services,
//                                         view_controller_)) {
//   fml::AutoResetWaitableEvent render_latch;
//   fml::TaskRunner::RunNowOrPostTask(
//       nullptr /*TODO render_runner*/,
//       fml::MakeCopyable([this, &render_latch]() mutable {
//         session_.raw().Rebind();  // Rebind the |Session| to the GPU thread's
//                                   // message dispatcher.
//         render_latch.Signal();
//       }));
//   render_latch.Wait();
// }

ScenicWindow::~ScenicWindow() = default;

void ScenicWindow::AwaitPresent(intptr_t baton) {}

bool ScenicWindow::CreateBackingStore(const FlutterBackingStoreConfig* layer_config,
                        FlutterBackingStore* backing_store_out) { return false; }
bool ScenicWindow::CollectBackingStore(const FlutterBackingStore* backing_store) { return false; }
bool ScenicWindow::PresentLayers(const FlutterLayer** layers, size_t layer_count) { return false; }

void ScenicWindow::PlatformMessageResponse(const FlutterPlatformMessage* message) {}
void ScenicWindow::UpdateSemanticsNode(const FlutterSemanticsNode* node) {}
void ScenicWindow::UpdateSemanticsCustomAction(const FlutterSemanticsCustomAction* action) {}

}  // namespace flutter_runner
