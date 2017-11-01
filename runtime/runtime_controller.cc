// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/runtime_controller.h"

#include "flutter/fml/message_loop.h"
#include "flutter/glue/trace_event.h"
#include "flutter/lib/ui/compositing/scene.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/window.h"
#include "flutter/runtime/runtime_delegate.h"
#include "lib/tonic/dart_message_handler.h"

namespace blink {

RuntimeController::RuntimeController(
    RuntimeDelegate& client,
    const DartVM* vm,
    TaskRunners task_runners,
    fml::WeakPtr<GrContext> resource_context,
    fxl::RefPtr<flow::SkiaUnrefQueue> unref_queue)
    : client_(client),
      root_isolate_(
          DartIsolate::CreateRootIsolate(vm,
                                         std::move(task_runners),
                                         std::make_unique<Window>(this),
                                         std::move(resource_context),
                                         std::move(unref_queue))) {
  if (auto window = GetWindowIfAvailable()) {
    tonic::DartState::Scope scope(root_isolate_.get());
    window->DidCreateIsolate();
    client_.DidCreateMainIsolate();
    if (!FlushRuntimeStateToIsolate()) {
      FXL_DLOG(ERROR) << "Could not setup intial isolate state.";
    }
  }
  FXL_DCHECK(Dart_CurrentIsolate() == nullptr);
}

RuntimeController::~RuntimeController() {
  FXL_DCHECK(Dart_CurrentIsolate() == nullptr);
  if (root_isolate_) {
    auto result = root_isolate_->Shutdown();
    if (!result) {
      FXL_DLOG(ERROR) << "Could not shutdown the root isolate.";
    }
    root_isolate_ = {};
  }
}

bool RuntimeController::FlushRuntimeStateToIsolate() {
  return SetViewportMetrics(viewport_metrics_) &&
         SetLocale(language_code_, country_code_) &&
         SetSemanticsEnabled(semantics_enabled_);
}

bool RuntimeController::SetViewportMetrics(const ViewportMetrics& metrics) {
  viewport_metrics_ = metrics;

  if (auto window = GetWindowIfAvailable()) {
    window->UpdateWindowMetrics(metrics);
    return true;
  }
  return false;
}

bool RuntimeController::SetLocale(const std::string& language_code,
                                  const std::string& country_code) {
  language_code_ = language_code;
  country_code_ = country_code;

  if (auto window = GetWindowIfAvailable()) {
    window->UpdateLocale(language_code_, country_code_);
    return true;
  }

  return false;
}

bool RuntimeController::SetUserSettingsData(const std::string& data) {
  user_settings_data_ = data;

  if (auto window = GetWindowIfAvailable()) {
    window->UpdateUserSettingsData(user_settings_data_);
    return true;
  }

  return false;
}

bool RuntimeController::SetSemanticsEnabled(bool enabled) {
  semantics_enabled_ = enabled;

  if (auto window = GetWindowIfAvailable()) {
    window->UpdateSemanticsEnabled(semantics_enabled_);
    return true;
  }

  return false;
}

bool RuntimeController::BeginFrame(fxl::TimePoint frame_time) {
  if (auto window = GetWindowIfAvailable()) {
    window->BeginFrame(frame_time);
    return true;
  }
  return false;
}

bool RuntimeController::NotifyIdle(int64_t deadline) {
  if (!root_isolate_) {
    return false;
  }

  tonic::DartState::Scope scope(root_isolate_.get());
  Dart_NotifyIdle(deadline);
  return true;
}

bool RuntimeController::DispatchPlatformMessage(
    fxl::RefPtr<PlatformMessage> message) {
  if (auto window = GetWindowIfAvailable()) {
    TRACE_EVENT1("flutter", "RuntimeController::DispatchPlatformMessage",
                 "mode", "basic");
    window->DispatchPlatformMessage(std::move(message));
    return true;
  }
  return false;
}

bool RuntimeController::DispatchPointerDataPacket(
    const PointerDataPacket& packet) {
  if (auto window = GetWindowIfAvailable()) {
    TRACE_EVENT1("flutter", "RuntimeController::DispatchPointerDataPacket",
                 "mode", "basic");
    window->DispatchPointerDataPacket(packet);
    return true;
  }
  return false;
}

bool RuntimeController::DispatchSemanticsAction(int32_t id,
                                                SemanticsAction action,
                                                std::vector<uint8_t> args) {
  TRACE_EVENT1("flutter", "RuntimeController::DispatchSemanticsAction", "mode",
               "basic");
  if (auto window = GetWindowIfAvailable()) {
    window->DispatchSemanticsAction(id, action, std::move(args));
    return true;
  }
  return false;
}

Window* RuntimeController::GetWindowIfAvailable() {
  return root_isolate_ ? root_isolate_->window() : nullptr;
}

std::string RuntimeController::DefaultRouteName() {
  return client_.DefaultRouteName();
}

void RuntimeController::ScheduleFrame() {
  client_.ScheduleFrame();
}

void RuntimeController::Render(Scene* scene) {
  client_.Render(scene->takeLayerTree());
}

void RuntimeController::UpdateSemantics(SemanticsUpdate* update) {
  if (semantics_enabled_) {
    client_.UpdateSemantics(update->takeNodes());
  }
}

void RuntimeController::HandlePlatformMessage(
    fxl::RefPtr<PlatformMessage> message) {
  client_.HandlePlatformMessage(std::move(message));
}

Dart_Port RuntimeController::GetMainPort() {
  return root_isolate_ ? root_isolate_->main_port() : ILLEGAL_PORT;
}

std::string RuntimeController::GetIsolateName() {
  return root_isolate_ ? root_isolate_->debug_name() : "";
}

bool RuntimeController::HasLivePorts() {
  if (!root_isolate_) {
    return false;
  }
  tonic::DartState::Scope scope(root_isolate_.get());
  return Dart_HasLivePorts();
}

tonic::DartErrorHandleType RuntimeController::GetLastError() {
  return root_isolate_ ? root_isolate_->message_handler().isolate_last_error()
                       : tonic::kNoError;
}

fxl::WeakPtr<DartIsolate> RuntimeController::GetRootIsolate() {
  return root_isolate_;
}

}  // namespace blink
