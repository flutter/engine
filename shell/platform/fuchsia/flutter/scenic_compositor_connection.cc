// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/scenic_compositor_connection.h"

#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_request.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/trace_event.h"

namespace flutter_runner {
namespace {

ScenicSceneController* CastToSceneController(void* user_data) {
  return static_cast<ScenicSceneController*>(user_data);
}

fuchsia::ui::views::ViewRef CloneViewRef(
    const fuchsia::ui::views::ViewRef& view_ref) {
  fuchsia::ui::views::ViewRef new_view_ref;
  view_ref.Clone(&new_view_ref);

  return new_view_ref;
}

}  // end namespace

ScenicCompositorConnection::ScenicCompositorConnection(
    std::string debug_label,
    std::shared_ptr<sys::ServiceDirectory> runner_services,
    fml::RefPtr<fml::TaskRunner> platform_task_runner,
    fml::RefPtr<fml::TaskRunner> render_task_runner,
    fuchsia::ui::views::ViewToken view_token,
    fml::closure error_callback) {
  fidl::InterfaceHandle<fuchsia::ui::scenic::Session> session_handle;
  fidl::InterfaceHandle<fuchsia::ui::scenic::SessionListener> session_listener;
  fidl::InterfaceRequest<fuchsia::ui::scenic::SessionListener>
      session_listener_request = session_listener.NewRequest();

  auto view_ref_pair = scenic::ViewRefPair::New();
  auto scenic = runner_services->Connect<fuchsia::ui::scenic::Scenic>();
  scenic->CreateSession(session_handle.NewRequest(),
                        std::move(session_listener));

  platform_task_runner->PostTask(fml::MakeCopyable(
      [this, runner_services = std::move(runner_services),
       session_listener_request = std::move(session_listener_request),
       view_ref = CloneViewRef(view_ref_pair.view_ref),
       platform_task_runner = platform_task_runner, error_callback]() mutable {
        platform_handler_.emplace(std::move(runner_services),
                                  std::move(platform_task_runner),
                                  std::move(session_listener_request),
                                  std::move(view_ref), error_callback);
      }));
  render_task_runner->PostTask(fml::MakeCopyable(
      [this, debug_label = std::move(debug_label),
       session_handle = std::move(session_handle),
       view_token = std::move(view_token),
       view_ref_pair = std::move(view_ref_pair),
       render_task_runner = render_task_runner, error_callback]() mutable {
        scene_controller_.emplace(
            std::move(debug_label), std::move(render_task_runner),
            std::move(session_handle), std::move(view_token),
            std::move(view_ref_pair), error_callback);
      }));
}

ScenicCompositorConnection::~ScenicCompositorConnection() = default;

FlutterCompositor ScenicCompositorConnection::GetCompositorCallbacks() {
  return {
      .struct_size = sizeof(FlutterCompositor),
      .user_data = this,
      .create_backing_store_callback =
          [](const FlutterBackingStoreConfig* layer_config,
             FlutterBackingStore* backing_store_out, void* user_data) {
            TRACE_EVENT0("flutter", "FlutterCompositorCreateBackingStore");
            bool status = CastToSceneController(user_data)->CreateBackingStore(
                layer_config, backing_store_out);
            if (!status) {
              FML_LOG(ERROR) << "Failed creating backing store";
              return false;
            }

            return true;
          },
      .collect_backing_store_callback =
          [](const FlutterBackingStore* backing_store, void* user_data) {
            TRACE_EVENT0("flutter", "FlutterCompositorCollectBackingStore");
            bool status = CastToSceneController(user_data)->CollectBackingStore(
                backing_store);
            if (!status) {
              FML_LOG(ERROR) << "Failed collecting backing store";
              return false;
            }

            return true;
          },
      .present_layers_callback =
          [](const FlutterLayer** layers, size_t layer_count, void* user_data) {
            TRACE_EVENT0("flutter", "FlutterCompositorPresentLayers");
            bool status = CastToSceneController(user_data)->PresentLayers(
                layers, layer_count);
            if (!status) {
              FML_LOG(ERROR) << "Failed presenting layers";
              return false;
            }

            return true;
          },
  };
}

}  // namespace flutter_runner
