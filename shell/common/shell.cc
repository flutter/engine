// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define RAPIDJSON_HAS_STDSTRING 1

#include "flutter/shell/common/shell.h"

#include <memory>
#include <sstream>
#include <vector>

#include "flutter/assets/directory_asset_bundle.h"
#include "flutter/fml/file.h"
#include "flutter/fml/icu_util.h"
#include "flutter/fml/log_settings.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/paths.h"
#include "flutter/fml/trace_event.h"
#include "flutter/fml/unique_fd.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/runtime/start_up.h"
#include "flutter/shell/common/engine.h"
#include "flutter/shell/common/persistent_cache.h"
#include "flutter/shell/common/skia_event_tracer_impl.h"
#include "flutter/shell/common/switches.h"
#include "flutter/shell/common/vsync_waiter.h"
#include "third_party/dart/runtime/include/dart_tools_api.h"
#include "third_party/skia/include/core/SkGraphics.h"
#include "third_party/tonic/common/log.h"

namespace flutter {

constexpr char kSkiaChannel[] = "flutter/skia";

std::unique_ptr<Shell> Shell::CreateShellOnPlatformThread(
    TaskRunners task_runners,
    Settings settings,
    Shell::CreateCallback<PlatformView> on_create_platform_view,
    Shell::CreateCallback<Rasterizer> on_create_rasterizer) {
  if (!task_runners.IsValid()) {
    FML_LOG(ERROR) << "Task runners to run the shell were invalid.";
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

  shell->platform_view_ = std::move(platform_view);
  shell->platform_view_promise_.set_value(shell->platform_view_->GetWeakPtr());

  // Create the IO manager on the IO thread.
  auto io_task_runner = shell->GetTaskRunners().GetIOTaskRunner();
  fml::TaskRunner::RunNowOrPostTask(
      io_task_runner,
      [shell = shell.get(),  //
       io_task_runner        //
  ]() {
        TRACE_EVENT0("flutter", "ShellSetupIOSubsystem");
        auto io_manager = std::make_unique<ShellIOManager>(
            shell->GetPlatformView()->CreateResourceContext(), io_task_runner);
        shell->io_manager_ = std::move(io_manager);
        shell->io_manager_promise_.set_value(shell->io_manager_->GetWeakPtr());
      });

  // Create the rasterizer on the GPU thread.
  fml::TaskRunner::RunNowOrPostTask(
      task_runners.GetGPUTaskRunner(), [on_create_rasterizer,  //
                                        shell = shell.get()    //
  ]() {
        TRACE_EVENT0("flutter", "ShellSetupGPUSubsystem");
        if (auto new_rasterizer = on_create_rasterizer(*shell)) {
          shell->rasterizer_ = std::move(new_rasterizer);
          shell->rasterizer_promise_.set_value(
              shell->rasterizer_->GetWeakPtr());
        }
      });

  // Create the engine on the UI thread.
  fml::TaskRunner::RunNowOrPostTask(
      shell->GetTaskRunners().GetUITaskRunner(),
      fml::MakeCopyable([shell = shell.get(),                    //
                         vsync_waiter = std::move(vsync_waiter)  //
  ]() mutable {
        TRACE_EVENT0("flutter", "ShellSetupUISubsystem");

        auto vm = DartVMRef::Create(shell->settings_);
        FML_CHECK(vm) << "Must be able to initialize the VM.";
        shell->vm_ = vm.get();

        const auto& task_runners = shell->GetTaskRunners();

        // The animator is owned by the UI thread but it gets its vsync pulses
        // from the platform.
        auto animator = std::make_unique<Animator>(*shell, task_runners,
                                                   std::move(vsync_waiter));

        auto engine = std::make_unique<Engine>(
            *shell,                                         //
            *&vm,                                           //
            vm->GetVMData()->GetIsolateSnapshot(),          //
            DartSnapshot::Empty(),                          //
            task_runners,                                   //
            shell->GetSettings(),                           //
            std::move(animator),                            //
            shell->GetRasterizer()->GetSnapshotDelegate(),  //
            shell->GetIOManager()                           //
        );
        shell->engine_ = std::move(engine);
        shell->engine_promise_.set_value(shell->engine_->GetWeakPtr());
        vm->GetServiceProtocol()->AddHandler(
            shell, shell->GetServiceProtocolDescription());
      }));

  if (!shell->Setup()) {
    return nullptr;
  }

  return shell;
}

static void RecordStartupTimestamp() {
  if (engine_main_enter_ts == 0) {
    engine_main_enter_ts = Dart_TimelineGetMicros();
  }
}

// Though there can be multiple shells, some settings apply to all components in
// the process. These have to be setup before the shell or any of its
// sub-components can be initialized. In a perfect world, this would be empty.
// TODO(chinmaygarde): The unfortunate side effect of this call is that settings
// that cause shell initialization failures will still lead to some of their
// settings being applied.
static void PerformInitializationTasks(const Settings& settings) {
  {
    fml::LogSettings log_settings;
    log_settings.min_log_level =
        settings.verbose_logging ? fml::LOG_INFO : fml::LOG_ERROR;
    fml::SetLogSettings(log_settings);
  }

  static std::once_flag gShellSettingsInitialization = {};
  std::call_once(gShellSettingsInitialization, [&settings] {
    RecordStartupTimestamp();

    tonic::SetLogHandler(
        [](const char* message) { FML_LOG(ERROR) << message; });

    if (settings.trace_skia) {
      InitSkiaEventTracer(settings.trace_skia);
    }

    if (!settings.skia_deterministic_rendering_on_cpu) {
      SkGraphics::Init();
    } else {
      FML_DLOG(INFO) << "Skia deterministic rendering is enabled.";
    }

    if (settings.icu_initialization_required) {
      if (settings.icu_data_path.size() != 0) {
        fml::icu::InitializeICU(settings.icu_data_path);
      } else if (settings.icu_mapper) {
        fml::icu::InitializeICUFromMapping(settings.icu_mapper());
      } else {
        FML_DLOG(WARNING) << "Skipping ICU initialization in the shell.";
      }
    }
  });
}

std::unique_ptr<Shell> Shell::Create(
    TaskRunners task_runners,
    Settings settings,
    Shell::CreateCallback<PlatformView> on_create_platform_view,
    Shell::CreateCallback<Rasterizer> on_create_rasterizer) {
  PerformInitializationTasks(settings);

  TRACE_EVENT0("flutter", "Shell::Create");

  if (!task_runners.IsValid() || !on_create_platform_view ||
      !on_create_rasterizer) {
    return nullptr;
  }

  fml::AutoResetWaitableEvent latch;
  std::unique_ptr<Shell> shell;
  fml::TaskRunner::RunNowOrPostTask(
      task_runners.GetPlatformTaskRunner(),
      fml::MakeCopyable([&latch,                                  //
                         &shell,                                  //
                         task_runners = std::move(task_runners),  //
                         settings,                                //
                         on_create_platform_view,                 //
                         on_create_rasterizer                     //
  ]() mutable {
        shell = CreateShellOnPlatformThread(std::move(task_runners),  //
                                            settings,                 //
                                            on_create_platform_view,  //
                                            on_create_rasterizer      //
        );
        latch.Signal();
      }));
  latch.Wait();
  return shell;
}

Shell::Shell(TaskRunners task_runners, Settings settings)
    : task_runners_(std::move(task_runners)),
      settings_(std::move(settings)),
      platform_view_future_(std::shared_future<fml::WeakPtr<PlatformView>>(
          platform_view_promise_.get_future())),
      engine_future_(std::shared_future<fml::WeakPtr<Engine>>(
          engine_promise_.get_future())),
      rasterizer_future_(std::shared_future<fml::WeakPtr<Rasterizer>>(
          rasterizer_promise_.get_future())),
      io_manager_future_(std::shared_future<fml::WeakPtr<ShellIOManager>>(
          io_manager_promise_.get_future())),
      weak_factory_(this) {
  FML_DCHECK(task_runners_.IsValid());
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  // Install service protocol handlers.

  service_protocol_handlers_[ServiceProtocol::kScreenshotExtensionName
                                 .ToString()] = {
      task_runners_.GetGPUTaskRunner(),
      std::bind(&Shell::OnServiceProtocolScreenshot, this,
                std::placeholders::_1, std::placeholders::_2)};
  service_protocol_handlers_[ServiceProtocol::kScreenshotSkpExtensionName
                                 .ToString()] = {
      task_runners_.GetGPUTaskRunner(),
      std::bind(&Shell::OnServiceProtocolScreenshotSKP, this,
                std::placeholders::_1, std::placeholders::_2)};
  service_protocol_handlers_[ServiceProtocol::kRunInViewExtensionName
                                 .ToString()] = {
      task_runners_.GetUITaskRunner(),
      std::bind(&Shell::OnServiceProtocolRunInView, this, std::placeholders::_1,
                std::placeholders::_2)};
  service_protocol_handlers_[ServiceProtocol::kFlushUIThreadTasksExtensionName
                                 .ToString()] = {
      task_runners_.GetUITaskRunner(),
      std::bind(&Shell::OnServiceProtocolFlushUIThreadTasks, this,
                std::placeholders::_1, std::placeholders::_2)};
  service_protocol_handlers_[ServiceProtocol::kSetAssetBundlePathExtensionName
                                 .ToString()] = {
      task_runners_.GetUITaskRunner(),
      std::bind(&Shell::OnServiceProtocolSetAssetBundlePath, this,
                std::placeholders::_1, std::placeholders::_2)};
  service_protocol_handlers_
      [ServiceProtocol::kGetDisplayRefreshRateExtensionName.ToString()] = {
          task_runners_.GetUITaskRunner(),
          std::bind(&Shell::OnServiceProtocolGetDisplayRefreshRate, this,
                    std::placeholders::_1, std::placeholders::_2)};
}

Shell::~Shell() {
  PersistentCache::GetCacheForProcess()->RemoveWorkerTaskRunner(
      task_runners_.GetIOTaskRunner());

  fml::AutoResetWaitableEvent ui_latch, gpu_latch, platform_latch, io_latch;

  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetUITaskRunner(),
      fml::MakeCopyable([shell = GetShell(), &ui_latch]() mutable {
        shell->vm_->GetServiceProtocol()->RemoveHandler(shell.get());
        shell->engine_future_.wait();
        shell->engine_.reset();
        ui_latch.Signal();
      }));
  ui_latch.Wait();

  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetGPUTaskRunner(),
      fml::MakeCopyable([shell = GetShell(), &gpu_latch]() mutable {
        shell->rasterizer_future_.wait();
        shell->rasterizer_.reset();
        gpu_latch.Signal();
      }));
  gpu_latch.Wait();

  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetIOTaskRunner(),
      fml::MakeCopyable([shell = GetShell(), &io_latch]() mutable {
        shell->platform_view_future_.wait();
        shell->io_manager_future_.wait();
        shell->io_manager_.reset();
        auto platform_view = shell->platform_view_.get();
        if (platform_view) {
          platform_view->ReleaseResourceContext();
        }
        io_latch.Signal();
      }));

  io_latch.Wait();

  // The platform view must go last because it may be holding onto platform side
  // counterparts to resources owned by subsystems running on other threads. For
  // example, the NSOpenGLContext on the Mac.
  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetPlatformTaskRunner(),
      fml::MakeCopyable([shell = GetShell(), &platform_latch]() mutable {
        shell->platform_view_.reset();
        platform_latch.Signal();
      }));
  platform_latch.Wait();
}

bool Shell::IsSetup() const {
  return is_setup_;
}

bool Shell::Setup() {
  if (is_setup_) {
    return false;
  }

  is_setup_ = true;

  PersistentCache::GetCacheForProcess()->AddWorkerTaskRunner(
      task_runners_.GetIOTaskRunner());

  PersistentCache::GetCacheForProcess()->SetIsDumpingSkp(
      settings_.dump_skp_on_shader_compilation);

  return true;
}

const Settings& Shell::GetSettings() const {
  return settings_;
}

const TaskRunners& Shell::GetTaskRunners() const {
  return task_runners_;
}

fml::WeakPtr<ShellIOManager> Shell::GetIOManager() const {
  if (io_manager_) {
    return io_manager_->GetWeakPtr();
  }
  if (io_manager_future_.valid()) {
    return io_manager_future_.get();
  }
  return {};
}

fml::WeakPtr<Rasterizer> Shell::GetRasterizer() const {
  if (rasterizer_) {
    return rasterizer_->GetWeakPtr();
  }
  if (rasterizer_future_.valid()) {
    return rasterizer_future_.get();
  }
  return {};
}

fml::WeakPtr<Engine> Shell::GetEngine() const {
  if (engine_) {
    return engine_->GetWeakPtr();
  }
  if (engine_future_.valid()) {
    return engine_future_.get();
  }
  return {};
}

fml::WeakPtr<PlatformView> Shell::GetPlatformView() const {
  if (platform_view_) {
    return platform_view_->GetWeakPtr();
  }
  if (platform_view_future_.valid()) {
    return platform_view_future_.get();
  }
  return {};
}

fml::WeakPtr<Shell> Shell::GetShell() const {
  return weak_factory_.GetWeakPtr();
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewCreated(std::unique_ptr<Surface> surface) {
  TRACE_EVENT0("flutter", "Shell::OnPlatformViewCreated");
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  // Note:
  // This is a synchronous operation because certain platforms depend on
  // setup/suspension of all activities that may be interacting with the GPU in
  // a synchronous fashion.

  auto ui_task = [shell = GetShell()] {
    if (shell) {
      auto engine = shell->GetEngine();
      if (engine) {
        engine->OnOutputSurfaceCreated();
      }
    }
  };

  fml::AutoResetWaitableEvent latch;
  auto gpu_task =
      fml::MakeCopyable([shell = GetShell(),                                //
                         surface = std::move(surface),                      //
                         ui_task_runner = task_runners_.GetUITaskRunner(),  //
                         ui_task, &latch]() mutable {
        if (shell) {
          auto rasterizer = shell->GetRasterizer();
          if (rasterizer) {
            rasterizer->Setup(std::move(surface));
          }
        }

        // Step 2: Next, post a task on the UI thread to tell the engine that it
        // has an output surface.
        fml::TaskRunner::RunNowOrPostTask(ui_task_runner, ui_task);

        // Step 3: All done. No need to wait for ui task.
        // Signal the latch that the platform thread is waiting on.
        latch.Signal();
      });

  // The normal flow executed by this method is that the platform thread is
  // starting the sequence and waiting on the latch. Later the UI thread posts
  // gpu_task to the GPU thread which signals the latch. If the GPU the and
  // platform threads are the same this results in a deadlock as the gpu_task
  // will never be posted to the plaform/gpu thread that is blocked on a latch.
  // To avoid the described deadlock, if the gpu and the platform threads are
  // the same, should_post_gpu_task will be false, and then instead of posting a
  // task to the gpu thread, the ui thread just signals the latch and the
  // platform/gpu thread follows with executing gpu_task.
  bool should_post_gpu_task =
      task_runners_.GetGPUTaskRunner() != task_runners_.GetPlatformTaskRunner();

  // Threading: Capture platform view by raw pointer and not the weak pointer.
  // We are going to use the pointer on the IO thread which is not safe with a
  // weak pointer. However, we are preventing the platform view from being
  // collected by using a latch.
  auto* platform_view = GetPlatformView().get();

  FML_DCHECK(platform_view);

  auto io_task = [shell = GetShell(), platform_view,
                  gpu_task_runner = task_runners_.GetGPUTaskRunner(), gpu_task,
                  &latch, should_post_gpu_task] {
    if (shell) {
      auto io_manager = shell->GetIOManager();
      if (io_manager && !io_manager->GetResourceContext()) {
        io_manager->NotifyResourceContextAvailable(
            platform_view->CreateResourceContext());
      }
    }

    // Step 1: Tell the GPU thread that it should create a surface for its
    // rasterizer.
    if (should_post_gpu_task) {
      fml::TaskRunner::RunNowOrPostTask(gpu_task_runner, gpu_task);
    } else {
      // See comment on should_post_gpu_task, in this case we just unblock
      // the platform thread.
      latch.Signal();
    }
  };

  fml::TaskRunner::RunNowOrPostTask(task_runners_.GetIOTaskRunner(), io_task);

  latch.Wait();
  if (!should_post_gpu_task) {
    // See comment on should_post_gpu_task, in this case the gpu_task
    // wasn't executed, and we just run it here as the platform thread
    // is the GPU thread.
    gpu_task();
  }
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewDestroyed() {
  TRACE_EVENT0("flutter", "Shell::OnPlatformViewDestroyed");
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  // Note:
  // This is a synchronous operation because certain platforms depend on
  // setup/suspension of all activities that may be interacting with the GPU in
  // a synchronous fashion.

  fml::AutoResetWaitableEvent latch;

  auto io_task = [io_manager = GetIOManager(), &latch]() {
    if (io_manager) {
      // Execute any pending Skia object deletions while GPU access is still
      // allowed.
      io_manager->GetSkiaUnrefQueue()->Drain();
    }
    // Step 3: All done. Signal the latch that the platform thread is waiting
    // on.
    latch.Signal();
  };

  auto gpu_task = [rasterizer = GetRasterizer(),
                   io_task_runner = task_runners_.GetIOTaskRunner(),
                   io_task]() {
    if (rasterizer) {
      rasterizer->Teardown();
    }
    // Step 2: Next, tell the IO thread to complete its remaining work.
    fml::TaskRunner::RunNowOrPostTask(io_task_runner, io_task);
  };

  // The normal flow executed by this method is that the platform thread is
  // starting the sequence and waiting on the latch. Later the UI thread posts
  // gpu_task to the GPU thread triggers signaling the latch(on the IO thread).
  // If the GPU the and platform threads are the same this results in a deadlock
  // as the gpu_task will never be posted to the plaform/gpu thread that is
  // blocked on a latch.  To avoid the described deadlock, if the gpu and the
  // platform threads are the same, should_post_gpu_task will be false, and then
  // instead of posting a task to the gpu thread, the ui thread just signals the
  // latch and the platform/gpu thread follows with executing gpu_task.
  bool should_post_gpu_task =
      task_runners_.GetGPUTaskRunner() != task_runners_.GetPlatformTaskRunner();

  auto ui_task = [engine = GetEngine(),
                  gpu_task_runner = task_runners_.GetGPUTaskRunner(), gpu_task,
                  should_post_gpu_task, &latch]() {
    if (engine) {
      engine->OnOutputSurfaceDestroyed();
    }
    // Step 1: Next, tell the GPU thread that its rasterizer should suspend
    // access to the underlying surface.
    if (should_post_gpu_task) {
      fml::TaskRunner::RunNowOrPostTask(gpu_task_runner, gpu_task);
    } else {
      // See comment on should_post_gpu_task, in this case we just unblock
      // the platform thread.
      latch.Signal();
    }
  };

  // Step 0: Post a task onto the UI thread to tell the engine that its output
  // surface is about to go away.
  fml::TaskRunner::RunNowOrPostTask(task_runners_.GetUITaskRunner(), ui_task);
  latch.Wait();
  if (!should_post_gpu_task) {
    // See comment on should_post_gpu_task, in this case the gpu_task
    // wasn't executed, and we just run it here as the platform thread
    // is the GPU thread.
    gpu_task();
    latch.Wait();
  }
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewSetViewportMetrics(const ViewportMetrics& metrics) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask([shell = GetShell(), metrics]() {
    if (!shell) {
      return;
    }
    auto engine = shell->GetEngine();
    if (engine) {
      engine->SetViewportMetrics(metrics);
    }
  });
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewDispatchPlatformMessage(
    fml::RefPtr<PlatformMessage> message) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask(
      [shell = GetShell(), message = std::move(message)] {
        if (!shell) {
          return;
        }
        auto engine = shell->GetEngine();
        if (engine) {
          engine->DispatchPlatformMessage(std::move(message));
        }
      });
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewDispatchPointerDataPacket(
    std::unique_ptr<PointerDataPacket> packet) {
  TRACE_EVENT0("flutter", "Shell::OnPlatformViewDispatchPointerDataPacket");
  TRACE_FLOW_BEGIN("flutter", "PointerEvent", next_pointer_flow_id_);
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());
  task_runners_.GetUITaskRunner()->PostTask(
      fml::MakeCopyable([shell = GetShell(), packet = std::move(packet),
                         flow_id = next_pointer_flow_id_] {
        if (!shell) {
          return;
        }
        auto engine = shell->GetEngine();
        if (engine) {
          engine->DispatchPointerDataPacket(*packet, flow_id);
        }
      }));
  next_pointer_flow_id_++;
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewDispatchSemanticsAction(int32_t id,
                                                  SemanticsAction action,
                                                  std::vector<uint8_t> args) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask(
      [shell = GetShell(), id, action, args = std::move(args)] {
        if (!shell) {
          return;
        }
        auto engine = shell->GetEngine();
        if (engine) {
          engine->DispatchSemanticsAction(id, action, std::move(args));
        }
      });
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewSetSemanticsEnabled(bool enabled) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask([shell = GetShell(), enabled] {
    if (!shell) {
      return;
    }
    auto engine = shell->GetEngine();
    if (engine) {
      engine->SetSemanticsEnabled(enabled);
    }
  });
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewSetAccessibilityFeatures(int32_t flags) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetUITaskRunner()->PostTask([shell = GetShell(), flags] {
    if (!shell) {
      return;
    }
    auto engine = shell->GetEngine();
    if (engine) {
      engine->SetAccessibilityFeatures(flags);
    }
  });
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewRegisterTexture(
    std::shared_ptr<flutter::Texture> texture) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetGPUTaskRunner()->PostTask([shell = GetShell(), texture] {
    if (!shell) {
      return;
    }
    auto rasterizer = shell->GetRasterizer();
    if (rasterizer) {
      if (auto* registry = rasterizer->GetTextureRegistry()) {
        registry->RegisterTexture(texture);
      }
    }
  });
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewUnregisterTexture(int64_t texture_id) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetGPUTaskRunner()->PostTask(
      [shell = GetShell(), texture_id]() {
        if (!shell) {
          return;
        }
        auto rasterizer = shell->GetRasterizer();
        if (rasterizer) {
          if (auto* registry = rasterizer->GetTextureRegistry()) {
            registry->UnregisterTexture(texture_id);
          }
        }
      });
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewMarkTextureFrameAvailable(int64_t texture_id) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  // Tell the rasterizer that one of its textures has a new frame available.
  task_runners_.GetGPUTaskRunner()->PostTask(
      [shell = GetShell(), texture_id]() {
        if (!shell) {
          return;
        }
        auto rasterizer = shell->GetRasterizer();
        if (!rasterizer) {
          return;
        }

        auto* registry = rasterizer->GetTextureRegistry();

        if (!registry) {
          return;
        }

        auto texture = registry->GetTexture(texture_id);

        if (!texture) {
          return;
        }

        texture->MarkNewFrameAvailable();
      });

  // Schedule a new frame without having to rebuild the layer tree.
  task_runners_.GetUITaskRunner()->PostTask([shell = GetShell()]() {
    if (!shell) {
      return;
    }
    auto engine = shell->GetEngine();
    if (engine) {
      engine->ScheduleFrame(false);
    }
  });
}

// |PlatformView::Delegate|
void Shell::OnPlatformViewSetNextFrameCallback(fml::closure closure) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetGPUTaskRunner()->PostTask(
      [shell = GetShell(), closure = std::move(closure)]() {
        if (!shell) {
          return;
        }
        auto rasterizer = shell->GetRasterizer();
        if (rasterizer) {
          rasterizer->SetNextFrameCallback(std::move(closure));
        }
      });
}

// |Animator::Delegate|
void Shell::OnAnimatorBeginFrame(fml::TimePoint frame_time) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  auto engine = GetEngine();
  if (engine) {
    engine->BeginFrame(frame_time);
  }
}

// |Animator::Delegate|
void Shell::OnAnimatorNotifyIdle(int64_t deadline) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  auto engine = GetEngine();
  if (engine) {
    engine->NotifyIdle(deadline);
  }
}

// |Animator::Delegate|
void Shell::OnAnimatorDraw(fml::RefPtr<Pipeline<flutter::LayerTree>> pipeline) {
  FML_DCHECK(is_setup_);

  task_runners_.GetGPUTaskRunner()->PostTask(
      [shell = GetShell(), pipeline = std::move(pipeline)]() {
        if (!shell) {
          return;
        }
        auto rasterizer = shell->GetRasterizer();
        if (rasterizer) {
          rasterizer->Draw(pipeline);
        }
      });
}

// |Animator::Delegate|
void Shell::OnAnimatorDrawLastLayerTree() {
  FML_DCHECK(is_setup_);

  task_runners_.GetGPUTaskRunner()->PostTask([shell = GetShell()]() {
    if (!shell) {
      return;
    }
    auto rasterizer = shell->GetRasterizer();
    if (rasterizer) {
      rasterizer->DrawLastLayerTree();
    }
  });
}

// |Engine::Delegate|
void Shell::OnEngineUpdateSemantics(SemanticsNodeUpdates update,
                                    CustomAccessibilityActionUpdates actions) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  task_runners_.GetPlatformTaskRunner()->PostTask(
      [shell = GetShell(), update = std::move(update),
       actions = std::move(actions)] {
        if (!shell) {
          return;
        }
        auto view = shell->GetPlatformView();
        if (view) {
          view->UpdateSemantics(std::move(update), std::move(actions));
        }
      });
}

// |Engine::Delegate|
void Shell::OnEngineHandlePlatformMessage(
    fml::RefPtr<PlatformMessage> message) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  if (message->channel() == kSkiaChannel) {
    HandleEngineSkiaMessage(std::move(message));
    return;
  }

  task_runners_.GetPlatformTaskRunner()->PostTask(
      [shell = GetShell(), message = std::move(message)]() {
        if (!shell) {
          return;
        }
        auto view = shell->GetPlatformView();
        if (view) {
          view->HandlePlatformMessage(std::move(message));
        }
      });
}

void Shell::HandleEngineSkiaMessage(fml::RefPtr<PlatformMessage> message) {
  const auto& data = message->data();

  rapidjson::Document document;
  document.Parse(reinterpret_cast<const char*>(data.data()), data.size());
  if (document.HasParseError() || !document.IsObject())
    return;
  auto root = document.GetObject();
  auto method = root.FindMember("method");
  if (method->value != "Skia.setResourceCacheMaxBytes")
    return;
  auto args = root.FindMember("args");
  if (args == root.MemberEnd() || !args->value.IsInt())
    return;

  task_runners_.GetGPUTaskRunner()->PostTask(
      [shell = GetShell(), max_bytes = args->value.GetInt()] {
        if (!shell) {
          return;
        }
        auto rasterizer = shell->GetRasterizer();
        if (rasterizer) {
          rasterizer->SetResourceCacheMaxBytes(max_bytes);
        }
      });
}

// |Engine::Delegate|
void Shell::OnPreEngineRestart() {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  fml::AutoResetWaitableEvent latch;
  fml::TaskRunner::RunNowOrPostTask(task_runners_.GetPlatformTaskRunner(),
                                    [shell = GetShell(), &latch]() {
                                      if (!shell) {
                                        return;
                                      }
                                      auto view = shell->GetPlatformView();
                                      if (view) {
                                        view->OnPreEngineRestart();
                                      }
                                      latch.Signal();
                                    });
  // This is blocking as any embedded platform views has to be flushed before
  // we re-run the Dart code.
  latch.Wait();
}

// |Engine::Delegate|
void Shell::UpdateIsolateDescription(const std::string isolate_name,
                                     int64_t isolate_port) {
  Handler::Description description(isolate_port, isolate_name);
  vm_->GetServiceProtocol()->SetHandlerDescription(this, description);
}

void Shell::SetNeedsReportTimings(bool value) {
  needs_report_timings_ = value;
}

void Shell::ReportTimings() {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetGPUTaskRunner()->RunsTasksOnCurrentThread());

  auto timings = std::move(unreported_timings_);
  unreported_timings_ = {};
  task_runners_.GetUITaskRunner()->PostTask([shell = GetShell(), timings] {
    if (!shell) {
      return;
    }
    auto engine = shell->GetEngine();
    if (engine) {
      engine->ReportTimings(std::move(timings));
    }
  });
}

size_t Shell::UnreportedFramesCount() const {
  // Check that this is running on the GPU thread to avoid race conditions.
  FML_DCHECK(task_runners_.GetGPUTaskRunner()->RunsTasksOnCurrentThread());
  FML_DCHECK(unreported_timings_.size() % FrameTiming::kCount == 0);
  return unreported_timings_.size() / FrameTiming::kCount;
}

void Shell::OnFrameRasterized(const FrameTiming& timing) {
  FML_DCHECK(is_setup_);
  FML_DCHECK(task_runners_.GetGPUTaskRunner()->RunsTasksOnCurrentThread());

  // The C++ callback defined in settings.h and set by Flutter runner. This is
  // independent of the timings report to the Dart side.
  if (settings_.frame_rasterized_callback) {
    settings_.frame_rasterized_callback(timing);
  }

  if (!needs_report_timings_) {
    return;
  }

  for (auto phase : FrameTiming::kPhases) {
    unreported_timings_.push_back(
        timing.Get(phase).ToEpochDelta().ToMicroseconds());
  }

  // In tests using iPhone 6S with profile mode, sending a batch of 1 frame or a
  // batch of 100 frames have roughly the same cost of less than 0.1ms. Sending
  // a batch of 500 frames costs about 0.2ms. The 1 second threshold usually
  // kicks in before we reaching the following 100 frames threshold. The 100
  // threshold here is mainly for unit tests (so we don't have to write a
  // 1-second unit test), and make sure that our vector won't grow too big with
  // future 120fps, 240fps, or 1000fps displays.
  //
  // In the profile/debug mode, the timings are used by development tools which
  // require a latency of no more than 100ms. Hence we lower that 1-second
  // threshold to 100ms because performance overhead isn't that critical in
  // those cases.
  if (UnreportedFramesCount() >= 100) {
    ReportTimings();
  } else if (!frame_timings_report_scheduled_) {
#if FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_RELEASE
    constexpr int kBatchTimeInMilliseconds = 1000;
#else
    constexpr int kBatchTimeInMilliseconds = 100;
#endif

    // Also make sure that frame times get reported with a max latency of 1
    // second. Otherwise, the timings of last few frames of an animation may
    // never be reported until the next animation starts.
    frame_timings_report_scheduled_ = true;
    task_runners_.GetGPUTaskRunner()->PostDelayedTask(
        [self = GetShell()]() {
          if (!self.get()) {
            return;
          }
          self->frame_timings_report_scheduled_ = false;
          if (self->UnreportedFramesCount() > 0) {
            self->ReportTimings();
          }
        },
        fml::TimeDelta::FromMilliseconds(kBatchTimeInMilliseconds));
  }
}

// |ServiceProtocol::Handler|
fml::RefPtr<fml::TaskRunner> Shell::GetServiceProtocolHandlerTaskRunner(
    fml::StringView method) const {
  FML_DCHECK(is_setup_);
  auto found = service_protocol_handlers_.find(method.ToString());
  if (found != service_protocol_handlers_.end()) {
    return found->second.first;
  }
  return task_runners_.GetUITaskRunner();
}

// |ServiceProtocol::Handler|
bool Shell::HandleServiceProtocolMessage(
    fml::StringView method,  // one if the extension names specified above.
    const ServiceProtocolMap& params,
    rapidjson::Document& response) {
  auto found = service_protocol_handlers_.find(method.ToString());
  if (found != service_protocol_handlers_.end()) {
    return found->second.second(params, response);
  }
  return false;
}

// |ServiceProtocol::Handler|
ServiceProtocol::Handler::Description Shell::GetServiceProtocolDescription()
    const {
  return {
      GetEngine()->GetUIIsolateMainPort(),
      GetEngine()->GetUIIsolateName(),
  };
}

static void ServiceProtocolParameterError(rapidjson::Document& response,
                                          std::string error_details) {
  auto& allocator = response.GetAllocator();
  response.SetObject();
  const int64_t kInvalidParams = -32602;
  response.AddMember("code", kInvalidParams, allocator);
  response.AddMember("message", "Invalid params", allocator);
  {
    rapidjson::Value details(rapidjson::kObjectType);
    details.AddMember("details", error_details, allocator);
    response.AddMember("data", details, allocator);
  }
}

static void ServiceProtocolFailureError(rapidjson::Document& response,
                                        std::string message) {
  auto& allocator = response.GetAllocator();
  response.SetObject();
  const int64_t kJsonServerError = -32000;
  response.AddMember("code", kJsonServerError, allocator);
  response.AddMember("message", message, allocator);
}

// Service protocol handler
bool Shell::OnServiceProtocolScreenshot(
    const ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  FML_DCHECK(task_runners_.GetGPUTaskRunner()->RunsTasksOnCurrentThread());
  auto screenshot = GetRasterizer()->ScreenshotLastLayerTree(
      Rasterizer::ScreenshotType::CompressedImage, true);
  if (screenshot.data) {
    response.SetObject();
    auto& allocator = response.GetAllocator();
    response.AddMember("type", "Screenshot", allocator);
    rapidjson::Value image;
    image.SetString(static_cast<const char*>(screenshot.data->data()),
                    screenshot.data->size(), allocator);
    response.AddMember("screenshot", image, allocator);
    return true;
  }
  ServiceProtocolFailureError(response, "Could not capture image screenshot.");
  return false;
}

// Service protocol handler
bool Shell::OnServiceProtocolScreenshotSKP(
    const ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  FML_DCHECK(task_runners_.GetGPUTaskRunner()->RunsTasksOnCurrentThread());
  auto screenshot = GetRasterizer()->ScreenshotLastLayerTree(
      Rasterizer::ScreenshotType::SkiaPicture, true);
  if (screenshot.data) {
    response.SetObject();
    auto& allocator = response.GetAllocator();
    response.AddMember("type", "ScreenshotSkp", allocator);
    rapidjson::Value skp;
    skp.SetString(static_cast<const char*>(screenshot.data->data()),
                  screenshot.data->size(), allocator);
    response.AddMember("skp", skp, allocator);
    return true;
  }
  ServiceProtocolFailureError(response, "Could not capture SKP screenshot.");
  return false;
}

// Service protocol handler
bool Shell::OnServiceProtocolRunInView(
    const ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  if (params.count("mainScript") == 0) {
    ServiceProtocolParameterError(response,
                                  "'mainScript' parameter is missing.");
    return false;
  }

  // TODO(chinmaygarde): In case of hot-reload from .dill files, the packages
  // file is ignored. Currently, the tool is passing a junk packages file to
  // pass this check. Update the service protocol interface and remove this
  // workaround.
  if (params.count("packagesFile") == 0) {
    ServiceProtocolParameterError(response,
                                  "'packagesFile' parameter is missing.");
    return false;
  }

  if (params.count("assetDirectory") == 0) {
    ServiceProtocolParameterError(response,
                                  "'assetDirectory' parameter is missing.");
    return false;
  }

  std::string main_script_path =
      fml::paths::FromURI(params.at("mainScript").ToString());
  std::string packages_path =
      fml::paths::FromURI(params.at("packagesFile").ToString());
  std::string asset_directory_path =
      fml::paths::FromURI(params.at("assetDirectory").ToString());

  auto main_script_file_mapping =
      std::make_unique<fml::FileMapping>(fml::OpenFile(
          main_script_path.c_str(), false, fml::FilePermission::kRead));

  auto isolate_configuration = IsolateConfiguration::CreateForKernel(
      std::move(main_script_file_mapping));

  RunConfiguration configuration(std::move(isolate_configuration));

  configuration.AddAssetResolver(
      std::make_unique<DirectoryAssetBundle>(fml::OpenDirectory(
          asset_directory_path.c_str(), false, fml::FilePermission::kRead)));

  auto& allocator = response.GetAllocator();
  response.SetObject();
  if (GetEngine()->Restart(std::move(configuration))) {
    response.AddMember("type", "Success", allocator);
    auto new_description = GetServiceProtocolDescription();
    rapidjson::Value view(rapidjson::kObjectType);
    new_description.Write(this, view, allocator);
    response.AddMember("view", view, allocator);
    return true;
  } else {
    FML_DLOG(ERROR) << "Could not run configuration in engine.";
    ServiceProtocolFailureError(response,
                                "Could not run configuration in engine.");
    return false;
  }

  FML_DCHECK(false);
  return false;
}

// Service protocol handler
bool Shell::OnServiceProtocolFlushUIThreadTasks(
    const ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());
  // This API should not be invoked by production code.
  // It can potentially starve the service isolate if the main isolate pauses
  // at a breakpoint or is in an infinite loop.
  //
  // It should be invoked from the VM Service and and blocks it until UI thread
  // tasks are processed.
  response.SetObject();
  response.AddMember("type", "Success", response.GetAllocator());
  return true;
}

bool Shell::OnServiceProtocolGetDisplayRefreshRate(
    const ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());
  response.SetObject();
  response.AddMember("fps", GetEngine()->GetDisplayRefreshRate(),
                     response.GetAllocator());
  return true;
}

// Service protocol handler
bool Shell::OnServiceProtocolSetAssetBundlePath(
    const ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  if (params.count("assetDirectory") == 0) {
    ServiceProtocolParameterError(response,
                                  "'assetDirectory' parameter is missing.");
    return false;
  }

  auto& allocator = response.GetAllocator();
  response.SetObject();

  auto asset_manager = std::make_shared<AssetManager>();

  asset_manager->PushFront(std::make_unique<DirectoryAssetBundle>(
      fml::OpenDirectory(params.at("assetDirectory").ToString().c_str(), false,
                         fml::FilePermission::kRead)));

  if (GetEngine()->UpdateAssetManager(std::move(asset_manager))) {
    response.AddMember("type", "Success", allocator);
    auto new_description = GetServiceProtocolDescription();
    rapidjson::Value view(rapidjson::kObjectType);
    new_description.Write(this, view, allocator);
    response.AddMember("view", view, allocator);
    return true;
  } else {
    FML_DLOG(ERROR) << "Could not update asset directory.";
    ServiceProtocolFailureError(response, "Could not update asset directory.");
    return false;
  }

  FML_DCHECK(false);
  return false;
}

Rasterizer::Screenshot Shell::Screenshot(
    Rasterizer::ScreenshotType screenshot_type,
    bool base64_encode) {
  TRACE_EVENT0("flutter", "Shell::Screenshot");
  fml::AutoResetWaitableEvent latch;
  Rasterizer::Screenshot screenshot;
  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetGPUTaskRunner(), [&latch,                        //
                                         rasterizer = GetRasterizer(),  //
                                         &screenshot,                   //
                                         screenshot_type,               //
                                         base64_encode                  //
  ]() {
        if (rasterizer) {
          screenshot = rasterizer->ScreenshotLastLayerTree(screenshot_type,
                                                           base64_encode);
        }
        latch.Signal();
      });
  latch.Wait();
  return screenshot;
}

}  // namespace flutter
