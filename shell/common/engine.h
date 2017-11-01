// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_ENGINE_H_
#define SHELL_COMMON_ENGINE_H_

#include "flutter/assets/directory_asset_bundle.h"
#include "flutter/assets/zip_asset_store.h"
#include "flutter/common/task_runners.h"
#include "flutter/lib/ui/semantics/semantics_node.h"
#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/runtime/platform_impl.h"
#include "flutter/runtime/runtime_controller.h"
#include "flutter/runtime/runtime_delegate.h"
#include "flutter/shell/common/animator.h"
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/common/run_configuration.h"
#include "lib/fxl/macros.h"
#include "lib/fxl/memory/weak_ptr.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace shell {

class Engine final : public blink::RuntimeDelegate {
 public:
  class Delegate {
   public:
    virtual void OnEngineUpdateSemantics(
        const Engine& engine,
        std::vector<blink::SemanticsNode> update) = 0;

    virtual void OnEngineHandlePlatformMessage(
        const Engine& engine,
        fxl::RefPtr<blink::PlatformMessage> message) = 0;
  };

  Engine(Delegate& delegate,
         const blink::DartVM& vm,
         blink::TaskRunners task_runners,
         blink::Settings settings,
         std::unique_ptr<Animator> animator,
         fml::WeakPtr<GrContext> resource_context,
         fxl::RefPtr<flow::SkiaUnrefQueue> unref_queue);

  ~Engine() override;

  fml::WeakPtr<Engine> GetWeakPtr() const;

  FXL_WARN_UNUSED_RESULT
  bool Run(const RunConfiguration& configuration);

  void BeginFrame(fxl::TimePoint frame_time);

  void NotifyIdle(int64_t deadline);

  Dart_Port GetUIIsolateMainPort();

  std::string GetUIIsolateName();

  bool UIIsolateHasLivePorts();

  tonic::DartErrorHandleType GetUIIsolateLastError();

  tonic::DartErrorHandleType GetLoadScriptError();

  void OnOutputSurfaceCreated();

  void OnOutputSurfaceDestroyed();

  void SetViewportMetrics(const blink::ViewportMetrics& metrics);

  void DispatchPlatformMessage(fxl::RefPtr<blink::PlatformMessage> message);

  void DispatchPointerDataPacket(const blink::PointerDataPacket& packet);

  void DispatchSemanticsAction(int id,
                               blink::SemanticsAction action,
                               std::vector<uint8_t> args);

  void SetSemanticsEnabled(bool enabled);

  void ScheduleFrame(bool regenerate_layer_tree = true) override;

 private:
  // |blink::RuntimeDelegate|
  std::string DefaultRouteName() override;

  // |blink::RuntimeDelegate|
  void Render(std::unique_ptr<flow::LayerTree> layer_tree) override;

  // |blink::RuntimeDelegate|
  void UpdateSemantics(std::vector<blink::SemanticsNode> update) override;

  // |blink::RuntimeDelegate|
  void HandlePlatformMessage(
      fxl::RefPtr<blink::PlatformMessage> message) override;

  // |blink::RuntimeDelegate|
  void DidCreateMainIsolate() override;

  void StopAnimator();

  void StartAnimatorIfPossible();

  void ConfigureAssetBundle(const std::string& path);

  bool HandleLifecyclePlatformMessage(blink::PlatformMessage* message);

  bool HandleNavigationPlatformMessage(
      fxl::RefPtr<blink::PlatformMessage> message);

  bool HandleLocalizationPlatformMessage(blink::PlatformMessage* message);

  void HandleSettingsPlatformMessage(blink::PlatformMessage* message);

  void HandleAssetPlatformMessage(fxl::RefPtr<blink::PlatformMessage> message);

  bool GetAssetAsBuffer(const std::string& name, std::vector<uint8_t>* data);

  Engine::Delegate& delegate_;
  const blink::Settings settings_;
  std::unique_ptr<Animator> animator_;
  std::unique_ptr<blink::RuntimeController> runtime_controller_;
  std::unique_ptr<blink::PlatformImpl> legacy_sky_platform_;
  tonic::DartErrorHandleType load_script_error_;
  std::string initial_route_;
  blink::ViewportMetrics viewport_metrics_;
  // TODO(abarth): Unify these two behind a common interface.
  fxl::RefPtr<blink::ZipAssetStore> asset_store_;
  fxl::RefPtr<blink::DirectoryAssetBundle> directory_asset_bundle_;
  // TODO(eseidel): This should move into an AnimatorStateMachine.
  bool activity_running_;
  bool have_surface_;
  fml::WeakPtr<Engine> weak_prototype_;
  fml::WeakPtrFactory<Engine> weak_factory_;

  FXL_DISALLOW_COPY_AND_ASSIGN(Engine);
};

}  // namespace shell

#endif  // SHELL_COMMON_ENGINE_H_
