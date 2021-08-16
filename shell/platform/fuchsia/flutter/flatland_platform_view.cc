// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flatland_platform_view.h"

#include "flutter/fml/make_copyable.h"

namespace flutter_runner {

FlatlandPlatformView::FlatlandPlatformView(
    flutter::PlatformView::Delegate& delegate,
    std::string debug_label,
    fuchsia::ui::views::ViewRef view_ref,
    flutter::TaskRunners task_runners,
    std::shared_ptr<sys::ServiceDirectory> runner_services,
    fidl::InterfaceHandle<fuchsia::sys::ServiceProvider>
        parent_environment_service_provider,
    fidl::InterfaceHandle<fuchsia::ui::views::ViewRefFocused> vrf,
    fidl::InterfaceHandle<fuchsia::ui::views::Focuser> focuser,
    fidl::InterfaceRequest<fuchsia::ui::input3::KeyboardListener>
        keyboard_listener,
    OnEnableWireframe wireframe_enabled_callback,
    OnCreateView on_create_view_callback,
    OnUpdateView on_update_view_callback,
    OnDestroyView on_destroy_view_callback,
    OnCreateSurface on_create_surface_callback,
    OnSemanticsNodeUpdate on_semantics_node_update_callback,
    OnRequestAnnounce on_request_announce_callback,
    OnShaderWarmup on_shader_warmup,
    std::shared_ptr<flutter::ExternalViewEmbedder> view_embedder,
    AwaitVsyncCallback await_vsync_callback,
    AwaitVsyncForSecondaryCallbackCallback
        await_vsync_for_secondary_callback_callback)
    : PlatformView(delegate,
                   std::move(debug_label),
                   std::move(view_ref),
                   std::move(task_runners),
                   std::move(runner_services),
                   std::move(parent_environment_service_provider),
                   std::move(vrf),
                   std::move(focuser),
                   std::move(keyboard_listener),
                   std::move(wireframe_enabled_callback),
                   std::move(on_create_view_callback),
                   std::move(on_update_view_callback),
                   std::move(on_destroy_view_callback),
                   std::move(on_create_surface_callback),
                   std::move(on_semantics_node_update_callback),
                   std::move(on_request_announce_callback),
                   std::move(on_shader_warmup),
                   std::move(view_embedder),
                   std::move(await_vsync_callback),
                   std::move(await_vsync_for_secondary_callback_callback)) {}

FlatlandPlatformView::~FlatlandPlatformView() = default;

}  // namespace flutter_runner
