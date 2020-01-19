// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_window.h"

#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/async/cpp/task.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/ui/scenic/cpp/id.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>
#include <zircon/status.h>

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/third_party/tonic/converter/dart_converter.h"
#include "flutter/third_party/tonic/logging/dart_error.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace flutter_runner {
namespace {

ScenicPlatformBridge::RenderDispatchTable GetRenderDispatchTable(
    ScenicSession& session,
    std::optional<ScenicView>& view,
    async_dispatcher_t* raster_dispatcher) {
  return {
      .child_view_connected_callback =
          [&session, &view,
           raster_dispatcher](scenic::ResourceId view_holder_id) {
            async::PostTask(raster_dispatcher,
                            [&session, &view, view_holder_id]() {
                              FX_DCHECK(session.connected());
                              view->OnChildViewConnected(view_holder_id);
                            });
          },
      .child_view_disconnected_callback =
          [&session, &view,
           raster_dispatcher](scenic::ResourceId view_holder_id) {
            async::PostTask(raster_dispatcher,
                            [&session, &view, view_holder_id]() {
                              FX_DCHECK(session.connected());
                              view->OnChildViewDisconnected(view_holder_id);
                            });
          },
      .child_view_state_changed_callback =
          [&session, &view, raster_dispatcher](
              scenic::ResourceId view_holder_id, bool is_rendering) {
            async::PostTask(raster_dispatcher, [&session, &view, view_holder_id,
                                                is_rendering]() {
              FX_DCHECK(session.connected());
              view->OnChildViewStateChanged(view_holder_id, is_rendering);
            });
          },
      .metrics_changed_callback =
          [&session, &view,
           raster_dispatcher](const FlutterWindowMetricsEvent* metrics) {
            async::PostTask(
                raster_dispatcher,
                [&session, &view, pixel_ratio = metrics->pixel_ratio]() {
                  FX_DCHECK(session.connected());
                  view->OnPixelRatioChanged(pixel_ratio);
                });
          },
      .view_enable_wireframe_callback =
          [&session, &view, raster_dispatcher](bool enable) {
            async::PostTask(raster_dispatcher, [&session, &view, enable]() {
              FX_DCHECK(session.connected());
              view->OnEnableWireframe(enable);
            });
          },
  };
}

fuchsia::ui::views::ViewRef CloneViewRef(
    const fuchsia::ui::views::ViewRef& view_ref) {
  fuchsia::ui::views::ViewRef clone;
  fidl::Clone(view_ref, &clone);

  return clone;
}

}  // namespace

ScenicWindow::ScenicWindow(Context context)
    : context_(std::move(context)),
      view_ref_pair_(scenic::ViewRefPair::New()),
      platform_bridge_(
          context_.dispatch_table,
          GetRenderDispatchTable(session_, view_, context_.raster_dispatcher),
          CloneViewRef(view_ref_pair_.view_ref),
          context_.incoming_services),
      view_provider_binding_(this) {
  view_provider_binding_.set_error_handler([this](zx_status_t status) {
    FX_LOG(ERROR)
        << "Interface error (binding) for fuchsia::ui::app::ViewProvider: "
        << zx_status_get_string(status);
    context_.dispatch_table.error_callback();
  });
}

void ScenicWindow::BindServices(BindServiceCallback bind_service_callback) {
  fidl::InterfaceRequestHandler<fuchsia::ui::app::ViewProvider> view_handler =
      [this](fidl::InterfaceRequest<fuchsia::ui::app::ViewProvider> request) {
        FX_DCHECK(!view_provider_binding_.is_bound());
        view_provider_binding_.Bind(std::move(request));
      };
  bind_service_callback(
      fuchsia::ui::app::ViewProvider::Name_,
      std::make_unique<vfs::Service>(std::move(view_handler)));
}

void ScenicWindow::ConfigureCurrentIsolate() {
  Dart_Handle library = Dart_LookupLibrary(tonic::ToDart("dart:fuchsia"));
  FX_CHECK(!tonic::LogIfError(library));

  auto view_ref = CloneViewRef(view_ref_pair_.view_ref);
  Dart_Handle result = Dart_SetField(library, tonic::ToDart("_viewRef"),
                                     tonic::ToDart(zircon::dart::Handle::Create(
                                         std::move(view_ref.reference))));
  FX_CHECK(!tonic::LogIfError(result));
}

void ScenicWindow::PlatformMessageResponse(
    const FlutterPlatformMessage* message) {
  platform_bridge_.PlatformMessageResponse(message);
}

void ScenicWindow::UpdateSemanticsNode(const FlutterSemanticsNode* node) {
  platform_bridge_.UpdateSemanticsNode(node);
}

void ScenicWindow::UpdateSemanticsCustomAction(
    const FlutterSemanticsCustomAction* action) {
  platform_bridge_.UpdateSemanticsCustomAction(action);
}

void ScenicWindow::AwaitVsync(intptr_t baton) {
  session_.AwaitVsync(baton);
}

bool ScenicWindow::CreateBackingStore(
    const FlutterBackingStoreConfig* layer_config,
    FlutterBackingStore* backing_store_out) {
  FX_DCHECK(session_.connected());

  return view_->CreateBackingStore(layer_config, backing_store_out);
}

bool ScenicWindow::CollectBackingStore(
    const FlutterBackingStore* backing_store) {
  FX_DCHECK(session_.connected());

  return view_->CollectBackingStore(backing_store);
}

bool ScenicWindow::PresentLayers(const FlutterLayer** layers,
                                 size_t layer_count) {
  FX_DCHECK(session_.connected());

  return view_->PresentLayers(layers, layer_count);
}

bool ScenicWindow::IsBackingStoreAvailable(
    const FlutterBackingStore* backing_store) {
  FX_DCHECK(session_.connected());

  return view_->IsBackingStoreAvailable(backing_store);
}

void ScenicWindow::CreateView(
    zx::eventpair token,
    fidl::InterfaceRequest<
        fuchsia::sys::ServiceProvider> /* incoming_services */,
    fidl::InterfaceHandle<
        fuchsia::sys::ServiceProvider> /* outgoing_services */) {
  FX_DCHECK(!session_.connected());

  // Connect to Scenic and create the View on the raster thread.
  async::PostTask(
      context_.raster_dispatcher, [this, token = std::move(token)]() mutable {
        session_.Connect(context_,
                         std::bind(&ScenicPlatformBridge::OnScenicEvent,
                                   &platform_bridge_, std::placeholders::_1));
        view_.emplace(context_.debug_label, session_,
                      scenic::ToViewToken(std::move(token)),
                      std::move(view_ref_pair_));
      });
}

}  // namespace flutter_runner
