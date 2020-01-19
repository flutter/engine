// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_PLATFORM_BRIDGE_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_PLATFORM_BRIDGE_H_

#include <fuchsia/ui/gfx/cpp/fidl.h>
#include <fuchsia/ui/input/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/ui/scenic/cpp/id.h>

#include <optional>
#include <set>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_accessibility_bridge.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_session.h"

namespace flutter_runner {

// The embedder component that is responsible for listening to and handling
// events from Scenic and related services.
//
// This class is thread-hostile.  All methods must be called on the platform
// thread.
class ScenicPlatformBridge final
    : public fuchsia::ui::input::InputMethodEditorClient {
 public:
  using ChildViewConnectedCallback = std::function<void(scenic::ResourceId)>;
  using ChildViewDisconnectedCallback = std::function<void(scenic::ResourceId)>;
  using ChildViewStateChangedCallback =
      std::function<void(scenic::ResourceId, bool)>;
  using MetricsChangedCallback =
      std::function<void(const FlutterWindowMetricsEvent*)>;
  using ViewEnableWireframeCallback = std::function<void(bool)>;
  struct RenderDispatchTable {
    ChildViewConnectedCallback child_view_connected_callback;
    ChildViewDisconnectedCallback child_view_disconnected_callback;
    ChildViewStateChangedCallback child_view_state_changed_callback;
    MetricsChangedCallback metrics_changed_callback;
    ViewEnableWireframeCallback view_enable_wireframe_callback;
  };

  ScenicPlatformBridge(Renderer::DispatchTable dispatch_table,
                       RenderDispatchTable render_dispatch_table,
                       fuchsia::ui::views::ViewRef view_ref,
                       std::shared_ptr<sys::ServiceDirectory> runner_services);
  ScenicPlatformBridge(const ScenicPlatformBridge&) = delete;
  ScenicPlatformBridge(ScenicPlatformBridge&&) = delete;
  ~ScenicPlatformBridge() = default;

  ScenicPlatformBridge& operator=(const ScenicPlatformBridge&) = delete;
  ScenicPlatformBridge& operator=(ScenicPlatformBridge&&) = delete;

  // Event handler invoked by the SessionListener.  Called on the platform
  // thread.
  void OnScenicEvent(std::vector<fuchsia::ui::scenic::Event> events);

  // Methods invoked by the |EmbedderPlatformView|.  Called on the platform
  // thread.
  void PlatformMessageResponse(const FlutterPlatformMessage* message);
  void UpdateSemanticsNode(const FlutterSemanticsNode* node);
  void UpdateSemanticsCustomAction(const FlutterSemanticsCustomAction* action);

 private:
  // |fuchsia::ui::input::InputMethodEditorClient|
  void DidUpdateState(
      fuchsia::ui::input::TextInputState state,
      std::unique_ptr<fuchsia::ui::input::InputEvent> event) override;
  void OnAction(fuchsia::ui::input::InputMethodAction action) override;

  // Scenic event handling.
  bool OnHandlePointerEvent(const fuchsia::ui::input::PointerEvent& pointer);
  bool OnHandleKeyboardEvent(const fuchsia::ui::input::KeyboardEvent& keyboard);
  bool OnHandleFocusEvent(const fuchsia::ui::input::FocusEvent& focus);

  // Platform message handling.
  void RegisterPlatformMessageHandlers();
  bool HandleAccessibilityChannelPlatformMessage(
      const FlutterPlatformMessage* message);
  bool HandleFlutterPlatformChannelPlatformMessage(
      const FlutterPlatformMessage* message);
  bool HandleFlutterTextInputChannelPlatformMessage(
      const FlutterPlatformMessage* message);
  bool HandleFlutterPlatformViewsChannelPlatformMessage(
      const FlutterPlatformMessage* message);

  // IME handling.
  void ActivateIme();
  void DeactivateIme();

  Renderer::DispatchTable dispatch_table_;
  RenderDispatchTable render_dispatch_table_;

  ScenicAccessibilityBridge accessibility_bridge_;

  // Pixel ratio is stored in z.
  fuchsia::ui::gfx::vec3 logical_metrics_ = {0.0f, 0.0f, 1.0f};

  fidl::Binding<fuchsia::ui::input::InputMethodEditorClient> ime_client_;
  fuchsia::ui::input::InputMethodEditorPtr ime_;
  fuchsia::ui::input::ImeServicePtr text_sync_service_;

  std::map<const char* /* channel */,
           std::function<bool(
               const FlutterPlatformMessage* /* message */)> /* handler */>
      platform_message_handlers_;

  std::set<int> down_pointers_;

  // The last state of the text input as reported by the IME or initialized by
  // Flutter. We set it to null if Flutter doesn't want any input, since then
  // there is no text input state at all.
  std::optional<fuchsia::ui::input::TextInputState> last_text_state_;
  int current_text_input_client_ = 0;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_PLATFORM_BRIDGE_H_
