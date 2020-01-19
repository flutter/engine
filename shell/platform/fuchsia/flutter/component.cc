// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/component.h"

#include <fcntl.h>
#include <fuchsia/fonts/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/async/cpp/task.h>
#include <lib/async/default.h>
#include <lib/fdio/directory.h>
#include <lib/fdio/io.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/trace/event.h>
#include <lib/vfs/cpp/composed_service_dir.h>
#include <lib/vfs/cpp/remote_dir.h>
#include <lib/vfs/cpp/service.h>
#include <lib/zx/time.h>
#include <zircon/status.h>
#include <zircon/types.h>

#include <utility>

#include "flutter/shell/platform/fuchsia/dart-pkg/fuchsia/sdk_ext/fuchsia.h"
#include "flutter/shell/platform/fuchsia/utils/files.h"
// TODO(dworsham)
// #include
// "flutter/shell/platform/fuchsia/utils/handle_exception.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/shell/platform/fuchsia/utils/mapped_resource.h"
#include "flutter/shell/platform/fuchsia/utils/tempfs.h"
#include "flutter/third_party/tonic/converter/dart_converter.h"
#include "flutter/third_party/tonic/logging/dart_error.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace flutter_runner {
namespace {

constexpr char kDataKey[] = "data";
constexpr char kTmpPath[] = "/tmp";
constexpr char kIcuDataPath[] = "/pkg/data/icudtl.dat";

void ConfigureCurrentIsolate(
    fidl::InterfaceHandle<fuchsia::sys::Environment> environment,
    fidl::InterfaceRequest<fuchsia::io::Directory> directory_request,
    fdio_ns_t* component_ns,
    Renderer* renderer) {
  fuchsia::dart::Initialize(std::move(environment),
                            std::move(directory_request));

  // Allow the |Renderer| to register any Dart hooks.
  renderer->ConfigureCurrentIsolate();

  // Tell dart:zircon about the FDIO namespace configured for this instance.
  Dart_Handle zircon_lib = Dart_LookupLibrary(tonic::ToDart("dart:zircon"));
  FX_CHECK(!tonic::LogIfError(zircon_lib));

  Dart_Handle namespace_type =
      Dart_GetType(zircon_lib, tonic::ToDart("_Namespace"), 0, nullptr);
  FX_CHECK(!tonic::LogIfError(namespace_type));

  Dart_Handle set_result =
      Dart_SetField(namespace_type,               //
                    tonic::ToDart("_namespace"),  //
                    tonic::ToDart(reinterpret_cast<intptr_t>(component_ns)));
  FX_CHECK(!tonic::LogIfError(set_result));

  // Grab the dart:io lib.
  Dart_Handle io_lib = Dart_LookupLibrary(tonic::ToDart("dart:io"));
  FX_CHECK(!tonic::LogIfError(io_lib));

  // Disable dart:io exit()
  Dart_Handle embedder_config_type =
      Dart_GetType(io_lib, tonic::ToDart("_EmbedderConfig"), 0, nullptr);
  FX_CHECK(!tonic::LogIfError(embedder_config_type));

  Dart_Handle set_result2 = Dart_SetField(
      embedder_config_type, tonic::ToDart("_mayExit"), Dart_False());
  FX_CHECK(!tonic::LogIfError(set_result2));

  // Tell dart:io about the FDIO namespace configured for this instance.
  Dart_Handle namespace_type2 =
      Dart_GetType(io_lib, tonic::ToDart("_Namespace"), 0, nullptr);
  FX_CHECK(!tonic::LogIfError(namespace_type2));

  Dart_Handle namespace_args[] = {
      Dart_NewInteger(reinterpret_cast<intptr_t>(component_ns)),
  };
  Dart_Handle invoke_result = Dart_Invoke(
      namespace_type2, tonic::ToDart("_setupNamespace"), 1, namespace_args);
  FX_CHECK(!tonic::LogIfError(invoke_result));
}

// TODO(dworsham)
// void CreateCompilationTrace(Dart_Isolate isolate) {
//   Dart_EnterIsolate(isolate);

//   {
//     Dart_EnterScope();
//     uint8_t* trace = nullptr;
//     intptr_t trace_length = 0;
//     Dart_Handle result = Dart_SaveCompilationTrace(&trace, &trace_length);
//     tonic::LogIfError(result);

//     for (intptr_t start = 0; start < trace_length;) {
//       intptr_t end = start;
//       while ((end < trace_length) && trace[end] != '\n')
//         end++;

//       std::string line(reinterpret_cast<char*>(&trace[start]), end - start);
//       FX_LOGF(INFO, FX_LOG_TAG, "compilation-trace: %s", line.c_str());

//       start = end + 1;
//     }

//     Dart_ExitScope();
//   }

//   // Re-enter Dart scope to release the compilation trace's memory.

//   {
//     Dart_EnterScope();
//     uint8_t* feedback = nullptr;
//     intptr_t feedback_length = 0;
//     Dart_Handle result = Dart_SaveTypeFeedback(&feedback, &feedback_length);
//     tonic::LogIfError(result);
//     const std::string kTypeFeedbackFile = "/data/dart_type_feedback.bin";
//     if (fx::WriteFile(kTypeFeedbackFile,
//                               reinterpret_cast<const char*>(feedback),
//                               feedback_length)) {
//       FX_LOGF(INFO, FX_LOG_TAG, "Dart type feedback written to %s",
//       kTypeFeedbackFile.c_str());
//     } else {
//       FX_LOGF(ERROR, FX_LOG_TAG, "Could not write Dart type feedback to %s",
//       kTypeFeedbackFile.c_str());
//     }
//     Dart_ExitScope();
//   }

//   Dart_ExitIsolate();
// }

}  // namespace

std::unique_ptr<Component, Component::Deleter> Component::Create(
    Context component_context) {
  // Destroy the component on its platform thread, synchronously.
  Component::Deleter deleter = [](Component* component) {
    ThreadHost component_threads = std::move(component->threads_);

    // Destroy the component and the thread on the appropriate thread.  There
    // is no need for a |TaskBarrier| because the |Join| serves as one.
    async::PostTask(component_threads.platform_thread.dispatcher(),
                    [component]() { delete component; });
    component_threads.platform_thread.Join();  // This will post a Quit() task.
  };

  // Create the component on its new platform thread, synchronously.
  std::unique_ptr<Component, Component::Deleter> component;
  ThreadHost component_threads(component_context.debug_label);
  component_threads.platform_thread.TaskBarrier([&]() {
    component = std::unique_ptr<Component, Component::Deleter>(
        new Component(std::move(component_context),
                      std::move(component_threads)),
        std::move(deleter));
  });

  return component;
}

Component::Component(Context component_context, ThreadHost threads)
    : threads_(std::move(threads)),
      controller_binding_(this,
                          std::move(component_context.controller_request)),
      component_url_(std::move(component_context.component_url)),
      debug_label_(std::move(component_context.debug_label)),
      incoming_services_(std::move(component_context.incoming_services)),
      renderer_(component_context.renderer_factory_callback(Renderer::Context{
          .dispatch_table = Renderer::DispatchTable{
              .window_metrics_callback =
                  [this](const FlutterWindowMetricsEvent* event) {
                    auto status =
                        FlutterEngineSendWindowMetricsEvent(engine_, event);
                    FX_DCHECK(status == kSuccess);
                  },
              .pointer_event_callback =
                  [this](const FlutterPointerEvent* events,
                         size_t events_count) {
                    auto status = FlutterEngineSendPointerEvent(engine_, events,
                                                                events_count);
                    FX_DCHECK(status == kSuccess);
                  },
              .platform_message_callback =
                  [this](const FlutterPlatformMessage* message) {
                    auto status =
                        FlutterEngineSendPlatformMessage(engine_, message);
                    FX_DCHECK(status == kSuccess);
                  },
              .platform_message_response_callback =
                  [this](const FlutterPlatformMessageResponseHandle* response) {
                    auto status = FlutterEngineSendPlatformMessageResponse(
                        engine_, response, nullptr, 0);
                    FX_DCHECK(status == kSuccess);
                  },
              .update_semantics_enabled_callback =
                  [this](bool enabled) {
                    auto status =
                        FlutterEngineUpdateSemanticsEnabled(engine_, enabled);
                    FX_DCHECK(status == kSuccess);
                  },
              .update_accessibility_features_callback =
                  [this](FlutterAccessibilityFeature features) {
                    auto status = FlutterEngineUpdateAccessibilityFeatures(
                        engine_, features);
                    FX_DCHECK(status == kSuccess);
                  },
              .semantics_action_callback =
                  [this](uint64_t id,
                         FlutterSemanticsAction action,
                         const uint8_t* data,
                         size_t data_length) {
                    auto status = FlutterEngineDispatchSemanticsAction(
                        engine_, id, action, data, data_length);
                    FX_DCHECK(status == kSuccess);
                  },
              .get_current_time_callback = FlutterEngineGetCurrentTime,
              .vsync_callback =
                  [this](intptr_t baton,
                         uint64_t previous_vsync,
                         uint64_t next_vsync) {
                    auto status = FlutterEngineOnVsync(
                        engine_, baton, previous_vsync, next_vsync);
                    FX_DCHECK(status == kSuccess);
                  },
              .error_callback = std::bind(&Component::Kill, this),
          },
          .debug_label = debug_label_,
          .incoming_services = incoming_services_,
          .input_dispatcher = threads_.platform_thread.dispatcher(),
          .raster_dispatcher = threads_.gpu_thread.dispatcher(),
      })),
      termination_callback_(std::move(component_context.termination_callback)) {
  flutter_directory_.set_error_handler([this](zx_status_t status) {
    FX_LOG(ERROR) << "Interface error for fuchsia::io::Directory: "
                  << zx_status_get_string(status);
    Kill();
  });
  controller_binding_.set_error_handler([this](zx_status_t status) {
    FX_LOG(ERROR)
        << "Interface error (binding) for fuchsia::sys::ComponentController: "
        << zx_status_get_string(status);
    Kill();
  });

  // Create a new namespace to serve as / for this component and setup this
  // component's /tmp to map to the process-local memfs.
  zx_status_t create_status = fdio_ns_create(&component_namespace_);
  FX_DCHECK(create_status == ZX_OK);
  fx::RunnerTemp::SetupComponent(component_namespace_);

  SetupOutgoingDirectory();
  ParseStartupInfo(std::move(component_context.startup_info));
  LaunchFlutter();
}

Component::~Component() {
  FX_DCHECK(engine_ == nullptr);  // Engine should not be active.

  if (assets_directory_ != -1) {
    close(assets_directory_);
  }

  zx_status_t status = fdio_ns_destroy(component_namespace_);
  FX_DCHECK(status == ZX_OK);
}

#if !defined(DART_PRODUCT)
void Component::WriteProfileToTrace() const {
  // Dump the trace on the platform thread, synchronously.
  threads_.platform_thread.TaskBarrier([]() {
    // TODO(dworsham)
    // Dart_Port main_port = shell_->GetEngine()->GetUIIsolateMainPort();
    // char* error = NULL;
    // bool success = Dart_WriteProfileToTimeline(main_port, &error);
    // if (!success) {
    //   FX_LOGF(ERROR, FX_LOG_TAG, "Failed to write Dart profile for %s to
    //   trace: %s", component_url_.c_str(), error); free(error);
    // }
  });
}

#endif  // !defined(DART_PRODUCT)

void Component::Kill() {
  TerminateFlutter();

  controller_binding_.events().OnTerminated(
      last_return_code_.second, fuchsia::sys::TerminationReason::EXITED);

  // WARNING: May collect `this`.
  termination_callback_(this);
}

void Component::Detach() {
  controller_binding_.set_error_handler(nullptr);
}

void Component::SetupOutgoingDirectory() {
  // Check if client is servicing the component directory before adding debug
  // directories to the outgoing directory.
  // TODO(anmittal): when fixing enumeration using new c++ vfs, make sure that
  // flutter_public_dir is only accessed once we receive OnOpen Event.
  // That will prevent FL-175 for public directory.
  flutter_directory_.events().OnOpen =
      [this](zx_status_t status, std::unique_ptr<fuchsia::io::NodeInfo> info) {
        // component_directory.events().OnOpen = nullptr;  // TODO(dworsham)
        if (status != ZX_OK) {
          FX_LOG(ERROR) << "Could not bind out directory for flutter app ("
                        << debug_label_
                        << "): " << zx_status_get_string(status);
          return;
        }
        const char* other_dirs[] = {"debug", "ctrl", "diagnostics"};
        // add other directories as RemoteDirs.
        for (auto& dir_str : other_dirs) {
          fidl::InterfaceHandle<fuchsia::io::Directory> dir;
          auto request = dir.NewRequest().TakeChannel();
          auto status = fdio_service_connect_at(
              flutter_directory_.channel().get(), dir_str, request.release());
          if (status == ZX_OK) {
            outgoing_directory_.AddEntry(
                dir_str, std::make_unique<vfs::RemoteDir>(dir.TakeChannel()));
          } else {
            FX_LOG(ERROR) << "Could not add out directory entry '" << dir_str
                          << "' for flutter app (" << debug_label_
                          << "): " << zx_status_get_string(status);
          }
        }
      };

  // Create the service directory that the Flutter Engine will use to expose its
  // outgoing services.
  fidl::InterfaceHandle<fuchsia::io::Directory> flutter_service_dir;
  fdio_service_connect_at(
      flutter_directory_.channel().get(), "svc",
      flutter_service_dir.NewRequest().TakeChannel().release());

  // Overlay the |Renderer|'s outgoing services on top of the Flutter Engine's
  // services.
  auto composed_service_dir = std::make_unique<vfs::ComposedServiceDir>();
  composed_service_dir->set_fallback(std::move(flutter_service_dir));
  renderer_->BindServices(
      [&composed_service_dir](const std::string& name,
                              std::unique_ptr<vfs::Service> service) {
        composed_service_dir->AddService(name, std::move(service));
      });

  // Provide the virtual directory as the |Component|'s outgoing services.
  outgoing_directory_.AddEntry("svc", std::move(composed_service_dir));
}

void Component::ParseStartupInfo(fuchsia::sys::StartupInfo startup_info) {
  auto& launch_info = startup_info.launch_info;

  // StartupInfo: program_metadata non-optional.
  for (size_t i = 0; i < startup_info.program_metadata->size(); ++i) {
    auto pg = startup_info.program_metadata->at(i);
    if (pg.key.compare(kDataKey) == 0) {
      assets_path_ = "pkg/" + pg.value;
    }
  }
  if (assets_path_.empty()) {
    FX_LOG(ERROR) << "Could not find a package data directory for "
                  << component_url_;
    return;
  }

  // StartupInfo::flat_namespace optional.
  for (size_t i = 0; i < startup_info.flat_namespace.paths.size(); ++i) {
    const auto& path = startup_info.flat_namespace.paths.at(i);
    if (path == kTmpPath) {
      continue;
    }

    zx_handle_t dir_handle =
        startup_info.flat_namespace.directories.at(i).release();
    if (fdio_ns_bind(component_namespace_, path.data(), dir_handle) != ZX_OK) {
      FX_DLOG(ERROR) << "Could not bind path to namespace: " << path;
      zx_handle_close(dir_handle);
    }
  }

  // LaunchInfo::arguments optional.
  if (launch_info.arguments) {
    for (auto& argument : launch_info.arguments.value()) {
      dart_entrypoint_args_.push_back(argument);
    }
  }

  // TODO: LaunchInfo::out.

  // TODO: LaunchInfo::err.

  // LaunchInfo::directory_request optional.
  if (launch_info.directory_request) {
    outgoing_directory_.Serve(fuchsia::io::OPEN_RIGHT_READABLE |
                                  fuchsia::io::OPEN_RIGHT_WRITABLE |
                                  fuchsia::io::OPEN_FLAG_DIRECTORY,
                              std::move(launch_info.directory_request));
  }

  // TODO: LaunchInfo::additional_services optional.

  // Open the assets directory in order to load AOT or JIT snapshots.
  {
    int component_directory = fdio_ns_opendir(component_namespace_);
    if (component_directory != -1) {
      assets_directory_ = openat(component_directory, assets_path_.c_str(),
                                 O_RDONLY | O_DIRECTORY);
      FX_DCHECK(assets_directory_ != -1)
          << "Failed to open component assets directory.";
    } else {
      FX_DCHECK(false) << "Failed to open root component directory.";
    }
    close(component_directory);
  }

  // Load the snapshots.
  if (FlutterEngineRunsAOTCompiledDartCode() &&
      aot_snapshot_.Load(assets_directory_, "app_aot_snapshot.so")) {
    const uint8_t* vm_data = aot_snapshot_.VmData();
    const uint8_t* vm_instructions = aot_snapshot_.VmInstrs();
    const uint8_t* isolate_data = aot_snapshot_.IsolateData();
    const uint8_t* isolate_instructions = aot_snapshot_.IsolateInstrs();
    if (vm_data == nullptr || vm_instructions == nullptr ||
        isolate_data == nullptr || isolate_instructions == nullptr) {
      FX_LOG(FATAL) << "ELF snapshot missing AOT symbols.";
    }
    vm_snapshot_data_mapping_ = fx::MappedResource(vm_data, 0);
    vm_snapshot_instructions_mapping_ = fx::MappedResource(vm_instructions, 0);
    isolate_snapshot_data_mapping_ = fx::MappedResource(isolate_data, 0);
    isolate_snapshot_instructions_mapping_ =
        fx::MappedResource(isolate_instructions, 0);
  } else {
    // Check if we can use the snapshot with the framework already loaded.
    std::string runner_framework;
    std::string app_framework;
    std::string framework_prefix = "";
    if (fx::ReadFileToString("/pkg/data/runner.frameworkversion",
                             &runner_framework) &&
        fx::ReadFileToStringAt(assets_directory_, "app.frameworkversion",
                               &app_framework) &&
        (runner_framework.compare(app_framework) == 0)) {
      FX_LOG(INFO) << "Using snapshot with framework for " << component_url_;

      framework_prefix = "framework_";
    } else {
      FX_LOG(INFO) << "Using snapshot without framework for " << component_url_;
    }
    vm_snapshot_data_mapping_ = fx::MappedResource::MakeFileMapping(
        nullptr, "/pkg/data/" + framework_prefix + "vm_snapshot_data.bin");
    isolate_snapshot_data_mapping_ = fx::MappedResource::MakeFileMapping(
        nullptr,
        "/pkg/data/" + framework_prefix + "isolate_core_snapshot_data.bin");
    vm_snapshot_instructions_mapping_ = fx::MappedResource::MakeFileMapping(
        nullptr, "/pkg/data/vm_snapshot_instructions.bin", true);
    isolate_snapshot_instructions_mapping_ =
        fx::MappedResource::MakeFileMapping(
            nullptr, "/pkg/data/isolate_core_snapshot_instructions.bin", true);
  }

  if (!FlutterEngineRunsAOTCompiledDartCode()) {
    // The interpreter is enabled unconditionally in JIT mode. If an app is
    // built for debugging (that is, with no bytecode), the VM will fall back on
    // ASTs.
    dart_flags_.push_back("--enable_interpreter");
  }

  // Scale back CPU profiler sampling period on ARM64 to avoid overloading
  // the tracing engine.
#if defined(__aarch64__)
  dart_flags_.push_back("--profile_period=10000");
#endif  // defined(__aarch64__)

  // TODO(FL-117): Re-enable causal async stack traces when this issue is
  // addressed.
  dart_flags_.push_back("--no_causal_async_stacks");

  // Disable code collection as it interferes with JIT code warmup
  // by decreasing usage counters and flushing code which is still useful.
  dart_flags_.push_back("--no-collect_code");

  // Don't collect CPU samples from Dart VM C++ code.
  dart_flags_.push_back("--no_profile_vm");
}

void Component::LaunchFlutter() {
  FlutterTaskRunnerDescription embedder_platform_task_runner = {
      .struct_size = sizeof(FlutterTaskRunnerDescription),
      .user_data = this,
      .runs_task_on_current_thread_callback = [](void* user_data) -> bool {
        Component* component = static_cast<Component*>(user_data);
        FX_DCHECK(component);

        return component->threads_.platform_thread.dispatcher() ==
               async_get_default_dispatcher();
      },
      .post_task_callback =
          [](FlutterTask task, uint64_t target_time, void* user_data) {
            Component* component = static_cast<Component*>(user_data);
            FX_DCHECK(component);

            async::PostTaskForTime(
                component->threads_.platform_thread.dispatcher(),
                [component, task = std::move(task)]() {
                  FlutterEngineRunTask(component->engine_, &task);
                },
                zx::time(target_time));  // TODO(dworsham): Sign mismatch here
                                         // seems dangerous
          },
      .identifier = 0,
  };
  FlutterTaskRunnerDescription embedder_render_task_runner = {
      .struct_size = sizeof(FlutterTaskRunnerDescription),
      .user_data = this,
      .runs_task_on_current_thread_callback = [](void* user_data) -> bool {
        Component* component = static_cast<Component*>(user_data);
        FX_DCHECK(component);

        return component->threads_.gpu_thread.dispatcher() ==
               async_get_default_dispatcher();
      },
      .post_task_callback =
          [](FlutterTask task, uint64_t target_time, void* user_data) {
            Component* component = static_cast<Component*>(user_data);
            FX_DCHECK(component);

            async::PostTaskForTime(
                component->threads_.gpu_thread.dispatcher(),
                [component, task = std::move(task)]() {
                  FlutterEngineRunTask(component->engine_, &task);
                },
                zx::time(target_time));  // TODO(dworsham): Sign mismatch here
                                         // seems dangerous
          },
      .identifier = 1,
  };

  FlutterCustomTaskRunners embedder_custom_task_runners = {
      .struct_size = sizeof(FlutterCustomTaskRunners),
      .platform_task_runner = &embedder_platform_task_runner,
      .render_task_runner = &embedder_render_task_runner,
      .ui_task_runner = nullptr,  // TODO(dworsham)
      .io_task_runner = nullptr,  // TODO(dworsham)
  };

  FlutterCompositor embedder_compositor_callbacks = {
      .struct_size = sizeof(FlutterCompositor),
      .user_data = renderer_.get(),
      .create_backing_store_callback =
          [](const FlutterBackingStoreConfig* layer_config,
             FlutterBackingStore* backing_store_out, void* user_data) {
            TRACE_DURATION("flutter", "FlutterCompositorCreateBackingStore");
            Renderer* renderer = static_cast<Renderer*>(user_data);
            FX_DCHECK(renderer);

            return renderer->CreateBackingStore(layer_config,
                                                backing_store_out);
          },
      .collect_backing_store_callback =
          [](const FlutterBackingStore* backing_store, void* user_data) {
            TRACE_DURATION("flutter", "FlutterCompositorCollectBackingStore");
            Renderer* renderer = static_cast<Renderer*>(user_data);
            FX_DCHECK(renderer);

            return renderer->CollectBackingStore(backing_store);
          },
      .present_layers_callback =
          [](const FlutterLayer** layers, size_t layer_count, void* user_data) {
            TRACE_DURATION("flutter", "FlutterCompositorPresentLayers");
            Renderer* renderer = static_cast<Renderer*>(user_data);
            FX_DCHECK(renderer);

            return renderer->PresentLayers(layers, layer_count);
          },
      .backing_store_available_callback =
          [](const FlutterBackingStore* backing_store, void* user_data) {
            TRACE_DURATION("flutter",
                           "FlutterCompositorIsBackingStoreAvailable");
            Renderer* renderer = static_cast<Renderer*>(user_data);
            FX_DCHECK(renderer);

            return renderer->IsBackingStoreAvailable(backing_store);
          }};

  FlutterProjectArgs embedder_args;
  std::string log_tag = debug_label_ + "(flutter)";
  std::vector<const char*> dart_flags_holder(dart_flags_.size());
  std::vector<const char*> dart_entrypoint_args_holder(
      dart_entrypoint_args_.size());
  embedder_args.struct_size = sizeof(FlutterProjectArgs);
  embedder_args.assets_path = assets_path_.c_str();
  embedder_args.assets_dir = assets_directory_;
  embedder_args.main_path__unused__ = nullptr;
  embedder_args.packages_path__unused__ = nullptr;
  embedder_args.icu_data_path = kIcuDataPath;
  embedder_args.command_line_argc = 0;
  embedder_args.command_line_argv = nullptr;
  embedder_args.platform_message_callback =
      [](const FlutterPlatformMessage* message, void* user_data) {
        Component* component = static_cast<Component*>(user_data);
        FX_DCHECK(component);

        component->renderer_->PlatformMessageResponse(message);
      };
  embedder_args.application_kernel_list_asset =
      "app.dilplist";  // TODO(dworsham)
  embedder_args.vm_snapshot_data = vm_snapshot_data_mapping_.address();
  embedder_args.vm_snapshot_data_size = vm_snapshot_data_mapping_.size();
  embedder_args.vm_snapshot_instructions =
      vm_snapshot_instructions_mapping_.address();
  embedder_args.vm_snapshot_instructions_size =
      vm_snapshot_instructions_mapping_.size();
  embedder_args.isolate_snapshot_data =
      isolate_snapshot_data_mapping_.address();
  embedder_args.isolate_snapshot_data_size =
      isolate_snapshot_data_mapping_.size();
  embedder_args.isolate_snapshot_instructions =
      isolate_snapshot_instructions_mapping_.address();
  embedder_args.isolate_snapshot_instructions_size =
      isolate_snapshot_instructions_mapping_.size();
  embedder_args.dart_flags_count = 0;
  for (auto& flag : dart_flags_) {
    dart_flags_holder[embedder_args.dart_flags_count++] = flag.c_str();
  }
  embedder_args.dart_entrypoint_args = dart_entrypoint_args_holder.data();
  embedder_args.dart_entrypoint_args_count = 0;
  for (auto& argument : dart_entrypoint_args_) {
    dart_entrypoint_args_holder[embedder_args.dart_entrypoint_args_count++] =
        argument.c_str();
  }
  embedder_args.dart_entrypoint_args = dart_entrypoint_args_holder.data();
  embedder_args.trace_skia = true;  // This controls whether category "skia"
                                    // trace events are enabled.
  // No asserts in debug or release product.
  // No asserts in release with flutter_profile=true (non-product)
  // Yes asserts in non-product debug.
#if !defined(DART_PRODUCT) && (!defined(FLUTTER_PROFILE) || !defined(NDEBUG))
  // Debug mode
  embedder_args.disable_dart_asserts = false;
#else
  // Release mode
  embedder_args.disable_dart_asserts = true;
#endif
  embedder_args.advisory_script_uri = debug_label_.c_str();
  embedder_args.advisory_script_entrypoint = debug_label_.c_str();
#if defined(DART_PRODUCT)
  embedder_args.enable_observatory = false;
#else
  embedder_args.enable_observatory = true;

  // TODO(cbracken): pass this in as a param to allow 0.0.0.0, ::1, etc.
  embedder_args.observatory_host = "127.0.0.1";
  embedder_args.observatory_port = 0;
#endif
  embedder_args.root_isolate_create_callback = [](void* user_data) {
    Component* component = static_cast<Component*>(user_data);
    FX_DCHECK(component);

    FX_DLOG(INFO) << "Main isolate for engine '" << component->debug_label_
                  << "' was started.";
    fidl::InterfaceHandle<fuchsia::sys::Environment> environment;
    // TODO(dworsham): This was component_incoming_services_.  Is that OK?
    // The component gets launched in the runner's environment so this should
    // be equivalent...except authors of Dart components no longer have to
    // include fuchsia.sys.Environment in their cmx for....reasons.
    component->incoming_services_->Connect(environment.NewRequest());
    ConfigureCurrentIsolate(
        std::move(environment), component->flutter_directory_.NewRequest(),
        component->component_namespace_, component->renderer_.get());
    // TODO(dworsham)
    // const intptr_t kCompilationTraceDelayInSeconds = 0;
    // if (kCompilationTraceDelayInSeconds != 0) {
    //   Dart_Isolate isolate = Dart_CurrentIsolate();
    //   FX_CHECK(isolate);
    //   shell_->GetTaskRunners().GetUITaskRunner()->PostDelayedTask(
    //       [engine = shell_->GetEngine(), isolate]() {
    //         if (!engine) {
    //           return;
    //         }
    //         CreateCompilationTrace(isolate);
    //       },
    //       kCompilationTraceDelayInSeconds * 1,000,000,000);
    // }
  };
  embedder_args.root_isolate_shutdown_callback = [](void* user_data) {
    Component* component = static_cast<Component*>(user_data);
    FX_DCHECK(component);

    FX_DLOG(INFO) << "Main isolate for engine '" << component->debug_label_
                  << "' shutting down.";

    // Will collect the component.
    component->Kill();
  };
  embedder_args.update_semantics_node_callback =
      [](const FlutterSemanticsNode* update, void* user_data) {
        Component* component = static_cast<Component*>(user_data);
        FX_DCHECK(component);

        component->renderer_->UpdateSemanticsNode(update);
      };
  embedder_args.update_semantics_custom_action_callback =
      [](const FlutterSemanticsCustomAction* action, void* user_data) {
        Component* component = static_cast<Component*>(user_data);
        FX_DCHECK(component);

        component->renderer_->UpdateSemanticsCustomAction(action);
      };
  embedder_args.persistent_cache_path = nullptr;
  embedder_args.is_persistent_cache_read_only = true;
  embedder_args.vsync_callback = [](void* user_data, intptr_t baton) {
    Component* component = static_cast<Component*>(user_data);
    FX_DCHECK(component);

    component->renderer_->AwaitVsync(baton);
  };
  embedder_args.custom_dart_entrypoint = nullptr;
  embedder_args.custom_task_runners = &embedder_custom_task_runners;
  embedder_args.shutdown_dart_vm_when_done = false;
  embedder_args.unhandled_exception_callback =
      [](void* user_data, const char* error, const char* stack_trace) -> bool {
    Component* component = static_cast<Component*>(user_data);
    FX_DCHECK(component);

    FX_LOG(ERROR) << "UNHANDLED EXCEPTION: " << error << " " << std::endl
                  << stack_trace;
    // TODO(dworsham)
    // Also not thread-safe :(
    // async::PostTask(component->platform_thread_.dispatcher(),
    //     [embedder, error, stack_trace]() {
    //       fx::HandleException(embedder->runner_incoming_services_,
    //       embedder->component_url_, error, stack_trace);
    //     });

    // Ideally we would return whether HandleException returned ZX_OK, but
    // short of knowing if the exception was correctly handled, we return
    // false to have the error and stack trace printed in the logs.
    return false;
  };
  embedder_args.verbose_logging = true;
  embedder_args.log_tag = log_tag.c_str();
  embedder_args.compositor = &embedder_compositor_callbacks;
  embedder_args.dart_old_gen_heap_size = -1;

  // FlutterRendererConfig embedder_renderer_config({
  //     .type = kVulkan,
  //     .vulkan =
  //         {
  //             .struct_size = sizeof(FlutterVulkanRendererConfig),
  //         },
  // });

  FlutterRendererConfig embedder_renderer_config({
      .type = kSoftware,
      .software =
          {
              .struct_size = sizeof(FlutterSoftwareRendererConfig),
              .surface_present_callback =
                  [](void* user_data, const void* allocation, size_t row_bytes,
                     size_t height) -> bool {
                FX_LOG(ERROR) << "SOFTWARE SURFACE PRESENT";
                FX_DCHECK(false);

                return true;
              },
          },
  });

  // Create the Flutter engine.
  auto init_result =
      FlutterEngineInitialize(FLUTTER_ENGINE_VERSION, &embedder_renderer_config,
                              &embedder_args, this, &engine_);
  FX_DCHECK(init_result == kSuccess);

  // Run the Flutter engine now that everything is ready to go.
  auto run_result = FlutterEngineRunInitialized(engine_);
  FX_DCHECK(run_result == kSuccess);

  // Connect to the system font provider and set the engine's default font
  // manager, now that it has been created.
  fuchsia::fonts::ProviderSyncPtr sync_font_provider;
  incoming_services_->Connect(sync_font_provider.NewRequest());
  auto reload_fonts_result = FlutterEngineReloadSystemFonts(
      engine_, static_cast<uintptr_t>(
                   sync_font_provider.Unbind().TakeChannel().release()));
  FX_DCHECK(reload_fonts_result == kSuccess);
}

void Component::TerminateFlutter() {
  // TODO(dworsham)
  // We may launch multiple shell in this application. However, we will
  // terminate when the last shell goes away. The error code return to the
  // application controller will be the last isolate that had an error.
  // auto return_code = std::pair<true,
  // 0>();//shell_holder->GetEngineReturnCode(); if (return_code.first) {
  //   last_return_code_ = return_code;
  // }

  // Destroy the Flutter engine now that nothing else depends on it.
  auto shutdown_result = FlutterEngineShutdown(engine_);
  FX_DCHECK(shutdown_result == kSuccess);
  engine_ = nullptr;
}

}  // namespace flutter_runner
