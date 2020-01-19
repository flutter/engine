// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_H_

#include <lib/async/dispatcher.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/vfs/cpp/service.h>

#include <functional>
#include <memory>
#include <string>

#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter_runner {

class Renderer {
 public:
  using WindowMetricsCallback =
      std::function<void(const FlutterWindowMetricsEvent*)>;
  using PointerEventCallback =
      std::function<void(const FlutterPointerEvent*, size_t)>;
  using PlatformMessageCallback =
      std::function<void(const FlutterPlatformMessage*)>;
  using PlatformMessageResponseCallback =
      std::function<void(const FlutterPlatformMessageResponseHandle*)>;
  using UpdateSemanticsEnabledCallback = std::function<void(bool)>;
  using UpdateAccessibilityFeaturesCallback =
      std::function<void(FlutterAccessibilityFeature)>;
  using SemanticsActionCallback = std::function<
      void(uint64_t, FlutterSemanticsAction, const uint8_t*, size_t)>;
  using GetCurrentTimeCallback = std::function<uint64_t()>;
  using VsyncCallback = std::function<void(intptr_t, uint64_t, uint64_t)>;
  using ErrorCallback = std::function<void()>;
  struct DispatchTable {
    WindowMetricsCallback window_metrics_callback;
    PointerEventCallback pointer_event_callback;
    PlatformMessageCallback platform_message_callback;
    PlatformMessageResponseCallback platform_message_response_callback;
    UpdateSemanticsEnabledCallback update_semantics_enabled_callback;
    UpdateAccessibilityFeaturesCallback update_accessibility_features_callback;
    SemanticsActionCallback semantics_action_callback;
    GetCurrentTimeCallback get_current_time_callback;
    VsyncCallback vsync_callback;
    ErrorCallback error_callback;
  };

  struct Context {
    DispatchTable dispatch_table;

    std::string debug_label;

    std::shared_ptr<sys::ServiceDirectory> incoming_services;

    async_dispatcher_t* input_dispatcher;
    async_dispatcher_t* raster_dispatcher;
  };

  using FactoryCallback =
      std::function<std::unique_ptr<Renderer>(Context renderer_context)>;
  using BindServiceCallback =
      std::function<void(const std::string& name,
                         std::unique_ptr<vfs::Service> service)>;

  virtual ~Renderer() {}

  // This methods are invoked by the Flutter embedder upon construction; they
  // give the renderer a chance to expose any desired services and Dart hooks.
  virtual void BindServices(BindServiceCallback bind_service_callback) = 0;
  virtual void ConfigureCurrentIsolate() = 0;

  // These methods are invoked by the Flutter engine via the Embedder API; they
  // are called on the platform thread.
  virtual void PlatformMessageResponse(
      const FlutterPlatformMessage* message) = 0;
  virtual void UpdateSemanticsNode(const FlutterSemanticsNode* node) = 0;
  virtual void UpdateSemanticsCustomAction(
      const FlutterSemanticsCustomAction* action) = 0;

  // These methods are invoked by the Flutter engine via the Embedder API; they
  // are called on the UI thread.
  virtual void AwaitVsync(intptr_t baton) = 0;

  // These methods are invoked by the Flutter engine via the Embedder API; they
  // are called on the raster thread.
  virtual bool CreateBackingStore(const FlutterBackingStoreConfig* layer_config,
                                  FlutterBackingStore* backing_store_out) = 0;
  virtual bool CollectBackingStore(
      const FlutterBackingStore* backing_store) = 0;
  virtual bool PresentLayers(const FlutterLayer** layers,
                             size_t layer_count) = 0;
  virtual bool IsBackingStoreAvailable(
      const FlutterBackingStore* backing_store) = 0;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_H_
