// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSDstyle license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_PLATFORM_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_PLATFORM_HANDLER_H_

#include <fuchsia/ui/input/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_ptr.h>
#include <lib/sys/cpp/service_directory.h>

#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/task_runner.h"
#include "flutter/shell/platform/fuchsia/flutter/accessibility_bridge.h"

namespace flutter_runner {

// The embedder component that is responsible for listening to and handling
// events from Scenic and related services.  It is sole owner and user of the
// |SessionListener|.
class ScenicPlatformHandler final
    : public fuchsia::ui::input::InputMethodEditorClient,
      public fuchsia::ui::scenic::SessionListener,
      public AccessibilityBridge::Delegate {
 public:
  ScenicPlatformHandler(
      std::shared_ptr<sys::ServiceDirectory> runner_services,
      fml::RefPtr<fml::TaskRunner> task_runner,
      fidl::InterfaceRequest<fuchsia::ui::scenic::SessionListener>
          session_listener_request,
      fuchsia::ui::views::ViewRef view_ref,
      fml::closure error_callback);
  ~ScenicPlatformHandler();

 private:
  // |fuchsia::ui::input::InputMethodEditorClient|
  void DidUpdateState(
      fuchsia::ui::input::TextInputState state,
      ::std::unique_ptr<fuchsia::ui::input::InputEvent> event) override {}

  // |fuchsia::ui::input::InputMethodEditorClient|
  void OnAction(fuchsia::ui::input::InputMethodAction action) override {}

  // |fuchsia::ui::scenic::SessionListener|
  void OnScenicError(std::string error) override;

  // |fuchsia::ui::scenic::SessionListener|
  void OnScenicEvent(std::vector<fuchsia::ui::scenic::Event> events) override;

  // |AccessibilityBridge::Delegate|
  void SetSemanticsEnabled(bool enabled) override;

  AccessibilityBridge accessibility_bridge_;

  // The last state of the text input as reported by the IME or initialized by
  // Flutter. We set it to null if Flutter doesn't want any input, since then
  // there is no text input state at all.
  // std::optional<fuchsia::ui::input::TextInputState> last_text_state_;

  fidl::Binding<fuchsia::ui::input::InputMethodEditorClient> ime_client_;
  fidl::Binding<fuchsia::ui::scenic::SessionListener> session_listener_;
  fuchsia::ui::input::InputMethodEditorPtr ime_;
  fuchsia::ui::input::ImeServicePtr text_sync_service_;

  fml::RefPtr<fml::TaskRunner> task_runner_;

  // std::set<int> down_pointers_;
  // std::map<
  //     std::string /* channel */,
  //     fit::function<void(
  //         fml::RefPtr<flutter::PlatformMessage> /* message */)> /* handler
  //         */>
  //     platform_message_handlers_;

  // int current_text_input_client_ = 0;

  FML_DISALLOW_COPY_AND_ASSIGN(ScenicPlatformHandler);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_PLATFORM_HANDLER_H_
