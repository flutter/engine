// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/shell.h"

#include <fcntl.h>
#include <memory>
#include <sstream>
#include <vector>

#include "flutter/fml/icu_util.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/trace_event.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/engine.h"
#include "flutter/shell/common/platform_view_service_protocol.h"
#include "flutter/shell/common/skia_event_tracer_impl.h"
#include "flutter/shell/common/switches.h"
#include "flutter/shell/common/vsync_waiter.h"
#include "lib/fxl/files/unique_fd.h"
#include "lib/fxl/functional/make_copyable.h"
#include "lib/fxl/logging.h"
#include "third_party/dart/runtime/include/dart_tools_api.h"
#include "third_party/skia/include/core/SkGraphics.h"

namespace shell {

std::unique_ptr<Shell> Shell::CreateShellOnPlatformThread(
    blink::TaskRunners task_runners,
    blink::Settings settings,
    Shell::CreateCallback<PlatformView> on_create_platform_view,
    Shell::CreateCallback<Rasterizer> on_create_rasterizer) {
  if (!task_runners.IsValid() ||
      fml::MessageLoop::GetCurrent().GetTaskRunner() !=
          task_runners.GetPlatformTaskRunner()) {
    return nullptr;
  }

  auto shell = std::unique_ptr<Shell>(new Shell(task_runners, settings));

  // Create the platform view on the platform thread (this thread).
  auto platform_view = on_create_platform_view(*shell.get());
  if (!platform_view || !platform_view->GetWeakPtr()) {
    return nullptr;
  }

  // Ask the platform view for the vsync waiter. This will be used by the engine
  // to create the animator.
  auto vsync_waiter = platform_view->CreateVSyncWaiter();
  if (!vsync_waiter) {
    return nullptr;
  }

  // Create the IO manager on the IO thread. The IO manager must be initialized
  // first because it has state that the other subsystems depend on. It must
  // first be booted and the necessary references obtained to initialize the
  // other subsystems.
  fxl::AutoResetWaitableEvent io_latch;
  std::unique_ptr<IOManager> io_manager;
  fml::WeakPtr<GrContext> resource_context;
  fxl::RefPtr<flow::SkiaUnrefQueue> unref_queue;
  shell->GetTaskRunners().GetIOTaskRunner()->RunNowOrPostTask(
      [&io_latch,          //
       &io_manager,        //
       &resource_context,  //
       &unref_queue,       //
       &platform_view      //
  ]() {
        io_manager =
            std::make_unique<IOManager>(platform_view->CreateResourceContext());
        resource_context = io_manager->GetResourceContext();
        unref_queue = io_manager->GetSkiaUnrefQueue();
        io_latch.Signal();
      });
  io_latch.Wait();

  // Create the rasterizer on the GPU thread.
  fxl::AutoResetWaitableEvent gpu_latch;
  std::unique_ptr<Rasterizer> rasterizer;
  task_runners.GetGPUTaskRunner()->RunNowOrPostTask([
    &gpu_latch,            //
    &rasterizer,           //
    on_create_rasterizer,  //
    shell = shell.get()    //
  ]() {
    if (auto new_rasterizer = on_create_rasterizer(*shell)) {
      rasterizer = std::move(new_rasterizer);
    }
    gpu_latch.Signal();
  });

  // Create the engine on the UI thread.
  fxl::AutoResetWaitableEvent ui_latch;
  std::unique_ptr<Engine> engine;
  shell->GetTaskRunners().GetUITaskRunner()->RunNowOrPostTask(
      fxl::MakeCopyable([
        &ui_latch,                                       //
        &engine,                                         //
        shell = shell.get(),                             //
        vsync_waiter = std::move(vsync_waiter),          //
        resource_context = std::move(resource_context),  //
        unref_queue = std::move(unref_queue)             //
      ]() mutable {
        const auto& task_runners = shell->GetTaskRunners();

        // The animator is owned by the UI thread but it gets its vsync pulses
        // from the platform.
        auto animator = std::make_unique<Animator>(*shell, task_runners,
                                                   std::move(vsync_waiter));

        engine = std::make_unique<Engine>(*shell,                       //
                                          shell->GetDartVM(),           //
                                          task_runners,                 //
                                          shell->GetSettings(),         //
                                          std::move(animator),          //
                                          std::move(resource_context),  //
                                          std::move(unref_queue)        //
        );
        ui_latch.Signal();
      }));

  gpu_latch.Wait();
  ui_latch.Wait();
  // We are already on the platform thread. So there is no platform latch to
  // wait on.

  if (!shell->Setup(std::move(platform_view),  //
                    std::move(engine),         //
                    std::move(rasterizer),     //
                    std::move(io_manager))     //
  ) {
    return nullptr;
  }

  return shell;
}

std::unique_ptr<Shell> Shell::Create(
    blink::TaskRunners task_runners,
    blink::Settings settings,
    Shell::CreateCallback<PlatformView> on_create_platform_view,
    Shell::CreateCallback<Rasterizer> on_create_rasterizer) {
  if (!task_runners.IsValid() || !on_create_platform_view ||
      !on_create_rasterizer) {
    return nullptr;
  }

  fxl::AutoResetWaitableEvent latch;
  std::unique_ptr<Shell> shell;
  task_runners.GetPlatformTaskRunner()->RunNowOrPostTask([
    &latch, &shell, task_runners = std::move(task_runners), settings,
    on_create_platform_view, on_create_rasterizer
  ]() {
    shell = CreateShellOnPlatformThread(std::move(task_runners), settings,
                                        on_create_platform_view,
                                        on_create_rasterizer);
    latch.Signal();
  });
  latch.Wait();
  return shell;
}

Shell::Shell(blink::TaskRunners task_runners, blink::Settings settings)
    : task_runners_(std::move(task_runners)),
      settings_(std::move(settings)),
      vm_(blink::DartVM::ForProcess(settings)) {
  FXL_DCHECK(task_runners_.IsValid());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  fml::icu::InitializeICU(settings_.icu_data_path);

  SkGraphics::Init();

  if (settings_.trace_skia) {
    InitSkiaEventTracer(settings_.trace_skia);
  }
}

Shell::~Shell() {
  fxl::AutoResetWaitableEvent ui_latch, gpu_latch, platform_latch, io_latch;

  task_runners_.GetUITaskRunner()->RunNowOrPostTask(
      fxl::MakeCopyable([ engine = std::move(engine_), &ui_latch ]() mutable {
        engine.reset();
        ui_latch.Signal();
      }));
  ui_latch.Wait();

  task_runners_.GetGPUTaskRunner()->RunNowOrPostTask(fxl::MakeCopyable(
      [ rasterizer = std::move(rasterizer_), &gpu_latch ]() mutable {
        rasterizer.reset();
        gpu_latch.Signal();
      }));
  gpu_latch.Wait();

  task_runners_.GetIOTaskRunner()->RunNowOrPostTask(fxl::MakeCopyable(
      [ io_manager = std::move(io_manager_), &io_latch ]() mutable {
        io_manager.reset();
        io_latch.Signal();
      }));

  io_latch.Wait();

  // The platform view must go last because it may be holding onto platform side
  // counterparts to resources owned by subsystems running on other threads. For
  // example, the NSOpenGLContext on the Mac.
  task_runners_.GetPlatformTaskRunner()->RunNowOrPostTask(fxl::MakeCopyable(
      [ platform_view = std::move(platform_view_), &platform_latch ]() mutable {
        platform_view.reset();
        platform_latch.Signal();
      }));
  platform_latch.Wait();
}

bool Shell::Setup(std::unique_ptr<PlatformView> platform_view,
                  std::unique_ptr<Engine> engine,
                  std::unique_ptr<Rasterizer> rasterizer,
                  std::unique_ptr<IOManager> io_manager) {
  if (is_setup_) {
    return false;
  }

  if (!platform_view || !engine || !rasterizer || !io_manager) {
    return false;
  }

  platform_view_ = std::move(platform_view);
  engine_ = std::move(engine);
  rasterizer_ = std::move(rasterizer);
  io_manager_ = std::move(io_manager);

  is_setup_ = true;

  return true;
}

const blink::Settings& Shell::GetSettings() const {
  return settings_;
}

const blink::TaskRunners& Shell::GetTaskRunners() const {
  return task_runners_;
}

fml::WeakPtr<Rasterizer> Shell::GetRasterizer() {
  FXL_DCHECK(is_setup_);
  return rasterizer_->GetWeakPtr();
}

fml::WeakPtr<Engine> Shell::GetEngine() {
  FXL_DCHECK(is_setup_);
  return engine_->GetWeakPtr();
}

fml::WeakPtr<PlatformView> Shell::GetPlatformView() {
  FXL_DCHECK(is_setup_);
  return platform_view_->GetWeakPtr();
}

const blink::DartVM& Shell::GetDartVM() const {
  return *vm_;
}

void Shell::RunInPlatformView(uintptr_t view_id,
                              const char* main_script,
                              const char* packages_file,
                              const char* asset_directory,
                              bool* view_existed,
                              int64_t* dart_isolate_id,
                              std::string* isolate_name) {
  FXL_DCHECK(is_setup_);

  if (main_script == nullptr || packages_file == nullptr ||
      asset_directory == nullptr || view_existed == nullptr || view_id == 0) {
    return;
  }

  *view_existed = false;

  if (reinterpret_cast<uintptr_t>(platform_view_.get()) != view_id) {
    return;
  }

  RunConfiguration config = {
      .bundle_path = asset_directory,
      .main_path = main_script,
      .packages_path = packages_file,
  };

  fxl::AutoResetWaitableEvent latch;
  task_runners_.GetUITaskRunner()->PostTask([
    engine = engine_->GetWeakPtr(),  //
    config = std::move(config),      //
    view_existed,                    //
    dart_isolate_id,                 //
    isolate_name,                    //
    &latch                           //
  ]() {
    auto signaller = [](decltype(latch)* latch) { latch->Signal(); };
    std::unique_ptr<decltype(latch), decltype(signaller)> auto_latch(&latch,
                                                                     signaller);

    if (!engine) {
      return;
    }

    if (!engine->Run(config)) {
      return;
    }

    *view_existed = true;
    *dart_isolate_id = engine->GetUIIsolateMainPort();
    *isolate_name = engine->GetUIIsolateName();
  });

  latch.Wait();  // Signalled in the UI thread PostTask.
}

// |shell::PlatformView::Delegate|
void Shell::OnPlatformViewCreated(const PlatformView& view,
                                  std::unique_ptr<Surface> surface) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(&view == platform_view_.get());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  // Note:
  // This is a synchronous operation because certain platforms depend on
  // setup/suspension of all activities that may be interacting with the GPU in
  // a synchronous fashion.

  fxl::AutoResetWaitableEvent latch;
  auto gpu_task = fxl::MakeCopyable([
    rasterizer = rasterizer_->GetWeakPtr(),  //
    surface = std::move(surface),            //
    &latch
  ]() mutable {
    if (rasterizer) {
      rasterizer->Setup(std::move(surface));
    }
    // Step 2: All done. Signal the latch that the platform thread is waiting
    // on.
    latch.Signal();
  });

  auto ui_task = [
    engine = engine_->GetWeakPtr(),                      //
    gpu_task_runner = task_runners_.GetGPUTaskRunner(),  //
    gpu_task                                             //
  ] {
    if (engine) {
      engine->OnOutputSurfaceCreated();
    }
    // Step 1: Next, tell the GPU thread that it should create a surface for its
    // rasterizer.
    gpu_task_runner->RunNowOrPostTask(gpu_task);
  };

  // Step 0: Post a task onto the UI thread to tell the engine that it has an
  // output surface.
  task_runners_.GetUITaskRunner()->RunNowOrPostTask(ui_task);
  latch.Wait();
}

// |shell::PlatformView::Delegate|
void Shell::OnPlatformViewDestroyed(const PlatformView& view) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(&view == platform_view_.get());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  // Note:
  // This is a synchronous operation because certain platforms depend on
  // setup/suspension of all activities that may be interacting with the GPU in
  // a synchronous fashion.

  fxl::AutoResetWaitableEvent latch;

  auto gpu_task = [ rasterizer = rasterizer_->GetWeakPtr(), &latch ]() {
    if (rasterizer) {
      rasterizer->Teardown();
    }
    // Step 2: All done. Signal the latch that the platform thread is waiting
    // on.
    latch.Signal();
  };

  auto ui_task = [
    engine = engine_->GetWeakPtr(),
    gpu_task_runner = task_runners_.GetGPUTaskRunner(), gpu_task
  ]() {
    if (engine) {
      engine->OnOutputSurfaceDestroyed();
    }
    // Step 1: Next, tell the GPU thread that its rasterizer should suspend
    // access to the underlying surface.
    gpu_task_runner->RunNowOrPostTask(gpu_task);
  };
  ;

  // Step 0: Post a task onto the UI thread to tell the engine that its output
  // surface is about to go away.
  task_runners_.GetUITaskRunner()->RunNowOrPostTask(ui_task);
  latch.Wait();
}

// |shell::PlatformView::Delegate|
void Shell::OnPlatformViewDispatchPlatformMessage(
    const PlatformView& view,
    fxl::RefPtr<blink::PlatformMessage> message) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(&view == platform_view_.get());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask(
      [ engine = engine_->GetWeakPtr(), message = std::move(message) ] {
        if (engine) {
          engine->DispatchPlatformMessage(std::move(message));
        }
      });
}

// |shell::PlatformView::Delegate|
void Shell::OnPlatformViewDispatchSemanticsAction(const PlatformView& view,
                                                  int32_t id,
                                                  blink::SemanticsAction action,
                                                  std::vector<uint8_t> args) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(&view == platform_view_.get());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask(
      [ engine = engine_->GetWeakPtr(), id, action, args = std::move(args) ] {
        if (engine) {
          engine->DispatchSemanticsAction(id, action, std::move(args));
        }
      });
}

// |shell::PlatformView::Delegate|
void Shell::OnPlatformViewSetSemanticsEnabled(const PlatformView& view,
                                              bool enabled) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(&view == platform_view_.get());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask(
      [ engine = engine_->GetWeakPtr(), enabled ] {
        if (engine) {
          engine->SetSemanticsEnabled(enabled);
        }
      });
}

// |shell::PlatformView::Delegate|
void Shell::OnPlatformViewRegisterTexture(
    const PlatformView& view,
    std::shared_ptr<flow::Texture> texture) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(&view == platform_view_.get());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetGPUTaskRunner()->PostTask(
      [ rasterizer = rasterizer_->GetWeakPtr(), texture ] {
        if (rasterizer) {
          rasterizer->GetTextureRegistry().RegisterTexture(texture);
        }
      });
}

// |shell::PlatformView::Delegate|
void Shell::OnPlatformViewUnregisterTexture(const PlatformView& view,
                                            int64_t texture_id) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(&view == platform_view_.get());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetGPUTaskRunner()->PostTask(
      [ rasterizer = rasterizer_->GetWeakPtr(), texture_id ]() {
        if (rasterizer) {
          rasterizer->GetTextureRegistry().UnregisterTexture(texture_id);
        }
      });
}

// |shell::PlatformView::Delegate|
void Shell::OnPlatformViewMarkTextureFrameAvailable(const PlatformView& view,
                                                    int64_t texture_id) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(&view == platform_view_.get());
  FXL_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask([engine = engine_->GetWeakPtr()]() {
    if (engine) {
      engine->ScheduleFrame(false);
    }
  });
}

// |shell::Animator::Delegate|
void Shell::OnAnimatorBeginFrame(const Animator& animator,
                                 fxl::TimePoint frame_time) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  if (engine_) {
    engine_->BeginFrame(frame_time);
  }
}

// |shell::Animator::Delegate|
void Shell::OnAnimatorNotifyIdle(const Animator& animator, int64_t deadline) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  if (engine_) {
    engine_->NotifyIdle(deadline);
  }
}

// |shell::Animator::Delegate|
void Shell::OnAnimatorDraw(
    const Animator& animator,
    fxl::RefPtr<flutter::Pipeline<flow::LayerTree>> pipeline) {
  FXL_DCHECK(is_setup_);

  task_runners_.GetGPUTaskRunner()->PostTask([
    rasterizer = rasterizer_->GetWeakPtr(), pipeline = std::move(pipeline)
  ]() {
    if (rasterizer) {
      rasterizer->Draw(pipeline);
    }
  });
}

// |shell::Animator::Delegate|
void Shell::OnAnimatorDrawLastLayerTree(const Animator& animator) {
  FXL_DCHECK(is_setup_);

  task_runners_.GetGPUTaskRunner()->PostTask([rasterizer =
                                                  rasterizer_->GetWeakPtr()]() {
    if (rasterizer) {
      rasterizer->DrawLastLayerTree();
    }
  });
}

// |shell::Engine::Delegate|
void Shell::OnEngineUpdateSemantics(const Engine& engine,
                                    std::vector<blink::SemanticsNode> update) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetPlatformTaskRunner()->PostTask(
      [ view = platform_view_->GetWeakPtr(), update = std::move(update) ] {
        if (view) {
          view->UpdateSemantics(std::move(update));
        }
      });
}

// |shell::Engine::Delegate|
void Shell::OnEngineHandlePlatformMessage(
    const Engine& engine,
    fxl::RefPtr<blink::PlatformMessage> message) {
  FXL_DCHECK(is_setup_);
  FXL_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetPlatformTaskRunner()->PostTask(
      [ view = platform_view_->GetWeakPtr(), message = std::move(message) ]() {
        if (view) {
          view->HandlePlatformMessage(std::move(message));
        }
      });
}

bool Shell::IsSetup() const {
  return is_setup_;
}

}  // namespace shell
