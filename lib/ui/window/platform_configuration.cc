// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/platform_configuration.h"

#include <cstring>

#include "flutter/lib/ui/compositing/scene.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_message_response_dart.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/lib/ui/window/window.h"
#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/dart_microtask_queue.h"
#include "third_party/tonic/logging/dart_invoke.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {
namespace {

Dart_Handle ToByteData(const fml::Mapping& buffer) {
  return tonic::DartByteData::Create(buffer.GetMapping(), buffer.GetSize());
}

}  // namespace

PlatformConfigurationClient::~PlatformConfigurationClient() {}

PlatformConfiguration::PlatformConfiguration(
    PlatformConfigurationClient* client)
    : client_(client) {}

PlatformConfiguration::~PlatformConfiguration() {}

void PlatformConfiguration::DidCreateIsolate() {
  Dart_Handle library = Dart_LookupLibrary(tonic::ToDart("dart:ui"));

  on_error_.Set(tonic::DartState::Current(),
                Dart_GetField(library, tonic::ToDart("_onError")));
  update_locales_.Set(tonic::DartState::Current(),
                      Dart_GetField(library, tonic::ToDart("_updateLocales")));
  update_user_settings_data_.Set(
      tonic::DartState::Current(),
      Dart_GetField(library, tonic::ToDart("_updateUserSettingsData")));
  update_lifecycle_state_.Set(
      tonic::DartState::Current(),
      Dart_GetField(library, tonic::ToDart("_updateLifecycleState")));
  update_semantics_enabled_.Set(
      tonic::DartState::Current(),
      Dart_GetField(library, tonic::ToDart("_updateSemanticsEnabled")));
  update_accessibility_features_.Set(
      tonic::DartState::Current(),
      Dart_GetField(library, tonic::ToDart("_updateAccessibilityFeatures")));
  dispatch_platform_message_.Set(
      tonic::DartState::Current(),
      Dart_GetField(library, tonic::ToDart("_dispatchPlatformMessage")));
  dispatch_semantics_action_.Set(
      tonic::DartState::Current(),
      Dart_GetField(library, tonic::ToDart("_dispatchSemanticsAction")));
  begin_frame_.Set(tonic::DartState::Current(),
                   Dart_GetField(library, tonic::ToDart("_beginFrame")));
  draw_frame_.Set(tonic::DartState::Current(),
                  Dart_GetField(library, tonic::ToDart("_drawFrame")));
  report_timings_.Set(tonic::DartState::Current(),
                      Dart_GetField(library, tonic::ToDart("_reportTimings")));
  windows_.insert(std::make_pair(
      0, std::make_unique<Window>(0, ViewportMetrics{1.0, 0.0, 0.0, -1})));
}

void PlatformConfiguration::UpdateLocales(
    const std::vector<std::string>& locales) {
  std::shared_ptr<tonic::DartState> dart_state =
      update_locales_.dart_state().lock();
  if (!dart_state) {
    return;
  }

  tonic::DartState::Scope scope(dart_state);
  tonic::CheckAndHandleError(
      tonic::DartInvoke(update_locales_.Get(),
                        {
                            tonic::ToDart<std::vector<std::string>>(locales),
                        }));
}

void PlatformConfiguration::UpdateUserSettingsData(const std::string& data) {
  std::shared_ptr<tonic::DartState> dart_state =
      update_user_settings_data_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);

  tonic::CheckAndHandleError(tonic::DartInvoke(update_user_settings_data_.Get(),
                                               {
                                                   tonic::StdStringToDart(data),
                                               }));
}

void PlatformConfiguration::UpdateLifecycleState(const std::string& data) {
  std::shared_ptr<tonic::DartState> dart_state =
      update_lifecycle_state_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  tonic::CheckAndHandleError(tonic::DartInvoke(update_lifecycle_state_.Get(),
                                               {
                                                   tonic::StdStringToDart(data),
                                               }));
}

void PlatformConfiguration::UpdateSemanticsEnabled(bool enabled) {
  std::shared_ptr<tonic::DartState> dart_state =
      update_semantics_enabled_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  UIDartState::ThrowIfUIOperationsProhibited();

  tonic::CheckAndHandleError(tonic::DartInvoke(update_semantics_enabled_.Get(),
                                               {tonic::ToDart(enabled)}));
}

void PlatformConfiguration::UpdateAccessibilityFeatures(int32_t values) {
  std::shared_ptr<tonic::DartState> dart_state =
      update_accessibility_features_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);

  tonic::CheckAndHandleError(tonic::DartInvoke(
      update_accessibility_features_.Get(), {tonic::ToDart(values)}));
}

void PlatformConfiguration::DispatchPlatformMessage(
    std::unique_ptr<PlatformMessage> message) {
  std::shared_ptr<tonic::DartState> dart_state =
      dispatch_platform_message_.dart_state().lock();
  if (!dart_state) {
    FML_DLOG(WARNING)
        << "Dropping platform message for lack of DartState on channel: "
        << message->channel();
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  Dart_Handle data_handle =
      (message->hasData()) ? ToByteData(message->data()) : Dart_Null();
  if (Dart_IsError(data_handle)) {
    FML_DLOG(WARNING)
        << "Dropping platform message because of a Dart error on channel: "
        << message->channel();
    return;
  }

  int response_id = 0;
  if (auto response = message->response()) {
    response_id = next_response_id_++;
    pending_responses_[response_id] = response;
  }

  tonic::CheckAndHandleError(
      tonic::DartInvoke(dispatch_platform_message_.Get(),
                        {tonic::ToDart(message->channel()), data_handle,
                         tonic::ToDart(response_id)}));
}

void PlatformConfiguration::DispatchSemanticsAction(int32_t id,
                                                    SemanticsAction action,
                                                    fml::MallocMapping args) {
  std::shared_ptr<tonic::DartState> dart_state =
      dispatch_semantics_action_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);

  Dart_Handle args_handle =
      (args.GetSize() <= 0) ? Dart_Null() : ToByteData(args);

  if (Dart_IsError(args_handle)) {
    return;
  }

  tonic::CheckAndHandleError(tonic::DartInvoke(
      dispatch_semantics_action_.Get(),
      {tonic::ToDart(id), tonic::ToDart(static_cast<int32_t>(action)),
       args_handle}));
}

void PlatformConfiguration::BeginFrame(fml::TimePoint frameTime,
                                       uint64_t frame_number) {
  std::shared_ptr<tonic::DartState> dart_state =
      begin_frame_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  TRACE_EVENT0("flutter", "PerformanceMode_Latency");
  Dart_PerformanceMode previous =
      Dart_SetPerformanceMode(Dart_PerformanceMode_Latency);

  int64_t microseconds = (frameTime - fml::TimePoint()).ToMicroseconds();

  tonic::CheckAndHandleError(
      tonic::DartInvoke(begin_frame_.Get(), {
                                                Dart_NewInteger(microseconds),
                                                Dart_NewInteger(frame_number),
                                            }));

  UIDartState::Current()->FlushMicrotasksNow();

  tonic::CheckAndHandleError(tonic::DartInvokeVoid(draw_frame_.Get()));

  Dart_SetPerformanceMode(previous);
}

void PlatformConfiguration::ReportTimings(std::vector<int64_t> timings) {
  std::shared_ptr<tonic::DartState> dart_state =
      report_timings_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);

  Dart_Handle data_handle =
      Dart_NewTypedData(Dart_TypedData_kInt64, timings.size());

  Dart_TypedData_Type type;
  void* data = nullptr;
  intptr_t num_acquired = 0;
  FML_CHECK(!Dart_IsError(
      Dart_TypedDataAcquireData(data_handle, &type, &data, &num_acquired)));
  FML_DCHECK(num_acquired == static_cast<int>(timings.size()));

  memcpy(data, timings.data(), sizeof(int64_t) * timings.size());
  FML_CHECK(Dart_TypedDataReleaseData(data_handle));

  tonic::CheckAndHandleError(
      tonic::DartInvoke(report_timings_.Get(), {
                                                   data_handle,
                                               }));
}

void PlatformConfiguration::CompletePlatformMessageEmptyResponse(
    int response_id) {
  if (!response_id) {
    return;
  }
  auto it = pending_responses_.find(response_id);
  if (it == pending_responses_.end()) {
    return;
  }
  auto response = std::move(it->second);
  pending_responses_.erase(it);
  response->CompleteEmpty();
}

void PlatformConfiguration::CompletePlatformMessageResponse(
    int response_id,
    std::vector<uint8_t> data) {
  if (!response_id) {
    return;
  }
  auto it = pending_responses_.find(response_id);
  if (it == pending_responses_.end()) {
    return;
  }
  auto response = std::move(it->second);
  pending_responses_.erase(it);
  response->Complete(std::make_unique<fml::DataMapping>(std::move(data)));
}

void PlatformConfigurationNativeApi::Render(Scene* scene) {
  UIDartState::ThrowIfUIOperationsProhibited();
  UIDartState::Current()->platform_configuration()->client()->Render(scene);
}

void PlatformConfigurationNativeApi::SetNeedsReportTimings(bool value) {
  UIDartState::ThrowIfUIOperationsProhibited();
  UIDartState::Current()
      ->platform_configuration()
      ->client()
      ->SetNeedsReportTimings(value);
}

Dart_Handle PlatformConfigurationNativeApi::SendPlatformMessage(
    const std::string& name,
    Dart_Handle callback,
    Dart_Handle data_handle) {
  UIDartState* dart_state = UIDartState::Current();

  if (!dart_state->platform_configuration()) {
    return tonic::ToDart(
        "Platform messages can only be sent from the main isolate");
  }

  fml::RefPtr<PlatformMessageResponse> response;
  if (!Dart_IsNull(callback)) {
    response = fml::MakeRefCounted<PlatformMessageResponseDart>(
        tonic::DartPersistentValue(dart_state, callback),
        dart_state->GetTaskRunners().GetUITaskRunner(), name);
  }
  if (Dart_IsNull(data_handle)) {
    dart_state->platform_configuration()->client()->HandlePlatformMessage(
        std::make_unique<PlatformMessage>(name, response));
  } else {
    tonic::DartByteData data(data_handle);
    const uint8_t* buffer = static_cast<const uint8_t*>(data.data());
    dart_state->platform_configuration()->client()->HandlePlatformMessage(
        std::make_unique<PlatformMessage>(
            name, fml::MallocMapping::Copy(buffer, data.length_in_bytes()),
            response));
  }

  return Dart_Null();
}

void PlatformConfigurationNativeApi::RespondToPlatformMessage(
    int response_id,
    const tonic::DartByteData& data) {
  if (Dart_IsNull(data.dart_handle())) {
    UIDartState::Current()
        ->platform_configuration()
        ->CompletePlatformMessageEmptyResponse(response_id);
  } else {
    // TODO(engine): Avoid this copy.
    const uint8_t* buffer = static_cast<const uint8_t*>(data.data());
    UIDartState::Current()
        ->platform_configuration()
        ->CompletePlatformMessageResponse(
            response_id,
            std::vector<uint8_t>(buffer, buffer + data.length_in_bytes()));
  }
}

void PlatformConfigurationNativeApi::SetIsolateDebugName(
    const std::string name) {
  UIDartState::ThrowIfUIOperationsProhibited();
  UIDartState::Current()->SetDebugName(name);
}

Dart_Handle PlatformConfigurationNativeApi::GetPersistentIsolateData() {
  UIDartState::ThrowIfUIOperationsProhibited();

  auto persistent_isolate_data = UIDartState::Current()
                                     ->platform_configuration()
                                     ->client()
                                     ->GetPersistentIsolateData();

  if (!persistent_isolate_data) {
    return Dart_Null();
  }

  return tonic::DartByteData::Create(persistent_isolate_data->GetMapping(),
                                     persistent_isolate_data->GetSize());
}

void PlatformConfigurationNativeApi::ScheduleFrame() {
  UIDartState::ThrowIfUIOperationsProhibited();
  UIDartState::Current()->platform_configuration()->client()->ScheduleFrame();
}

void PlatformConfigurationNativeApi::UpdateSemantics(SemanticsUpdate* update) {
  UIDartState::ThrowIfUIOperationsProhibited();
  UIDartState::Current()->platform_configuration()->client()->UpdateSemantics(
      update);
}

Dart_Handle PlatformConfigurationNativeApi::ComputePlatformResolvedLocale(
    Dart_Handle supportedLocalesHandle) {
  UIDartState::ThrowIfUIOperationsProhibited();
  std::vector<std::string> supportedLocales =
      tonic::DartConverter<std::vector<std::string>>::FromDart(
          supportedLocalesHandle);

  std::vector<std::string> results =
      *UIDartState::Current()
           ->platform_configuration()
           ->client()
           ->ComputePlatformResolvedLocale(supportedLocales);

  return tonic::DartConverter<std::vector<std::string>>::ToDart(results);
}

std::string PlatformConfigurationNativeApi::DefaultRouteName() {
  UIDartState::ThrowIfUIOperationsProhibited();
  return UIDartState::Current()
      ->platform_configuration()
      ->client()
      ->DefaultRouteName();
}

}  // namespace flutter
