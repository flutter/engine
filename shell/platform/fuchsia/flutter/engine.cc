// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/shell/platform/fuchsia/flutter/engine.h"

#include <lib/async/cpp/task.h>
#include <lib/async/default.h>
#include <zircon/status.h>

#include "flutter/common/task_runners.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/task_runner.h"
#include "flutter/runtime/dart_vm_lifecycle.h"
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/common/run_configuration.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/embedder_render_target.h"
#include "flutter/shell/platform/embedder/platform_view_embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/fuchsia_intl.h"
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/files.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/ports/SkFontMgr_fuchsia.h"

namespace flutter_runner {
namespace {

void UpdateNativeThreadLabelNames(const std::string& label,
                                  const flutter::TaskRunners& runners) {
  auto set_thread_name = [](fml::RefPtr<fml::TaskRunner> runner,
                            std::string prefix, std::string suffix) {
    if (!runner) {
      return;
    }
    fml::TaskRunner::RunNowOrPostTask(runner, [name = prefix + suffix]() {
      zx::thread::self()->set_property(ZX_PROP_NAME, name.c_str(), name.size());
    });
  };
  set_thread_name(runners.GetPlatformTaskRunner(), label, ".platform");
  set_thread_name(runners.GetUITaskRunner(), label, ".ui");
  set_thread_name(runners.GetGPUTaskRunner(), label, ".gpu");
  set_thread_name(runners.GetIOTaskRunner(), label, ".io");
}

sk_sp<SkSurface> MakeSkSurfaceFromBackingStore(
    GrContext* context,
    const FlutterBackingStoreConfig& config,
    const FlutterSoftwareBackingStore* software) {
  const auto image_info =
      SkImageInfo::MakeN32Premul(config.size.width, config.size.height);

  struct Captures {
    VoidCallback destruction_callback;
    void* user_data;
  };
  auto captures = std::make_unique<Captures>();
  captures->destruction_callback = software->destruction_callback;
  captures->user_data = software->user_data;
  auto release_proc = [](void* pixels, void* context) {
    auto captures = reinterpret_cast<Captures*>(context);
    captures->destruction_callback(captures->user_data);
  };

  auto surface = SkSurface::MakeRasterDirectReleaseProc(
      image_info,                               // image info
      const_cast<void*>(software->allocation),  // pixels
      software->row_bytes,                      // row bytes
      release_proc,                             // release proc
      captures.release()                        // release context
  );

  if (!surface) {
    FML_LOG(ERROR)
        << "Could not wrap embedder supplied software render buffer.";
    software->destruction_callback(software->user_data);
    return nullptr;
  }
  return surface;
}

sk_sp<SkSurface> MakeSkSurfaceFromBackingStore(
    GrContext* context,
    const FlutterBackingStoreConfig& config,
    const FlutterVulkanImage* image) {
  FML_DCHECK(false) << "surface_from_image called";
  return nullptr;
}

sk_sp<SkSurface> MakeSkSurfaceFromBackingStore(
    GrContext* context,
    const FlutterBackingStoreConfig& config,
    const FlutterVulkanSwapchain* swapchain) {
  FML_DCHECK(false) << "surface_from_swapchain called";
  return nullptr;
}

std::unique_ptr<flutter::EmbedderRenderTarget> CreateEmbedderRenderTarget(
    FlutterBackingStoreCreateCallback create_backing_store_callback,
    FlutterBackingStoreCollectCallback collect_backing_store_callback,
    void* user_data,
    const FlutterBackingStoreConfig& config,
    GrContext* context) {
  FlutterBackingStore backing_store = {};
  backing_store.struct_size = sizeof(backing_store);

  {
    TRACE_EVENT0("flutter", "FlutterCompositorCreateBackingStore");
    if (!create_backing_store_callback(&config, &backing_store, user_data)) {
      FML_LOG(ERROR) << "Could not create the embedder backing store.";
      return nullptr;
    }
  }

  if (backing_store.struct_size != sizeof(backing_store)) {
    FML_LOG(ERROR) << "Embedder modified the backing store struct size.";
    return nullptr;
  }

  // In case we return early without creating an embedder render target, the
  // embedder has still given us ownership of its baton which we must return
  // back to it. If this method is successful, the closure is released when the
  // render target is eventually released.
  fml::ScopedCleanupClosure collect_callback(
      [collect_backing_store_callback, backing_store, user_data]() {
        TRACE_EVENT0("flutter", "FlutterCompositorCollectBackingStore");
        collect_backing_store_callback(&backing_store, user_data);
      });

  sk_sp<SkSurface> render_surface;
  switch (backing_store.type) {
    case kFlutterBackingStoreTypeSoftware:
      render_surface = MakeSkSurfaceFromBackingStore(context, config,
                                                     &backing_store.software);
      break;
    case kFlutterBackingStoreTypeVulkan:
      switch (backing_store.vulkan.type) {
        case kFlutterVulkanTargetTypeImage:
          render_surface = MakeSkSurfaceFromBackingStore(
              context, config, &backing_store.vulkan.image);
          break;
        case kFlutterVulkanTargetTypeSwapchain:
          render_surface = MakeSkSurfaceFromBackingStore(
              context, config, &backing_store.vulkan.swapchain);
          break;
      }
      break;
    case kFlutterBackingStoreTypeOpenGL:
      FML_NOTIMPLEMENTED();
      break;
  };

  if (!render_surface) {
    FML_LOG(ERROR) << "Could not create a surface from an embedder provided "
                      "render target.";
    return nullptr;
  }

  return std::make_unique<flutter::EmbedderRenderTarget>(
      backing_store, std::move(render_surface), collect_callback.Release());
}

fml::RefPtr<flutter::PlatformMessage> MakeLocalizationPlatformMessage(
    const fuchsia::intl::Profile& intl_profile) {
  return fml::MakeRefCounted<flutter::PlatformMessage>(
      "flutter/localization", MakeLocalizationPlatformMessageData(intl_profile),
      nullptr);
}

}  // end namespace

Engine::Engine(Delegate& delegate,
               std::string thread_label,
               std::shared_ptr<sys::ServiceDirectory> svc,
               std::shared_ptr<sys::ServiceDirectory> runner_services,
               flutter::Settings settings,
               fml::RefPtr<const flutter::DartSnapshot> isolate_snapshot,
               fuchsia::ui::views::ViewToken view_token,
               UniqueFDIONS fdio_ns,
               fidl::InterfaceRequest<fuchsia::io::Directory> directory_request)
    : delegate_(delegate),
      thread_host_(thread_label + ".",
                   flutter::ThreadHost::Type::IO |
                       flutter::ThreadHost::Type::UI |
                       flutter::ThreadHost::Type::GPU),
      weak_factory_(this) {
  fml::MessageLoop::EnsureInitializedForCurrentThread();

  // Get the task runners from the managed threads. The current thread will be
  // used as the "platform" thread.
  const flutter::TaskRunners task_runners(
      thread_label,                                    // Dart thread labels
      fml::MessageLoop::GetCurrent().GetTaskRunner(),  // platform
      thread_host_.gpu_thread->GetTaskRunner(),        // gpu
      thread_host_.ui_thread->GetTaskRunner(),         // ui
      thread_host_.io_thread->GetTaskRunner()          // io
  );
  UpdateNativeThreadLabelNames(thread_label, task_runners);

  // Compositor errors can occur on the platform thread or the GPU thread, but
  // we must always terminate ourselves on the platform thread.
  //
  // This handles any of those errors gracefully by terminating the Engine on
  // the platform thread regardless.
  fml::closure compositor_error_callback =
      [platform_runner = task_runners.GetPlatformTaskRunner(),
       weak = weak_factory_.GetWeakPtr()]() {
        fml::TaskRunner::RunNowOrPostTask(platform_runner, [weak]() {
          if (weak) {
            weak->Terminate();
          }
        });
      };

  // Create the Compositor; all of the other objects rely on it.
  scenic_compositor_ = std::make_unique<ScenicCompositorConnection>(
      thread_label, runner_services, task_runners.GetPlatformTaskRunner(),
      task_runners.GetGPUTaskRunner(), std::move(view_token),
      std::move(compositor_error_callback));

  FlutterCompositor compositor_callbacks =
      scenic_compositor_->GetCompositorCallbacks();

  flutter::EmbedderExternalViewEmbedder::CreateRenderTargetCallback
      create_render_target_callback =
          [compositor_callbacks](GrContext* context,
                                 const FlutterBackingStoreConfig& config) {
            return CreateEmbedderRenderTarget(
                compositor_callbacks.create_backing_store_callback,
                compositor_callbacks.collect_backing_store_callback,
                compositor_callbacks.user_data, config, context);
          };

  flutter::EmbedderExternalViewEmbedder::PresentCallback present_callback =
      [compositor_callbacks](const auto& layers) {
        return compositor_callbacks.present_layers_callback(
            const_cast<const FlutterLayer**>(layers.data()), layers.size(),
            compositor_callbacks.user_data);
      };

  // Create the ExternalViewEmbedder which will serve as the Rasterizer's
  // interface to the Compositor.
  auto external_view_embedder =
      std::make_unique<flutter::EmbedderExternalViewEmbedder>(
          create_render_target_callback, present_callback);

  flutter::PlatformViewEmbedder::PlatformDispatchTable platform_dispatch_table =
      {
          // UpdateSemanticsNodesCallback update_semantics_nodes_callback;  //
          // optional
          // UpdateSemanticsCustomActionsCallback
          // update_semantics_custom_actions_callback;  // optional
          // PlatformMessageResponseCallback platform_message_response_callback;
          // // optional
          // VsyncWaiterEmbedder::VsyncCallback vsync_callback;  // optional
      };

  flutter::EmbedderSurfaceVulkan::VulkanDispatchTable vulkan_dispatch_table = {
      // std::function<const char*()> get_extension_name;  // required
      // std::function<uint32_t()> get_skia_extension_name;  // required
      // std::function<SkISize()> get_size;  // required
      // std::function<VkSurfaceKHR(VulkanProcTable&, const VkInstance&)>
      // create_surface_handle;  // required
  };

  // Setup the callback that will instantiate the platform view.
  flutter::Shell::CreateCallback<flutter::PlatformView>
      on_create_platform_view = fml::MakeCopyable(
          [vulkan_dispatch_table, platform_dispatch_table,
           external_view_embedder = std::move(external_view_embedder)](
              flutter::Shell& shell) mutable {
            return std::make_unique<flutter::PlatformViewEmbedder>(
                shell,                               // delegate
                shell.GetTaskRunners(),              // task runners
                std::move(vulkan_dispatch_table),    // vulkan dispatch table
                std::move(platform_dispatch_table),  // platform dispatch table
                std::move(external_view_embedder)    // external view embedder
            );
          });

  // Setup the callback that will instantiate the rasterizer.
  flutter::Shell::CreateCallback<flutter::Rasterizer> on_create_rasterizer =
      [](flutter::Shell& shell) {
        return std::make_unique<flutter::Rasterizer>(shell,
                                                     shell.GetTaskRunners());
      };

  settings.verbose_logging = true;

  settings.advisory_script_uri = thread_label;

  settings.advisory_script_entrypoint = thread_label;

  settings.root_isolate_create_callback = std::bind(
      [thread_label = thread_label, weak = weak_factory_.GetWeakPtr()]() {
        if (weak) {
          weak->OnMainIsolateStart(std::move(thread_label));
        }
      });

  settings.root_isolate_shutdown_callback =
      std::bind([thread_label = thread_label, weak = weak_factory_.GetWeakPtr(),
                 runner = task_runners.GetPlatformTaskRunner()]() {
        runner->PostTask(
            [thread_label = std::move(thread_label), weak = std::move(weak)] {
              if (weak) {
                weak->OnMainIsolateShutdown(std::move(thread_label));
              }
            });
      });

  auto vm = flutter::DartVMRef::Create(settings);

  if (!isolate_snapshot) {
    isolate_snapshot = vm->GetVMData()->GetIsolateSnapshot();
  }

  {
    TRACE_EVENT0("flutter", "CreateShell");
    shell_ = flutter::Shell::Create(
        task_runners,                 // host task runners
        flutter::WindowData(),        // default window data
        settings,                     // shell launch settings
        std::move(isolate_snapshot),  // isolate snapshot
        on_create_platform_view,      // platform view create callback
        on_create_rasterizer,         // rasterizer create callback
        std::move(vm)                 // vm reference
    );
  }

  if (!shell_) {
    FML_LOG(ERROR) << "Could not launch the shell.";
    return;
  }

  // Shell has been created. Before we run the engine, setup the isolate
  // configurator.
  {
    fuchsia::sys::EnvironmentPtr environment;
    svc->Connect(environment.NewRequest());
    isolate_configurator_ = std::make_unique<IsolateConfigurator>(
        std::move(fdio_ns),              //
        std::move(environment),          //
        directory_request.TakeChannel()  //
    );
  }

  // Connect to the intl property provider.  If the connection fails, the
  // initialization of the engine will simply proceed, printing a warning
  // message.  The engine will be fully functional, except that the user's
  // locale preferences would not be communicated to flutter engine.
  {
    intl_property_provider_.set_error_handler([](zx_status_t status) {
      FML_LOG(WARNING) << "Failed to connect to "
                       << fuchsia::intl::PropertyProvider::Name_ << ": "
                       << zx_status_get_string(status)
                       << " This is not a fatal error, but the user locale "
                       << " preferences will not be forwarded to flutter apps";
    });

    // Note that we're using the runner's services, not the component's.
    // Flutter locales should be updated regardless of whether the component has
    // direct access to the fuchsia.intl.PropertyProvider service.
    ZX_ASSERT(runner_services->Connect(intl_property_provider_.NewRequest()) ==
              ZX_OK);

    auto get_profile_callback = [flutter_runner_engine =
                                     weak_factory_.GetWeakPtr()](
                                    const fuchsia::intl::Profile& profile) {
      if (!flutter_runner_engine) {
        return;
      }
      if (!profile.has_locales()) {
        FML_LOG(WARNING) << "Got intl Profile without locales";
      }
      auto message = MakeLocalizationPlatformMessage(profile);
      FML_VLOG(-1) << "Sending LocalizationPlatformMessage";
      flutter_runner_engine->shell_->GetPlatformView()->DispatchPlatformMessage(
          message);
    };

    FML_VLOG(-1) << "Requesting intl Profile";

    // Make the initial request
    intl_property_provider_->GetProfile(get_profile_callback);

    // And register for changes
    intl_property_provider_.events().OnChange = [this, runner_services,
                                                 get_profile_callback]() {
      FML_VLOG(-1) << fuchsia::intl::PropertyProvider::Name_ << ": OnChange";
      runner_services->Connect(intl_property_provider_.NewRequest());
      intl_property_provider_->GetProfile(get_profile_callback);
    };
  }

  // Connect to the system font provider.
  fuchsia::fonts::ProviderSyncPtr sync_font_provider;
  svc->Connect(sync_font_provider.NewRequest());

  // This platform does not get a separate surface platform view creation
  // notification. Fire one eagerly.
  // shell_->GetPlatformView()->NotifyCreated();

  // Launch the engine in the appropriate configuration.
  auto run_configuration = flutter::RunConfiguration::InferFromSettings(
      settings, task_runners.GetIOTaskRunner());

  auto on_run_failure = [weak = weak_factory_.GetWeakPtr()]() {
    // The engine could have been killed by the caller right after the
    // constructor was called but before it could run on the UI thread.
    if (weak) {
      weak->Terminate();
    }
  };

  shell_->GetTaskRunners().GetUITaskRunner()->PostTask(
      fml::MakeCopyable([engine = shell_->GetEngine(),                        //
                         run_configuration = std::move(run_configuration),    //
                         sync_font_provider = std::move(sync_font_provider),  //
                         on_run_failure                                       //
  ]() mutable {
        if (!engine) {
          return;
        }

        // Set default font manager.
        engine->GetFontCollection().GetFontCollection()->SetDefaultFontManager(
            SkFontMgr_New_Fuchsia(std::move(sync_font_provider)));

        if (engine->Run(std::move(run_configuration)) ==
            flutter::Engine::RunStatus::Failure) {
          on_run_failure();
        }
      }));
}

Engine::~Engine() {
  shell_.reset();
}

std::pair<bool, uint32_t> Engine::GetEngineReturnCode() const {
  std::pair<bool, uint32_t> code(false, 0);
  if (!shell_) {
    return code;
  }
  fml::AutoResetWaitableEvent latch;
  fml::TaskRunner::RunNowOrPostTask(
      shell_->GetTaskRunners().GetUITaskRunner(),
      [&latch, &code, engine = shell_->GetEngine()]() {
        if (engine) {
          code = engine->GetUIIsolateReturnCode();
        }
        latch.Signal();
      });
  latch.Wait();
  return code;
}

static void CreateCompilationTrace(Dart_Isolate isolate) {
  Dart_EnterIsolate(isolate);

  {
    Dart_EnterScope();
    uint8_t* trace = nullptr;
    intptr_t trace_length = 0;
    Dart_Handle result = Dart_SaveCompilationTrace(&trace, &trace_length);
    tonic::LogIfError(result);

    for (intptr_t start = 0; start < trace_length;) {
      intptr_t end = start;
      while ((end < trace_length) && trace[end] != '\n')
        end++;

      std::string line(reinterpret_cast<char*>(&trace[start]), end - start);
      FML_LOG(INFO) << "compilation-trace: " << line;

      start = end + 1;
    }

    Dart_ExitScope();
  }

  // Re-enter Dart scope to release the compilation trace's memory.

  {
    Dart_EnterScope();
    uint8_t* feedback = nullptr;
    intptr_t feedback_length = 0;
    Dart_Handle result = Dart_SaveTypeFeedback(&feedback, &feedback_length);
    tonic::LogIfError(result);
    const std::string kTypeFeedbackFile = "/data/dart_type_feedback.bin";
    if (dart_utils::WriteFile(kTypeFeedbackFile,
                              reinterpret_cast<const char*>(feedback),
                              feedback_length)) {
      FML_LOG(INFO) << "Dart type feedback written to " << kTypeFeedbackFile;
    } else {
      FML_LOG(ERROR) << "Could not write Dart type feedback to "
                     << kTypeFeedbackFile;
    }
    Dart_ExitScope();
  }

  Dart_ExitIsolate();
}

void Engine::OnMainIsolateStart(std::string thread_label) {
  if (!isolate_configurator_ ||
      !isolate_configurator_->ConfigureCurrentIsolate()) {
    FML_LOG(ERROR) << "Could not configure some native embedder bindings for a "
                      "new root isolate.";
  }
  FML_DLOG(INFO) << "Main isolate for engine '" << thread_label
                 << "' was started.";

  const intptr_t kCompilationTraceDelayInSeconds = 0;
  if (kCompilationTraceDelayInSeconds != 0) {
    Dart_Isolate isolate = Dart_CurrentIsolate();
    FML_CHECK(isolate);
    shell_->GetTaskRunners().GetUITaskRunner()->PostDelayedTask(
        [engine = shell_->GetEngine(), isolate]() {
          if (!engine) {
            return;
          }
          CreateCompilationTrace(isolate);
        },
        fml::TimeDelta::FromSeconds(kCompilationTraceDelayInSeconds));
  }
}

void Engine::OnMainIsolateShutdown(std::string thread_label) {
  FML_DLOG(INFO) << "Main isolate for engine '" << thread_label
                 << "' shutting down.";
  Terminate();
}

void Engine::Terminate() {
  delegate_.OnEngineTerminate(this);
  // Warning. Do not do anything after this point as the delegate may have
  // collected this object.
}

#if !defined(DART_PRODUCT)
void Engine::WriteProfileToTrace() const {
  Dart_Port main_port = shell_->GetEngine()->GetUIIsolateMainPort();
  char* error = NULL;
  bool success = Dart_WriteProfileToTimeline(main_port, &error);
  if (!success) {
    FML_LOG(ERROR) << "Failed to write Dart profile to trace: " << error;
    free(error);
  }
}
#endif  // !defined(DART_PRODUCT)

}  // namespace flutter_runner
