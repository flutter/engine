// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_SHELL_H_
#define SHELL_COMMON_SHELL_H_

#include <functional>

#include "flutter/common/settings.h"
#include "flutter/common/task_runners.h"
#include "flutter/flow/texture.h"
#include "flutter/fml/memory/thread_checker.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/thread.h"
#include "flutter/lib/ui/semantics/semantics_node.h"
#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/shell/common/animator.h"
#include "flutter/shell/common/engine.h"
#include "flutter/shell/common/io_manager.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/common/surface.h"
#include "lib/fxl/functional/closure.h"
#include "lib/fxl/macros.h"
#include "lib/fxl/memory/ref_ptr.h"
#include "lib/fxl/memory/weak_ptr.h"
#include "lib/fxl/synchronization/mutex.h"
#include "lib/fxl/synchronization/thread_annotations.h"
#include "lib/fxl/synchronization/thread_checker.h"
#include "lib/fxl/synchronization/waitable_event.h"

namespace shell {

class Shell final : public PlatformView::Delegate,
                    public Animator::Delegate,
                    public Engine::Delegate {
 public:
  template <class T>
  using CreateCallback = std::function<std::unique_ptr<T>(Shell&)>;
  static std::unique_ptr<Shell> Create(
      blink::TaskRunners task_runners,
      blink::Settings,
      CreateCallback<PlatformView> on_create_platform_view,
      CreateCallback<Rasterizer> on_create_rasterizer);

  ~Shell();

  const blink::Settings& GetSettings() const;

  const blink::TaskRunners& GetTaskRunners() const;

  // Attempt to run a script inside a flutter view indicated by |view_id|.
  // Will set |view_existed| to true if the view was found and false otherwise.
  void RunInPlatformView(uintptr_t view_id,
                         const char* main_script,
                         const char* packages_file,
                         const char* asset_directory,
                         bool* view_existed,
                         int64_t* dart_isolate_id,
                         std::string* isolate_name);

  fml::WeakPtr<Rasterizer> GetRasterizer();

  fml::WeakPtr<Engine> GetEngine();

  fml::WeakPtr<PlatformView> GetPlatformView();

  const blink::DartVM& GetDartVM() const;

  bool IsSetup() const;

 private:
  const blink::TaskRunners task_runners_;
  const blink::Settings settings_;
  fxl::RefPtr<blink::DartVM> vm_;
  std::unique_ptr<PlatformView> platform_view_;  // on platform task runner
  std::unique_ptr<Engine> engine_;               // on UI task runner
  std::unique_ptr<Rasterizer> rasterizer_;       // on GPU task runner
  std::unique_ptr<IOManager> io_manager_;        // on IO task runner
  bool is_setup_ = false;

  Shell(blink::TaskRunners task_runners, blink::Settings settings);

  static std::unique_ptr<Shell> CreateShellOnPlatformThread(
      blink::TaskRunners task_runners,
      blink::Settings settings,
      Shell::CreateCallback<PlatformView> on_create_platform_view,
      Shell::CreateCallback<Rasterizer> on_create_rasterizer);

  bool Setup(std::unique_ptr<PlatformView> platform_view,
             std::unique_ptr<Engine> engine,
             std::unique_ptr<Rasterizer> rasterizer,
             std::unique_ptr<IOManager> io_manager);

  // |shell::PlatformView::Delegate|
  void OnPlatformViewCreated(const PlatformView& view,
                             std::unique_ptr<Surface> surface) override;

  // |shell::PlatformView::Delegate|
  void OnPlatformViewDestroyed(const PlatformView& view) override;

  // |shell::PlatformView::Delegate|
  void OnPlatformViewDispatchPlatformMessage(
      const PlatformView& view,
      fxl::RefPtr<blink::PlatformMessage> message) override;

  // |shell::PlatformView::Delegate|
  void OnPlatformViewDispatchSemanticsAction(
      const PlatformView& view,
      int32_t id,
      blink::SemanticsAction action,
      std::vector<uint8_t> args) override;

  // |shell::PlatformView::Delegate|
  void OnPlatformViewSetSemanticsEnabled(const PlatformView& view,
                                         bool enabled) override;

  // |shell::PlatformView::Delegate|
  void OnPlatformViewRegisterTexture(
      const PlatformView& view,
      std::shared_ptr<flow::Texture> texture) override;

  // |shell::PlatformView::Delegate|
  void OnPlatformViewUnregisterTexture(const PlatformView& view,
                                       int64_t texture_id) override;

  // |shell::PlatformView::Delegate|
  void OnPlatformViewMarkTextureFrameAvailable(const PlatformView& view,
                                               int64_t texture_id) override;

  // |shell::Animator::Delegate|
  void OnAnimatorBeginFrame(const Animator& animator,
                            fxl::TimePoint frame_time) override;

  // |shell::Animator::Delegate|
  void OnAnimatorNotifyIdle(const Animator& animator,
                            int64_t deadline) override;

  // |shell::Animator::Delegate|
  void OnAnimatorDraw(
      const Animator& animator,
      fxl::RefPtr<flutter::Pipeline<flow::LayerTree>> pipeline) override;

  // |shell::Animator::Delegate|
  void OnAnimatorDrawLastLayerTree(const Animator& animator) override;

  // |shell::Engine::Delegate|
  void OnEngineUpdateSemantics(
      const Engine& engine,
      std::vector<blink::SemanticsNode> update) override;

  // |shell::Engine::Delegate|
  void OnEngineHandlePlatformMessage(
      const Engine& engine,
      fxl::RefPtr<blink::PlatformMessage> message) override;

  FXL_DISALLOW_COPY_AND_ASSIGN(Shell);
};

}  // namespace shell

#endif  // SHELL_COMMON_SHELL_H_
