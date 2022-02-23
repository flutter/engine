// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dart_runner.h"

#include <lib/async-loop/loop.h>
#include <lib/async/default.h>
#include <lib/syslog/global.h>
#include <sys/stat.h>
#include <zircon/status.h>
#include <zircon/syscalls.h>

#include <cerrno>
#include <memory>
#include <sstream>
#include <thread>
#include <utility>

#include "dart_component_controller.h"
#include "dart_component_controller_v2.h"
#include "dart_test_component_controller_v2.h"
#include "flutter/fml/command_line.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "logging.h"
#include "runtime/dart/utils/inlines.h"
#include "runtime/dart/utils/program_metadata.h"
#include "runtime/dart/utils/vmservice_object.h"
#include "service_isolate.h"
#include "third_party/dart/runtime/include/bin/dart_io_api.h"
#include "third_party/tonic/dart_microtask_queue.h"
#include "third_party/tonic/dart_state.h"

// #include "fuchsia_pkg_url.h"

#if defined(AOT_RUNTIME)
extern "C" uint8_t _kDartVmSnapshotData[];
extern "C" uint8_t _kDartVmSnapshotInstructions[];
#endif

namespace {
struct ComponentArgs {
  std::string legacy_url;
  std::shared_ptr<sys::ServiceDirectory> test_component_svc;
  fidl::InterfaceHandle<fuchsia::io::Directory> component_pkg;
  std::vector<fuchsia::component::runner::ComponentNamespaceEntry> ns;
};

// fpromise::result<ComponentArgs, fuchsia::component::Error> GetComponentArgs(
//     fuchsia::component::runner::ComponentStartInfo& start_info) {
//   // component::FuchsiaPkgUrl url;
//   // if (!url.Parse(start_info.resolved_url())) {
//   //   // FX_LOGS(WARNING) << "cannot run test: " <<
//   start_info.resolved_url()
//   //   //                  << ", as we cannot parse url.";
//   //   return fpromise::error(fuchsia::component::Error::INVALID_ARGUMENTS);
//   // }
//   FML_LOG(INFO) << "DART RUNNER : GET COMPONENT ARGS ";
//   if (!start_info.program().has_entries()) {
//     FML_LOG(INFO) << "cannot run test: " << start_info.resolved_url()
//                   << ", as it has no program entry.";
//     return fpromise::error(fuchsia::component::Error::INVALID_ARGUMENTS);
//   }

//   auto ns = std::move(*start_info.mutable_ns());
//   // const std::string& legacy_manifest = it->value->str();
//   FML_LOG(INFO) << "DART RUNNER : GET COMPONENT ARGS 2";

//   auto pkg_it = std::find_if(
//       ns.begin(), ns.end(),
//       [](fuchsia::component::runner::ComponentNamespaceEntry& entry) {
//         return entry.path() == "/pkg";
//       });
//   FML_LOG(INFO) << "DART RUNNER : GET COMPONENT ARGS 3";

//   auto component_pkg = std::move(*pkg_it->mutable_directory());
//   FML_LOG(INFO) << "DART RUNNER : GET COMPONENT ARGS 4";

//   // auto fd = fsl::OpenChannelAsFileDescriptor(component_pkg.TakeChannel());
//   // fsl::SizedVmo vmo;
//   // if (!fsl::VmoFromFilenameAt(fd.get(), legacy_manifest, &vmo)) {
//   //   FX_LOGS(WARNING) << "cannot run test: " << start_info.resolved_url()
//   //                    << ", as cannot read legacy manifest file.";
//   //   return
//   fpromise::error(fuchsia::component::Error::INSTANCE_CANNOT_START);
//   // }

//   // const uint64_t size = vmo.size();
//   // std::string cmx_str(size, ' ');
//   // auto status = vmo.vmo().read(cmx_str.data(), 0, size);
//   // if (status != ZX_OK) {
//   //   FX_LOGS(WARNING) << "cannot run test: " << start_info.resolved_url()
//   //                    << ", as cannot read legacy manifest file: "
//   //                    << zx_status_get_string(status) << ".";
//   //   return
//   fpromise::error(fuchsia::component::Error::INSTANCE_CANNOT_START);
//   // }
//   // auto legacy_url = url.package_path() + "#" + legacy_manifest;

//   auto legacy_url = start_info.resolved_url();
//   FML_LOG(INFO) << "DART RUNNER : GET COMPONENT ARGS 5";
//   auto svc_it = std::find_if(
//       ns.begin(), ns.end(),
//       [](fuchsia::component::runner::ComponentNamespaceEntry& entry) {
//         return entry.path() == "/svc";
//       });

//   FML_LOG(INFO) << "DART RUNNER : GET COMPONENT ARGS 6";

//   auto component_svc = std::make_shared<sys::ServiceDirectory>(
//       std::move(*svc_it->mutable_directory()));
//   FML_LOG(INFO) << "DART RUNNER : GET COMPONENT ARGS 7";
//   return fpromise::ok(
//       ComponentArgs{.legacy_url = std::move(legacy_url),
//                     .test_component_svc = std::move(component_svc),
//                     .component_pkg = std::move(component_pkg),
//                     .ns = std::move(ns)});
// }

}  // namespace

namespace dart_runner {

namespace {

const char* kDartVMArgs[] = {
    // clang-format off
    "--lazy_async_stacks",

    "--systrace_timeline",
    "--timeline_streams=Compiler,Dart,Debugger,Embedder,GC,Isolate,VM",

#if defined(AOT_RUNTIME)
    "--precompilation",
#else
    "--enable_mirrors=false",
#endif

    // No asserts in debug or release product.
    // No asserts in release with flutter_profile=true (non-product)
    // Yes asserts in non-product debug.
#if !defined(DART_PRODUCT) && (!defined(FLUTTER_PROFILE) || !defined(NDEBUG))
    "--enable_asserts",
#endif
    // clang-format on
};

Dart_Isolate IsolateGroupCreateCallback(const char* uri,
                                        const char* name,
                                        const char* package_root,
                                        const char* package_config,
                                        Dart_IsolateFlags* flags,
                                        void* callback_data,
                                        char** error) {
  if (std::string(uri) == DART_VM_SERVICE_ISOLATE_NAME) {
#if defined(DART_PRODUCT)
    *error = strdup("The service isolate is not implemented in product mode");
    return NULL;
#else
    return CreateServiceIsolate(uri, flags, error);
#endif
  }

  *error = strdup("Isolate spawning is not implemented in dart_runner");
  return NULL;
}

void IsolateShutdownCallback(void* isolate_group_data, void* isolate_data) {
  // The service isolate (and maybe later the kernel isolate) doesn't have an
  // async loop.
  auto dispatcher = async_get_default_dispatcher();
  auto loop = async_loop_from_dispatcher(dispatcher);
  if (loop) {
    tonic::DartMicrotaskQueue* queue =
        tonic::DartMicrotaskQueue::GetForCurrentThread();
    if (queue) {
      queue->Destroy();
    }

    async_loop_quit(loop);
  }

  auto state =
      static_cast<std::shared_ptr<tonic::DartState>*>(isolate_group_data);
  state->get()->SetIsShuttingDown();
}

void IsolateGroupCleanupCallback(void* isolate_group_data) {
  delete static_cast<std::shared_ptr<tonic::DartState>*>(isolate_group_data);
}

// Runs the application for a V1 component.
void RunApplicationV1(
    DartRunner* runner,
    fuchsia::sys::Package package,
    fuchsia::sys::StartupInfo startup_info,
    std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
    ::fidl::InterfaceRequest<fuchsia::sys::ComponentController> controller) {
  int64_t start = Dart_TimelineGetMicros();

  DartComponentController app(std::move(package), std::move(startup_info),
                              runner_incoming_services, std::move(controller));
  bool success = app.Setup();

  int64_t end = Dart_TimelineGetMicros();
  Dart_TimelineEvent("DartComponentController::Setup", start, end,
                     Dart_Timeline_Event_Duration, 0, NULL, NULL);
  if (success) {
    app.Run();
  }

  if (Dart_CurrentIsolate()) {
    Dart_ShutdownIsolate();
  }
}

// Runs the application for a V2 component.
void RunApplicationV2(
    DartRunner* runner,
    fuchsia::component::runner::ComponentStartInfo start_info,
    std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
    fidl::InterfaceRequest<fuchsia::component::runner::ComponentController>
        controller) {
  const int64_t start = Dart_TimelineGetMicros();

  DartComponentControllerV2 app(std::move(start_info), runner_incoming_services,
                                std::move(controller));
  const bool success = app.SetUp();

  const int64_t end = Dart_TimelineGetMicros();
  Dart_TimelineEvent("DartComponentControllerV2::Setup", start, end,
                     Dart_Timeline_Event_Duration, 0, NULL, NULL);
  if (success) {
    app.Run();
  }

  if (Dart_CurrentIsolate()) {
    Dart_ShutdownIsolate();
  }
}

void RunTestApplicationV2(
    DartRunner* runner,
    fuchsia::component::runner::ComponentStartInfo start_info,
    std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
    fidl::InterfaceRequest<fuchsia::component::runner::ComponentController>
        controller,
    fit::function<void(std::unique_ptr<DartTestComponentControllerV2>)>
        create_callback,
    fit::function<void(DartTestComponentControllerV2*)> done_callback) {
  const int64_t start = Dart_TimelineGetMicros();

  auto test_component = std::make_unique<DartTestComponentControllerV2>(
      std::move(start_info), runner_incoming_services, std::move(controller),
      std::move(done_callback));

  // DartTestComponentControllerV2 app(
  //     std::move(start_info), runner_incoming_services,
  //     std::move(controller));
  test_component->SetUp();
  // const bool success = app.SetUp();
  FML_LOG(INFO) << "DART COMPONENT SET UP: ";
  const int64_t end = Dart_TimelineGetMicros();
  Dart_TimelineEvent("DartTestComponentControllerV2::Setup", start, end,
                     Dart_Timeline_Event_Duration, 0, NULL, NULL);
  create_callback(std::move(test_component));

  // app.Run();

  if (Dart_CurrentIsolate()) {
    Dart_ShutdownIsolate();
  }
}

bool EntropySource(uint8_t* buffer, intptr_t count) {
  zx_cprng_draw(buffer, count);
  return true;
}

}  // namespace

// "data" and "assets" are arguments that are specific to the Flutter runner.
// They will likely go away if we migrate to the ELF runner.
constexpr char kDataKey[] = "data";
constexpr char kAssetsKey[] = "assets";

// "args" are how the component specifies arguments to the runner.
constexpr char kArgsKey[] = "args";
constexpr char kOldGenHeapSizeKey[] = "old_gen_heap_size";
constexpr char kExposeDirsKey[] = "expose_dirs";

// constexpr char kTmpPath[] = "/tmp";
// constexpr char kServiceRootPath[] = "/svc";
// constexpr char kRunnerConfigPath[] = "/config/data/flutter_runner_config";

/// Parses the |args| field from the "program" field into
/// |metadata|.
void ParseArgs(std::vector<std::string>& args, ProgramMetadata* metadata) {
  // fml::CommandLine expects the first argument to be the name of the program,
  // so we prepend a dummy argument so we can use fml::CommandLine to parse the
  // arguments for us.
  std::vector<std::string> command_line_args = {""};
  command_line_args.insert(command_line_args.end(), args.begin(), args.end());
  fml::CommandLine parsed_args = fml::CommandLineFromIterators(
      command_line_args.begin(), command_line_args.end());

  std::string is_test;
  if (parsed_args.GetOptionValue("is_test", &is_test)) {
    // FML_LOG(INFO) << "HIT";
    // FML_LOG(INFO) << is_test;
    metadata->is_test = is_test == "true";
  }

  std::string old_gen_heap_size_option;
  if (parsed_args.GetOptionValue(kOldGenHeapSizeKey,
                                 &old_gen_heap_size_option)) {
    int64_t specified_old_gen_heap_size = strtol(
        old_gen_heap_size_option.c_str(), nullptr /* endptr */, 10 /* base */);
    if (specified_old_gen_heap_size != 0) {
      metadata->old_gen_heap_size = specified_old_gen_heap_size;
    } else {
      // FML_LOG(ERROR) << "Invalid old_gen_heap_size: "
      //                << old_gen_heap_size_option;
    }
  }

  std::string expose_dirs_option;
  if (parsed_args.GetOptionValue(kExposeDirsKey, &expose_dirs_option)) {
    // Parse the comma delimited string
    std::vector<std::string> expose_dirs;
    std::stringstream s(expose_dirs_option);
    while (s.good()) {
      std::string dir;
      getline(s, dir, ',');  // get first string delimited by comma
      metadata->expose_dirs.push_back(dir);
    }
  }
}

ProgramMetadata ParseProgramMetadata(
    const fuchsia::data::Dictionary& program_metadata) {
  ProgramMetadata result;

  for (const auto& entry : program_metadata.entries()) {
    if (entry.key.compare(kDataKey) == 0 && entry.value != nullptr) {
      result.data_path = "pkg/" + entry.value->str();
    } else if (entry.key.compare(kAssetsKey) == 0 && entry.value != nullptr) {
      result.assets_path = "pkg/" + entry.value->str();
    } else if (entry.key.compare(kArgsKey) == 0 && entry.value != nullptr) {
      ParseArgs(entry.value->str_vec(), &result);
    }
  }

  // assets_path defaults to the same as data_path if omitted.
  if (result.assets_path.empty()) {
    result.assets_path = result.data_path;
  }

  return result;
}

DartRunner::DartRunner(sys::ComponentContext* context) : context_(context) {
  context_->outgoing()->AddPublicService<fuchsia::sys::Runner>(
      [this](fidl::InterfaceRequest<fuchsia::sys::Runner> request) {
        bindings_.AddBinding(this, std::move(request));
      });

  context_->outgoing()
      ->AddPublicService<fuchsia::component::runner::ComponentRunner>(
          [this](fidl::InterfaceRequest<
                 fuchsia::component::runner::ComponentRunner> request) {
            component_runner_bindings_.AddBinding(this, std::move(request));
          });

#if !defined(DART_PRODUCT)
  // The VM service isolate uses the process-wide namespace. It writes the
  // vm service protocol port under /tmp. The VMServiceObject exposes that
  // port number to The Hub.
  context_->outgoing()->debug_dir()->AddEntry(
      dart_utils::VMServiceObject::kPortDirName,
      std::make_unique<dart_utils::VMServiceObject>());

#endif  // !defined(DART_PRODUCT)

  dart::bin::BootstrapDartIo();

  char* error =
      Dart_SetVMFlags(dart_utils::ArraySize(kDartVMArgs), kDartVMArgs);
  if (error) {
    FX_LOGF(FATAL, LOG_TAG, "Dart_SetVMFlags failed: %s", error);
  }

  Dart_InitializeParams params = {};
  params.version = DART_INITIALIZE_PARAMS_CURRENT_VERSION;
#if defined(AOT_RUNTIME)
  params.vm_snapshot_data = ::_kDartVmSnapshotData;
  params.vm_snapshot_instructions = ::_kDartVmSnapshotInstructions;
#else
  if (!dart_utils::MappedResource::LoadFromNamespace(
          nullptr, "/pkg/data/vm_snapshot_data.bin", vm_snapshot_data_)) {
    FX_LOG(FATAL, LOG_TAG, "Failed to load vm snapshot data");
  }
  if (!dart_utils::MappedResource::LoadFromNamespace(
          nullptr, "/pkg/data/vm_snapshot_instructions.bin",
          vm_snapshot_instructions_, true /* executable */)) {
    FX_LOG(FATAL, LOG_TAG, "Failed to load vm snapshot instructions");
  }
  params.vm_snapshot_data = vm_snapshot_data_.address();
  params.vm_snapshot_instructions = vm_snapshot_instructions_.address();
#endif
  params.create_group = IsolateGroupCreateCallback;
  params.shutdown_isolate = IsolateShutdownCallback;
  params.cleanup_group = IsolateGroupCleanupCallback;
  params.entropy_source = EntropySource;
#if !defined(DART_PRODUCT)
  params.get_service_assets = GetVMServiceAssetsArchiveCallback;
#endif
  error = Dart_Initialize(&params);
  if (error)
    FX_LOGF(FATAL, LOG_TAG, "Dart_Initialize failed: %s", error);
}

DartRunner::~DartRunner() {
  char* error = Dart_Cleanup();
  if (error)
    FX_LOGF(FATAL, LOG_TAG, "Dart_Cleanup failed: %s", error);
}

void DartRunner::StartComponent(
    fuchsia::sys::Package package,
    fuchsia::sys::StartupInfo startup_info,
    fidl::InterfaceRequest<fuchsia::sys::ComponentController> controller) {
  // TRACE_DURATION currently requires that the string data does not change
  // in the traced scope. Since |package| gets moved in the construction of
  // |thread| below, we cannot ensure that |package.resolved_url| does not
  // move or change, so we make a copy to pass to TRACE_DURATION.
  // TODO(PT-169): Remove this copy when TRACE_DURATION reads string arguments
  // eagerly.
  FML_LOG(INFO) << "RESOLVED URL V1: " << package.resolved_url;
  std::string url_copy = package.resolved_url;
  TRACE_EVENT1("dart", "StartComponent", "url", url_copy.c_str());
  std::thread thread(RunApplicationV1, this, std::move(package),
                     std::move(startup_info), context_->svc(),
                     std::move(controller));
  thread.detach();
}

void DartRunner::Start(
    fuchsia::component::runner::ComponentStartInfo start_info,
    fidl::InterfaceRequest<fuchsia::component::runner::ComponentController>
        controller) {
  ProgramMetadata metadata = ParseProgramMetadata(start_info.program());
  FML_LOG(INFO) << "RESOLVED URL V2: " << start_info.resolved_url();

  if (metadata.is_test) {
    FML_LOG(INFO) << "RESOLVED URL: " << start_info.resolved_url();

    std::string url_copy = start_info.resolved_url();
    TRACE_EVENT1("dart", "Start", "url", url_copy.c_str());
    std::thread thread(
        RunTestApplicationV2, this, std::move(start_info), context_->svc(),
        std::move(controller),
        [this](std::unique_ptr<DartTestComponentControllerV2> ptr) {
          FML_LOG(INFO) << "TEST COMPONENT SAVED";
          test_components_.emplace(ptr.get(), std::move(ptr));
        },
        [this](DartTestComponentControllerV2* ptr) {
          FML_LOG(INFO) << "TEST COMPONENT ERASED";
          auto it = test_components_.find(ptr);
          if (it != test_components_.end()) {
            test_components_.erase(it);
          }
        });
    thread.detach();
  } else {
    std::string url_copy = start_info.resolved_url();
    TRACE_EVENT1("dart", "Start", "url", url_copy.c_str());
    std::thread thread(RunApplicationV2, this, std::move(start_info),
                       context_->svc(), std::move(controller));
    thread.detach();
  }
}

}  // namespace dart_runner
