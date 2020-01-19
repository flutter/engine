// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/component.h"

#include <fuchsia/fonts/cpp/fidl.h>
#include <fuchsia/mem/cpp/fidl.h>
#include <lib/fdio/directory.h>
#include <lib/fdio/io.h>
#include <lib/fdio/namespace.h>
#include <lib/vfs/cpp/remote_dir.h>
#include <lib/vfs/cpp/service.h>
#include <zircon/status.h>
#include <zircon/types.h>
#include <fcntl.h>

#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/files.h"
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/handle_exception.h"
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/mapped_resource.h"
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/tempfs.h"
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/vmo.h"
#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/icu/source/common/unicode/bytestream.h"
#include "third_party/icu/source/common/unicode/errorcode.h"
#include "third_party/icu/source/common/unicode/locid.h"
#include "third_party/icu/source/common/unicode/strenum.h"
#include "third_party/icu/source/common/unicode/stringpiece.h"
#include "third_party/icu/source/common/unicode/uloc.h"
#include "third_party/skia/include/core/SkFontMgr.h"
#include "third_party/skia/include/ports/SkFontMgr_fuchsia.h"
#include "third_party/tonic/logging/dart_error.h"

namespace flutter_runner {
namespace {

constexpr char kDataKey[] = "data";
constexpr char kTmpPath[] = "/tmp";
constexpr char kServiceRootPath[] = "/svc";

std::string DebugLabelForURL(const std::string& url) {
  auto found = url.rfind("/");
  if (found == std::string::npos) {
    return url;
  } else {
    return {url, found + 1};
  }
}

std::unique_ptr<fml::FileMapping> MakeFileMapping(const char* path, bool executable) {
  uint32_t flags = fuchsia::io::OPEN_RIGHT_READABLE;
  if (executable) {
    flags |= fuchsia::io::OPEN_RIGHT_EXECUTABLE;
  }

  int fd = 0;
  // The returned file descriptor is compatible with standard posix operations
  // such as close, mmap, etc. We only need to treat open/open_at specially.
  zx_status_t status = fdio_open_fd(path, flags, &fd);

  if (status != ZX_OK) {
    FML_LOG(ERROR) << "Failed to open " << path;
    fd = -1;
  }

  using Protection = fml::FileMapping::Protection;

  std::initializer_list<Protection> protection_execute = {Protection::kRead,
                                                          Protection::kExecute};
  std::initializer_list<Protection> protection_read = {Protection::kRead};
  auto mapping = std::make_unique<fml::FileMapping>(fml::UniqueFD{fd},
                          executable ? protection_execute : protection_read);
  if (!mapping->IsValid()) {
    FML_LOG(ERROR) << "Invalid FileMapping for " << path;
  }

  return mapping;
}

// TODO
// void UpdateNativeThreadLabelNames(const std::string& label,
//                                   const flutter::TaskRunners& runners) {
//   auto set_thread_name = [](fml::RefPtr<fml::TaskRunner> runner,
//                             std::string prefix, std::string suffix) {
//     if (!runner) {
//       return;
//     }
//     fml::TaskRunner::RunNowOrPostTask(runner, [name = prefix + suffix]() {
//       zx::thread::self()->set_property(ZX_PROP_NAME, name.c_str(),
//       name.size());
//     });
//   };
//   set_thread_name(runners.GetPlatformTaskRunner(), label, ".platform");
//   set_thread_name(runners.GetUITaskRunner(), label, ".ui");
//   set_thread_name(runners.GetGPUTaskRunner(), label, ".gpu");
//   set_thread_name(runners.GetIOTaskRunner(), label, ".io");
// }

// TODO
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
//       FML_LOG(INFO) << "compilation-trace: " << line;

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
//     if (dart_utils::WriteFile(kTypeFeedbackFile,
//                               reinterpret_cast<const char*>(feedback),
//                               feedback_length)) {
//       FML_LOG(INFO) << "Dart type feedback written to " << kTypeFeedbackFile;
//     } else {
//       FML_LOG(ERROR) << "Could not write Dart type feedback to "
//                      << kTypeFeedbackFile;
//     }
//     Dart_ExitScope();
//   }

//   Dart_ExitIsolate();
// }

}  // namespace

Component::Component(
    TerminationCallback termination_callback,
    fuchsia::sys::Package package,
    fuchsia::sys::StartupInfo startup_info,
    std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
    fidl::InterfaceRequest<fuchsia::sys::ComponentController> controller_request)
    : debug_label_(DebugLabelForURL(startup_info.launch_info.url)),
      component_url_(package.resolved_url),
      window_manager_(debug_label_,
          std::bind(&Component::OnWindowCreated, this, std::placeholders::_1),
          std::bind(&Component::OnWindowDestroyed, this, std::placeholders::_1),
          runner_incoming_services),
      component_controller_(this),
      outgoing_dir_(new vfs::PseudoDir()),
      runner_incoming_services_(runner_incoming_services),
      termination_callback_(std::move(termination_callback)),
      weak_factory_(this) {
  FML_DCHECK(fdio_ns_.is_valid());

  component_controller_.set_error_handler(
      [this](zx_status_t status) { Kill(); });

  // Determine /pkg/data directory from StartupInfo.
  for (size_t i = 0; i < startup_info.program_metadata->size(); ++i) {
    auto pg = startup_info.program_metadata->at(i);
    if (pg.key.compare(kDataKey) == 0) {
      project_args_.data_path = "pkg/" + pg.value;
    }
  }
  if (project_args_.data_path.empty()) {
    FML_DLOG(ERROR) << "Could not find a /pkg/data directory for "
                    << package.resolved_url;
    return;
  }

  // Setup /tmp to be mapped to the process-local memfs.
  dart_utils::RunnerTemp::SetupComponent(fdio_ns_.get());

  // LaunchInfo::url non-optional.
  auto& launch_info = startup_info.launch_info;

  // LaunchInfo::arguments optional.
  if (launch_info.arguments) {
    for (auto& argument : launch_info.arguments.value()) {
      project_args_.dart_entrypoint_args.push_back(argument);
    }
  }

  // LaunchInfo::flat_namespace optional.
  for (size_t i = 0; i < startup_info.flat_namespace.paths.size(); ++i) {
    const auto& path = startup_info.flat_namespace.paths.at(i);
    if (path == kTmpPath) {
      continue;
    }

    zx::channel dir;
    if (path == kServiceRootPath) {
      component_incoming_services_ = std::make_unique<sys::ServiceDirectory>(
          std::move(startup_info.flat_namespace.directories.at(i)));
      dir = component_incoming_services_->CloneChannel().TakeChannel();
    } else {
      dir = std::move(startup_info.flat_namespace.directories.at(i));
    }

    zx_handle_t dir_handle = dir.release();
    if (fdio_ns_bind(fdio_ns_.get(), path.data(), dir_handle) != ZX_OK) {
      FML_DLOG(ERROR) << "Could not bind path to namespace: " << path;
      zx_handle_close(dir_handle);
    }
  }

  auto component_directory = fml::UniqueFD(fdio_ns_opendir(fdio_ns_.get()));
  FML_DCHECK(component_directory.is_valid());

  assets_directory_.reset(openat(
      component_directory.get(), project_args_.data_path.c_str(), O_RDONLY | O_DIRECTORY));
  FML_DCHECK(assets_directory_.is_valid());

  // TODO: LaunchInfo::out.

  // TODO: LaunchInfo::err.

  // LaunchInfo::service_request optional.
  if (launch_info.directory_request) {
    outgoing_dir_->Serve(fuchsia::io::OPEN_RIGHT_READABLE |
                             fuchsia::io::OPEN_RIGHT_WRITABLE |
                             fuchsia::io::OPEN_FLAG_DIRECTORY,
                         std::move(launch_info.directory_request));
  }

  directory_request_ = directory_ptr_.NewRequest();

  fidl::InterfaceHandle<fuchsia::io::Directory> flutter_public_dir;
  // TODO(anmittal): when fixing enumeration using new c++ vfs, make sure that
  // flutter_public_dir is only accessed once we receive OnOpen Event.
  // That will prevent FL-175 for public directory
  auto request = flutter_public_dir.NewRequest().TakeChannel();
  fdio_service_connect_at(directory_ptr_.channel().get(), "svc",
                          request.release());

  auto composed_service_dir = std::make_unique<vfs::ComposedServiceDir>();
  composed_service_dir->set_fallback(std::move(flutter_public_dir));

  // Clone and check if client is servicing the directory.
  directory_ptr_->Clone(fuchsia::io::OPEN_FLAG_DESCRIBE |
                            fuchsia::io::OPEN_RIGHT_READABLE |
                            fuchsia::io::OPEN_RIGHT_WRITABLE,
                        cloned_directory_ptr_.NewRequest());

  cloned_directory_ptr_.events().OnOpen =
      [this](zx_status_t status, std::unique_ptr<fuchsia::io::NodeInfo> info) {
        cloned_directory_ptr_.Unbind();
        if (status != ZX_OK) {
          FML_LOG(ERROR) << "could not bind out directory for flutter app("
                         << debug_label_
                         << "): " << zx_status_get_string(status);
          return;
        }
        const char* other_dirs[] = {"debug", "ctrl", "diagnostics"};
        // add other directories as RemoteDirs.
        for (auto& dir_str : other_dirs) {
          fidl::InterfaceHandle<fuchsia::io::Directory> dir;
          auto request = dir.NewRequest().TakeChannel();
          auto status = fdio_service_connect_at(directory_ptr_.channel().get(),
                                                dir_str, request.release());
          if (status == ZX_OK) {
            outgoing_dir_->AddEntry(
                dir_str, std::make_unique<vfs::RemoteDir>(dir.TakeChannel()));
          } else {
            FML_LOG(ERROR) << "could not add out directory entry(" << dir_str
                           << ") for flutter app(" << debug_label_
                           << "): " << zx_status_get_string(status);
          }
        }
      };

  cloned_directory_ptr_.set_error_handler(
      [this](zx_status_t status) { cloned_directory_ptr_.Unbind(); });

  // TODO: LaunchInfo::additional_services optional.

  // All launch arguments have been read. Perform service binding and
  // final settings configuration.
  if (component_incoming_services_) {
    window_manager_.RegisterServices(composed_service_dir.get());
  } else {
    FML_DLOG(ERROR)
        << "Component incoming services was invalid after reading launch "
        << "arguments.  This flutter_runner will not be able to respond to "
        << "View requests.";
  }
  outgoing_dir_->AddEntry("svc", std::move(composed_service_dir));

  // Setup the application controller binding.
  if (controller_request) {
    component_controller_.Bind(std::move(controller_request));
  }

//   // Connect to the intl property provider.  If the connection fails, the
//   // initialization of the engine will simply proceed, printing a warning
//   // message.  The engine will be fully functional, except that the user's
//   // locale preferences would not be communicated to flutter engine.
//   {
//     intl_property_provider_.set_error_handler([](zx_status_t status) {
//       FML_LOG(WARNING) << "Failed to connect to "
//                        << fuchsia::intl::PropertyProvider::Name_ << ": "
//                        << zx_status_get_string(status)
//                        << " This is not a fatal error, but the user locale "
//                        << " preferences will not be forwarded to flutter apps";
//     });

//     // Note that we're using the runner's services, not the component's.
//     // Flutter locales should be updated regardless of whether the component has
//     // direct access to the fuchsia.intl.PropertyProvider service.
//     ZX_ASSERT(runner_incoming_services->Connect(
//                   intl_property_provider_.NewRequest()) == ZX_OK);

//     auto get_profile_callback = [weak = weak_factory_.GetWeakPtr()](
//                                     const fuchsia::intl::Profile& profile) {
//       if (!weak) {
//         return;
//       }

//       if (!profile.has_locales()) {
//         FML_LOG(WARNING) << "Got intl Profile without locales";
//       }

//       FML_VLOG(-1) << "Sending LocalizationPlatformMessage";
//       std::vector<FlutterLocale> locales_holder(profile.locales().size());
//       std::vector<const FlutterLocale*> locales;
//       for (const auto& locale_id : profile.locales()) {
//         UErrorCode error_code = U_ZERO_ERROR;
//         icu::Locale icu_locale =
//             icu::Locale::forLanguageTag(locale_id.id, error_code);
//         if (U_FAILURE(error_code)) {
//           FML_LOG(ERROR) << "Error parsing locale ID \"" << locale_id.id
//                          << "\"";
//           continue;
//         }

//         std::string country =
//             icu_locale.getCountry() != nullptr ? icu_locale.getCountry() : "";
//         std::string script =
//             icu_locale.getScript() != nullptr ? icu_locale.getScript() : "";
//         std::string variant =
//             icu_locale.getVariant() != nullptr ? icu_locale.getVariant() : "";
//         // ICU4C capitalizes the variant for backward compatibility, even though
//         // the preferred form is lowercase.  So we lowercase here.
//         std::transform(begin(variant), end(variant), begin(variant),
//                        [](unsigned char c) { return std::tolower(c); });

//         FlutterLocale locale = {
//             .struct_size = sizeof(FlutterLocale),
//             .language_code = icu_locale.getLanguage(),
//             .country_code = country.c_str(),
//             .script_code = script.c_str(),
//             .variant_code = variant.c_str(),
//         };
//         locales_holder.emplace_back(std::move(locale));
//         locales.push_back(&locales_holder.back());
//       }
//       for (auto& embedder : weak->flutter_embedders_) {
//         FlutterEngineUpdateLocales(embedder->engine, locales.data(),
//                                    locales.size());
//       }
//     };

//     FML_VLOG(-1) << "Requesting intl Profile";

//     // Make the initial request
//     intl_property_provider_->GetProfile(get_profile_callback);

//     // And register for changes
//     intl_property_provider_.events().OnChange =
//         [weak = weak_factory_.GetWeakPtr(), get_profile_callback]() {
//           if (!weak) {
//             return;
//           }

//           FML_VLOG(-1) << fuchsia::intl::PropertyProvider::Name_
//                        << ": OnChange";
//           weak->runner_incoming_services_->Connect(
//               weak->intl_property_provider_.NewRequest());
//           weak->intl_property_provider_->GetProfile(get_profile_callback);
//         };
//   }

  // Check if we can use the snapshot with the framework already loaded.
  {
    std::string runner_framework;
    std::string app_framework;
    if (dart_utils::ReadFileToStringAt(assets_directory_.get(),
                                       "runner.frameworkversion",
                                       &runner_framework) &&
        dart_utils::ReadFileToStringAt(assets_directory_.get(),
                                       "app.frameworkversion",
                                       &app_framework) &&
        (runner_framework.compare(app_framework) == 0)) {
      project_args_.vm_snapshot_data_mapping =
          MakeFileMapping("/pkg/data/framework_vm_snapshot_data.bin", false);
      project_args_.isolate_snapshot_data_mapping = MakeFileMapping(
          "/pkg/data/framework_isolate_core_snapshot_data.bin", false);

      FML_LOG(INFO) << "Using snapshot with framework for "
                    << package.resolved_url;
    } else {
      project_args_.vm_snapshot_data_mapping =
          MakeFileMapping("/pkg/data/vm_snapshot_data.bin", false);
      project_args_.isolate_snapshot_data_mapping =
          MakeFileMapping("/pkg/data/isolate_core_snapshot_data.bin", false);

      FML_LOG(INFO) << "Using snapshot without framework for "
                    << package.resolved_url;
    }
  }
  project_args_.vm_snapshot_instructions_mapping =
      MakeFileMapping("/pkg/data/vm_snapshot_instructions.bin", true);
  project_args_.isolate_snapshot_instructions_mapping =
      MakeFileMapping("/pkg/data/isolate_core_snapshot_instructions.bin", true);

  if (!FlutterEngineRunsAOTCompiledDartCode()) {
    // The interpreter is enabled unconditionally in JIT mode. If an app is
    // built for debugging (that is, with no bytecode), the VM will fall back on
    // ASTs.
    project_args_.dart_flags.push_back("--enable_interpreter");
  }

  // Scale back CPU profiler sampling period on ARM64 to avoid overloading
  // the tracing engine.
#if defined(__aarch64__)
  project_args_.dart_flags.push_back("--profile_period=10000");
#endif  // defined(__aarch64__)

  // TODO(FL-117): Re-enable causal async stack traces when this issue is
  // addressed.
  project_args_.dart_flags.push_back("--no_causal_async_stacks");

  // Disable code collection as it interferes with JIT code warmup
  // by decreasing usage counters and flushing code which is still useful.
  project_args_.dart_flags.push_back("--no-collect_code");

  // Don't collect CPU samples from Dart VM C++ code.
  project_args_.dart_flags.push_back("--no_profile_vm");

  // Custom log tag for this component.
  project_args_.log_tag = debug_label_ + std::string{"(flutter)"};
}

Component::~Component() = default;

const std::string& Component::GetDebugLabel() const {
  return debug_label_;
}

#if !defined(DART_PRODUCT)
void Component::WriteProfileToTrace() const {
  // TODO
  // for (const auto& embedder : flutter_embedders_) {
  //   embedder->engine.WriteProfileToTrace();
  //   Dart_Port main_port = shell_->GetEngine()->GetUIIsolateMainPort();
  //   char* error = NULL;
  //   bool success = Dart_WriteProfileToTimeline(main_port, &error);
  //   if (!success) {
  //     FML_LOG(ERROR) << "Failed to write Dart profile to trace: " << error;
  //     free(error);
  //   }
  // }
}

#endif  // !defined(DART_PRODUCT)

void Component::Kill() {
  component_controller_.events().OnTerminated(
      last_return_code_.second, fuchsia::sys::TerminationReason::EXITED);

  // WARNING: Don't do anything past this line as this instance may have been
  // collected.
  termination_callback_(this);
}

void Component::Detach() {
  component_controller_.set_error_handler(nullptr);
}

void Component::OnWindowCreated(ScenicWindow* window) {
  FML_DCHECK(component_incoming_services_);

  fuchsia::sys::EnvironmentPtr environment;
  component_incoming_services_->Connect(environment.NewRequest());

  auto& embedder_iter =
      flutter_embedders_.emplace_back(std::make_unique<Embedder>(Embedder{
        .debug_label = debug_label_,
        .weak_component = weak_factory_.GetWeakPtr(),
        // TODO ViewRef
        .configurator = IsolateConfigurator(std::move(fdio_ns_), std::move(environment), directory_request_.TakeChannel(), zx::eventpair()),
        .window = window,
      }));

  FlutterRendererConfig renderer_config({
      .type = kVulkan,
      .vulkan =
          {
              .struct_size = sizeof(FlutterVulkanRendererConfig),
          },
  });

  FlutterCompositor compositor_callbacks = {
      .struct_size = sizeof(FlutterCompositor),
      .user_data = &embedder_iter,
      .create_backing_store_callback =
          [](const FlutterBackingStoreConfig* layer_config,
             FlutterBackingStore* backing_store_out, void* user_data) {
            TRACE_EVENT0("flutter", "FlutterCompositorCreateBackingStore");
            auto embedder = static_cast<Embedder*>(user_data);
            FML_DCHECK(embedder);

            bool status = embedder->window->CreateBackingStore(
                layer_config, backing_store_out);
            if (!status) {
              FML_LOG(ERROR) << "Failed creating backing store";
              return false;
            }

            return true;
          },
      .collect_backing_store_callback =
          [](const FlutterBackingStore* backing_store, void* user_data) {
            TRACE_EVENT0("flutter", "FlutterCompositorCollectBackingStore");
            auto embedder = static_cast<Embedder*>(user_data);
            FML_DCHECK(embedder);

            bool status = embedder->window->CollectBackingStore(backing_store);
            if (!status) {
              FML_LOG(ERROR) << "Failed collecting backing store";
              return false;
            }

            return true;
          },
      .present_layers_callback =
          [](const FlutterLayer** layers, size_t layer_count, void* user_data) {
            TRACE_EVENT0("flutter", "FlutterCompositorPresentLayers");
            auto embedder = static_cast<Embedder*>(user_data);
            FML_DCHECK(embedder);

            bool status = embedder->window->PresentLayers(layers, layer_count);
            if (!status) {
              FML_LOG(ERROR) << "Failed presenting layers";
              return false;
            }

            return true;
          },
  };

  FlutterProjectArgs embedder_args;
  std::vector<const char*> dart_flags_holder(project_args_.dart_flags.size());
  std::vector<const char*> dart_entrypoint_args_holder(project_args_.dart_entrypoint_args.size());
  embedder_args.struct_size = sizeof(FlutterProjectArgs);
  embedder_args.assets_path = project_args_.data_path.c_str();
  embedder_args.assets_dir = assets_directory_.get();
  embedder_args.main_path__unused__ = nullptr;
  embedder_args.packages_path__unused__ = nullptr;
  embedder_args.icu_data_path = "";
  embedder_args.command_line_argc = 0;
  embedder_args.command_line_argv = nullptr;
  embedder_args.platform_message_callback =
      [](const FlutterPlatformMessage* message, void* user_data) {
        auto embedder = static_cast<Embedder*>(user_data);
        FML_DCHECK(embedder);

        embedder->window->PlatformMessageResponse(message);
      };
  embedder_args.application_kernel_asset = "app_aot_snapshot.so";  // TODO
  embedder_args.application_kernel_list_asset = "app.dilplist";  // TODO
  if (project_args_.vm_snapshot_data_mapping) {
    embedder_args.vm_snapshot_data = project_args_.vm_snapshot_data_mapping->GetMapping();
    embedder_args.vm_snapshot_data_size = project_args_.vm_snapshot_data_mapping->GetSize();
  }
  if (project_args_.vm_snapshot_instructions_mapping) {
    embedder_args.vm_snapshot_instructions = project_args_.vm_snapshot_instructions_mapping->GetMapping();
    embedder_args.vm_snapshot_instructions_size = project_args_.vm_snapshot_instructions_mapping->GetSize();
  }
  if (project_args_.isolate_snapshot_data_mapping) {
    embedder_args.isolate_snapshot_data = project_args_.isolate_snapshot_data_mapping->GetMapping();
    embedder_args.isolate_snapshot_data_size = project_args_.isolate_snapshot_data_mapping->GetSize();
  }
  if (project_args_.isolate_snapshot_instructions_mapping) {
    embedder_args.isolate_snapshot_instructions = project_args_.isolate_snapshot_instructions_mapping->GetMapping();
    embedder_args.isolate_snapshot_instructions_size = project_args_.isolate_snapshot_instructions_mapping->GetSize();
  }
  for (auto& flag : project_args_.dart_flags) {
    dart_flags_holder[embedder_args.dart_flags_count++] =
        flag.c_str();
  }
  embedder_args.dart_entrypoint_args = dart_entrypoint_args_holder.data();
  for (auto& argument : project_args_.dart_entrypoint_args) {
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
    FML_LOG(ERROR) << "ROOT ISOLATE CREATE";
    Embedder* embedder = static_cast<Embedder*>(user_data);
    FML_DCHECK(embedder);

    FML_DLOG(INFO) << "Main isolate for engine '" << embedder->debug_label
                   << "' was started.";

    if (!embedder->configurator.ConfigureCurrentIsolate()) {
      FML_LOG(ERROR)
          << "Could not configure some native embedder bindings for a "
             "new root isolate.";
    }

    // TODO
    // const intptr_t kCompilationTraceDelayInSeconds = 0;
    // if (kCompilationTraceDelayInSeconds != 0) {
    //   Dart_Isolate isolate = Dart_CurrentIsolate();
    //   FML_CHECK(isolate);
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
    FML_DCHECK(false) << "ROOT ISOLATE SHUTDOWN";
    Embedder* embedder = static_cast<Embedder*>(user_data);
    FML_DCHECK(embedder);

    FML_DLOG(INFO) << "Main isolate for engine '" << embedder->debug_label
                   << "' shutting down.";
    if (!embedder->weak_component) {
      FML_DLOG(INFO) << "Component already destroyed in isolate shutdown "
                     << "callback for engine '" << embedder->debug_label
                     << "'; skipping shutdown";
      return;
    }

    embedder->weak_component->DestroyEmbedder(embedder);
  };
  embedder_args.update_semantics_node_callback =
      [](const FlutterSemanticsNode* update, void* user_data) {
        auto embedder = static_cast<Embedder*>(user_data);
        FML_DCHECK(embedder);

        embedder->window->UpdateSemanticsNode(update);
      };
  embedder_args.update_semantics_custom_action_callback =
      [](const FlutterSemanticsCustomAction* action, void* user_data) {
        auto embedder = static_cast<Embedder*>(user_data);
        FML_DCHECK(embedder);

        embedder->window->UpdateSemanticsCustomAction(action);
      };
  embedder_args.persistent_cache_path = nullptr;
  embedder_args.is_persistent_cache_read_only = true;
  embedder_args.vsync_callback = [](void* user_data, intptr_t baton) {
    auto embedder = static_cast<Embedder*>(user_data);
    FML_DCHECK(embedder);

    embedder->window->AwaitPresent(baton);
  };
  embedder_args.custom_dart_entrypoint = nullptr;
  embedder_args.custom_task_runners = nullptr;
  embedder_args.shutdown_dart_vm_when_done = false;
  embedder_args.unhandled_exception_callback =
      [](void* user_data, const char* error, const char* stack_trace) -> bool {
    Embedder* embedder = static_cast<Embedder*>(user_data);
    FML_DCHECK(embedder);

    // TODO
    // embedder->platform_task_runner->PostTask(
    //     [embedder, error, stack_trace]() {
    //       if (embedder->weak_component) {
    //         dart_utils::HandleException(
    //             embedder->weak_component->runner_incoming_services_,
    //             embedder->weak_component->component_url_, error,
    //             stack_trace);
    //       } else {
    //         FML_LOG(WARNING)
    //             << "Exception was thrown which was not caught in Flutter app:
    //             "
    //             << error;
    //       }
    //     });

    // Ideally we would return whether HandleException returned ZX_OK, but
    // short of knowing if the exception was correctly handled, we return
    // false to have the error and stack trace printed in the logs.
    return false;
  };
  embedder_args.verbose_logging = true;
  embedder_args.log_tag = project_args_.log_tag.c_str();
  embedder_args.compositor = &compositor_callbacks;
  embedder_args.dart_old_gen_heap_size = -1;

  // Create the Flutter engine.
  auto init_result = FlutterEngineInitialize(
      FLUTTER_ENGINE_VERSION, &renderer_config, &embedder_args,
      embedder_iter.get(), &embedder_iter->engine);
  FML_DCHECK(init_result == kSuccess);

  // Bind the Flutter engine to the |Window|, now that it has been created.
  window->BindFlutterEngine(embedder_iter->engine);

  // TODO
  // // Connect to the system font provider and set the engine's default font
  // // manager, now that it has been created.
  // fuchsia::fonts::ProviderSyncPtr sync_font_provider;
  // component_incoming_services_->Connect(sync_font_provider.NewRequest());
  // engine->GetFontCollection().GetFontCollection()->SetDefaultFontManager(
  //     SkFontMgr_New_Fuchsia(std::move(sync_font_provider)));

  // Run the Flutter engine now that everything is ready to go.
  auto run_result = FlutterEngineRunInitialized(embedder_iter->engine);
  FML_DCHECK(run_result == kSuccess);
}

void Component::OnWindowDestroyed(ScenicWindow* window) {
  FML_DCHECK(window);

  // Find the embedder associated with this |Window|.
  auto found =
      std::find_if(flutter_embedders_.begin(), flutter_embedders_.end(),
                   [window](const auto& embedder) -> bool {
                     return embedder->window == window;
                   });
  if (found == flutter_embedders_.end()) {
    FML_LOG(INFO) << "Tried to terminate a Flutter engine that did not exist";
    return;
  }

  // "Swap'n'pop" to remove the embedder.
  auto embedder_unique_ptr = std::move(*found);
  if (flutter_embedders_.size() > 1) {
    std::iter_swap(found, flutter_embedders_.end() - 1);
  }
  flutter_embedders_.pop_back();

  // TODO
  // We may launch multiple shell in this application. However, we will
  // terminate when the last shell goes away. The error code return to the
  // application controller will be the last isolate that had an error.
  // auto return_code = std::pair<true,
  // 0>();//shell_holder->GetEngineReturnCode(); if (return_code.first) {
  //   last_return_code_ = return_code;
  // }

  // Destroy the Flutter engine now that nothing else depends on it.
  auto shutdown_result = FlutterEngineShutdown(found->get()->engine);
  FML_DCHECK(shutdown_result == kSuccess);

  // "Swap'n'pop" to remove the embedder.
  if (flutter_embedders_.size() > 1) {
    std::iter_swap(found, flutter_embedders_.end() - 1);
  }
  flutter_embedders_.pop_back();

  // Shutdown the component if no more embedders are left.
  if (flutter_embedders_.size() == 0) {
    // WARNING: Don't do anything past this line because the delegate may have
    // collected this instance via the termination callback.
    Kill();
  }
}

void Component::DestroyEmbedder(Embedder* embedder) {
  FML_DCHECK(embedder);

  // Find the embedder's position in the vector.
  auto found =
      std::find_if(flutter_embedders_.begin(), flutter_embedders_.end(),
                   [embedder](const auto& unique_embedder) -> bool {
                     return unique_embedder.get() == embedder;
                   });
  if (found == flutter_embedders_.end()) {
    FML_LOG(INFO) << "Tried to terminate a Flutter engine that did not exist";
    return;
  }

  // The isolate has already stopped running so its safe to destroy the window.
  window_manager_.DestroyWindow(found->get()->window);

  // We may launch multiple shell in this application. However, we will
  // terminate when the last shell goes away. The error code return to the
  // application controller will be the last isolate that had an error.
  // TODO
  // auto return_code = std::pair<true,
  // 0>();//shell_holder->GetEngineReturnCode(); if (return_code.first) {
  //   last_return_code_ = return_code;
  // }

  // Destroy the Flutter engine now that nothing else depends on it.
  auto shutdown_result = FlutterEngineShutdown(found->get()->engine);
  FML_DCHECK(shutdown_result == kSuccess);

  // "Swap'n'pop" to remove the embedder.
  if (flutter_embedders_.size() > 1) {
    std::iter_swap(found, flutter_embedders_.end() - 1);
  }
  flutter_embedders_.pop_back();

  // Shutdown the component if no more embedders are left.
  if (flutter_embedders_.size() == 0) {
    // WARNING: Don't do anything past this line because the delegate may have
    // collected this instance via the termination callback.
    Kill();
  }
}

}  // namespace flutter_runner
