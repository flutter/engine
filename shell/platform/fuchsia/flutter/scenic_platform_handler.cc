// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/scenic_platform_handler.h"

#include <lib/ui/scenic/cpp/commands.h>
#include <zircon/types.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/platform/fuchsia/flutter/vsync_recorder.h"
#include "flutter/shell/platform/fuchsia/flutter/vsync_waiter.h"

namespace flutter_runner {
namespace {

template <class T>
void SetInterfaceErrorHandler(fidl::Binding<T>& binding,
                              fml::closure error_callback) {
  binding.set_error_handler([error_callback](zx_status_t status) {
    FML_LOG(ERROR) << "Interface error (binding) << " << status
                   << " on: ";  // << T::Name_;
    error_callback();
  });
}

template <class T>
void SetInterfaceErrorHandler(fidl::InterfacePtr<T>& interface,
                              fml::closure error_callback) {
  interface.set_error_handler([error_callback](zx_status_t status) {
    FML_LOG(ERROR) << "Interface error " << status << " on: ";  // << T::Name_;
    error_callback();
  });
}

}  // end namespace

ScenicPlatformHandler::ScenicPlatformHandler(
    std::shared_ptr<sys::ServiceDirectory> runner_services,
    fml::RefPtr<fml::TaskRunner> task_runner,
    fidl::InterfaceRequest<fuchsia::ui::scenic::SessionListener>
        session_listener_request,
    fuchsia::ui::views::ViewRef view_ref,
    fml::closure error_callback)
    : accessibility_bridge_(*this, runner_services, std::move(view_ref)),
      ime_client_(this),
      session_listener_(this, std::move(session_listener_request)),
      task_runner_(std::move(task_runner)) {
  SetInterfaceErrorHandler(ime_client_, error_callback);
  SetInterfaceErrorHandler(session_listener_, error_callback);
  SetInterfaceErrorHandler(ime_, error_callback);
  SetInterfaceErrorHandler(text_sync_service_, error_callback);

  /*parent_environment_services_.get()->ConnectToService(
      fuchsia::ui::input::ImeService::Name_,
      text_sync_service_.NewRequest().TakeChannel());*/
}

ScenicPlatformHandler::~ScenicPlatformHandler() = default;

void ScenicPlatformHandler::SetSemanticsEnabled(bool enabled) {
  // if (enabled) {
  //   platform_view_->SetAccessibilityFeatures(static_cast<int32_t>(
  //       flutter::AccessibilityFeatureFlag::kAccessibleNavigation));
  // } else {
  //   platform_view_->SetAccessibilityFeatures(0);
  // }
}

void ScenicPlatformHandler::OnScenicError(std::string error) {}

void ScenicPlatformHandler::OnScenicEvent(
    std::vector<fuchsia::ui::scenic::Event> events) {}

}  // namespace flutter_runner
