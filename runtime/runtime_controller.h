// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_RUNTIME_CONTROLLER_H_
#define FLUTTER_RUNTIME_RUNTIME_CONTROLLER_H_

#include <memory>

#include "flutter/common/task_runners.h"
#include "flutter/flow/layers/layer_tree.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/pointer_data_packet.h"
#include "flutter/lib/ui/window/window.h"
#include "flutter/runtime/dart_vm.h"
#include "lib/fxl/macros.h"

namespace blink {
class Scene;
class RuntimeDelegate;
class View;
class Window;

class RuntimeController final : public WindowClient {
 public:
  RuntimeController(RuntimeDelegate& client,
                    const DartVM* vm,
                    TaskRunners task_runners,
                    fml::WeakPtr<GrContext> resource_context,
                    fxl::RefPtr<flow::SkiaUnrefQueue> unref_queue);

  ~RuntimeController();

  bool SetViewportMetrics(const ViewportMetrics& metrics);

  bool SetLocale(const std::string& language_code,
                 const std::string& country_code);

  bool SetUserSettingsData(const std::string& data);

  bool SetSemanticsEnabled(bool enabled);

  bool BeginFrame(fxl::TimePoint frame_time);

  bool NotifyIdle(int64_t deadline);

  bool DispatchPlatformMessage(fxl::RefPtr<PlatformMessage> message);

  bool DispatchPointerDataPacket(const PointerDataPacket& packet);

  bool DispatchSemanticsAction(int32_t id,
                               SemanticsAction action,
                               std::vector<uint8_t> args);

  Dart_Port GetMainPort();

  std::string GetIsolateName();

  bool HasLivePorts();

  tonic::DartErrorHandleType GetLastError();

  fxl::WeakPtr<DartIsolate> GetRootIsolate();

 private:
  RuntimeDelegate& client_;
  fxl::WeakPtr<DartIsolate> root_isolate_;

  ViewportMetrics viewport_metrics_;
  std::string language_code_;
  std::string country_code_;
  std::string user_settings_data_ = "{}";
  bool semantics_enabled_ = false;

  Window* GetWindowIfAvailable();

  bool FlushRuntimeStateToIsolate();

  // |blink::WindowClient|
  std::string DefaultRouteName() override;

  // |blink::WindowClient|
  void ScheduleFrame() override;

  // |blink::WindowClient|
  void Render(Scene* scene) override;

  // |blink::WindowClient|
  void UpdateSemantics(SemanticsUpdate* update) override;

  // |blink::WindowClient|
  void HandlePlatformMessage(fxl::RefPtr<PlatformMessage> message) override;

  FXL_DISALLOW_COPY_AND_ASSIGN(RuntimeController);
};

}  // namespace blink

#endif  // FLUTTER_RUNTIME_RUNTIME_CONTROLLER_H_
