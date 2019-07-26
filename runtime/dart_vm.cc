// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_vm.h"

#include <sys/stat.h>

// These headers should only be needed in debug mode.
#if OS_IOS && (FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG)
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#include <mutex>
#include <vector>

#include "flutter/common/settings.h"
#include "flutter/fml/compiler_specific.h"
#include "flutter/fml/file.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/mapping.h"
#include "flutter/fml/size.h"
#include "flutter/fml/synchronization/count_down_latch.h"
#include "flutter/fml/synchronization/thread_annotations.h"
#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/trace_event.h"
#include "flutter/lib/io/dart_io.h"
#include "flutter/lib/ui/dart_runtime_hooks.h"
#include "flutter/lib/ui/dart_ui.h"
#include "flutter/runtime/dart_isolate.h"
#include "flutter/runtime/dart_service_isolate.h"
#include "flutter/runtime/start_up.h"
#include "third_party/dart/runtime/include/bin/dart_io_api.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_class_library.h"
#include "third_party/tonic/dart_class_provider.h"
#include "third_party/tonic/file_loader/file_loader.h"
#include "third_party/tonic/logging/dart_error.h"
#include "third_party/tonic/scopes/dart_api_scope.h"
#include "third_party/tonic/typed_data/typed_list.h"

namespace dart {
namespace observatory {

#if !OS_FUCHSIA && (FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE)

// These two symbols are defined in |observatory_archive.cc| which is generated
// by the |//third_party/dart/runtime/observatory:archive_observatory| rule.
// Both of these symbols will be part of the data segment and therefore are read
// only.
extern unsigned int observatory_assets_archive_len;
extern const uint8_t* observatory_assets_archive;

#endif  // !OS_FUCHSIA && (FLUTTER_RUNTIME_MODE !=
        // FLUTTER_RUNTIME_MODE_RELEASE)

}  // namespace observatory
}  // namespace dart

namespace flutter {

// Arguments passed to the Dart VM in all configurations.
static const char* kDartLanguageArgs[] = {
    // clang-format off
    "--enable_mirrors=false",
    "--background_compilation",
    "--causal_async_stacks",
    // clang-format on
};

static const char* kDartPrecompilationArgs[] = {
    "--precompilation",
};

FML_ALLOW_UNUSED_TYPE
static const char* kDartWriteProtectCodeArgs[] = {
    "--no_write_protect_code",
};

static const char* kDartAssertArgs[] = {
    // clang-format off
    "--enable_asserts",
    // clang-format on
};

static const char* kDartStartPausedArgs[]{
    "--pause_isolates_on_start",
};

static const char* kDartDisableServiceAuthCodesArgs[]{
    "--disable-service-auth-codes",
};

static const char* kDartTraceStartupArgs[]{
    "--timeline_streams=Compiler,Dart,Debugger,Embedder,GC,Isolate,VM",
};

static const char* kDartEndlessTraceBufferArgs[]{
    "--timeline_recorder=endless",
};

static const char* kDartSystraceTraceBufferArgs[]{
    "--timeline_recorder=systrace",
};

static const char* kDartFuchsiaTraceArgs[] FML_ALLOW_UNUSED_TYPE = {
    "--systrace_timeline",
};

static const char* kDartTraceStreamsArgs[] = {
    "--timeline_streams=Compiler,Dart,Debugger,Embedder,GC,Isolate,VM",
};

constexpr char kFileUriPrefix[] = "file://";
constexpr size_t kFileUriPrefixLength = sizeof(kFileUriPrefix) - 1;

bool DartFileModifiedCallback(const char* source_url, int64_t since_ms) {
  if (strncmp(source_url, kFileUriPrefix, kFileUriPrefixLength) != 0u) {
    // Assume modified.
    return true;
  }

  const char* path = source_url + kFileUriPrefixLength;
  struct stat info;
  if (stat(path, &info) < 0)
    return true;

  // If st_mtime is zero, it's more likely that the file system doesn't support
  // mtime than that the file was actually modified in the 1970s.
  if (!info.st_mtime)
    return true;

  // It's very unclear what time bases we're with here. The Dart API doesn't
  // document the time base for since_ms. Reading the code, the value varies by
  // platform, with a typical source being something like gettimeofday.
  //
  // We add one to st_mtime because st_mtime has less precision than since_ms
  // and we want to treat the file as modified if the since time is between
  // ticks of the mtime.
  fml::TimeDelta mtime = fml::TimeDelta::FromSeconds(info.st_mtime + 1);
  fml::TimeDelta since = fml::TimeDelta::FromMilliseconds(since_ms);

  return mtime > since;
}

void ThreadExitCallback() {}

Dart_Handle GetVMServiceAssetsArchiveCallback() {
#if (FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_RELEASE)
  return nullptr;
#elif OS_FUCHSIA
  fml::UniqueFD fd = fml::OpenFile("pkg/data/observatory.tar", false,
                                   fml::FilePermission::kRead);
  fml::FileMapping mapping(fd, {fml::FileMapping::Protection::kRead});
  if (mapping.GetSize() == 0 || mapping.GetMapping() == nullptr) {
    FML_LOG(ERROR) << "Fail to load Observatory archive";
    return nullptr;
  }
  return tonic::DartConverter<tonic::Uint8List>::ToDart(mapping.GetMapping(),
                                                        mapping.GetSize());
#else
  return tonic::DartConverter<tonic::Uint8List>::ToDart(
      ::dart::observatory::observatory_assets_archive,
      ::dart::observatory::observatory_assets_archive_len);
#endif
}

static const char kStdoutStreamId[] = "Stdout";
static const char kStderrStreamId[] = "Stderr";

static bool ServiceStreamListenCallback(const char* stream_id) {
  if (strcmp(stream_id, kStdoutStreamId) == 0) {
    dart::bin::SetCaptureStdout(true);
    return true;
  } else if (strcmp(stream_id, kStderrStreamId) == 0) {
    dart::bin::SetCaptureStderr(true);
    return true;
  }
  return false;
}

static void ServiceStreamCancelCallback(const char* stream_id) {
  if (strcmp(stream_id, kStdoutStreamId) == 0) {
    dart::bin::SetCaptureStdout(false);
  } else if (strcmp(stream_id, kStderrStreamId) == 0) {
    dart::bin::SetCaptureStderr(false);
  }
}

bool DartVM::IsRunningPrecompiledCode() {
  return Dart_IsPrecompiledRuntime();
}

static std::vector<const char*> ProfilingFlags(bool enable_profiling) {
// Disable Dart's built in profiler when building a debug build. This
// works around a race condition that would sometimes stop a crash's
// stack trace from being printed on Android.
#ifndef NDEBUG
  enable_profiling = false;
#endif

  // We want to disable profiling by default because it overwhelms LLDB. But
  // the VM enables the same by default. In either case, we have some profiling
  // flags.
  if (enable_profiling) {
    return {// This is the default. But just be explicit.
            "--profiler",
            // This instructs the profiler to walk C++ frames, and to include
            // them in the profile.
            "--profile-vm"};
  } else {
    return {"--no-profiler"};
  }
}

void PushBackAll(std::vector<const char*>* args,
                 const char** argv,
                 size_t argc) {
  for (size_t i = 0; i < argc; ++i) {
    args->push_back(argv[i]);
  }
}

static void EmbedderInformationCallback(Dart_EmbedderInformation* info) {
  info->version = DART_EMBEDDER_INFORMATION_CURRENT_VERSION;
  dart::bin::GetIOEmbedderInformation(info);
  info->name = "Flutter";
}

std::shared_ptr<DartVM> DartVM::Create(
    Settings settings,
    fml::RefPtr<DartSnapshot> vm_snapshot,
    fml::RefPtr<DartSnapshot> isolate_snapshot,
    fml::RefPtr<DartSnapshot> shared_snapshot,
    std::shared_ptr<IsolateNameServer> isolate_name_server) {
  auto vm_data = DartVMData::Create(settings,                     //
                                    std::move(vm_snapshot),       //
                                    std::move(isolate_snapshot),  //
                                    std::move(shared_snapshot)    //
  );

  if (!vm_data) {
    FML_LOG(ERROR) << "Could not setup VM data to bootstrap the VM from.";
    return {};
  }

  // Note: std::make_shared unviable due to hidden constructor.
  return std::shared_ptr<DartVM>(
      new DartVM(std::move(vm_data), std::move(isolate_name_server)));
}

static std::atomic_size_t gVMLaunchCount;

size_t DartVM::GetVMLaunchCount() {
  return gVMLaunchCount;
}

// Note: it is __imperative__ that the following two functions are
// __not__ included in release or profile builds.
//
// Please see the following documents for explanation:
//   - go/decommissioning-dbc
//   - go/decommissioning-dbc-engine
//   - go/decommissioning-dbc-tools
#if (FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG)
#define PT_TRACE_ME 0
#define PT_SIGEXC 12
extern "C" int ptrace(int request, pid_t pid, caddr_t addr, int data);

static bool DebuggedIOS(const Settings& vm_settings) {
  // Only the Flutter CLI passes "--enable-checked-mode". Therefore, if the flag
  // is present, we have been launched by "ios-deploy" via "debugserver".
  //
  // We choose this flag because it is always passed to launch debug builds.
  if (vm_settings.enable_checked_mode) {
    return true;
  }

  // Use "sysctl()" to check if we're currently being debugged (e.g. by Xcode).
  // We could also check "getppid() != 1" (launchd), but this is more direct.
  const pid_t self = getpid();
  int mib[5] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, self, 0};

  auto proc = std::make_unique<struct kinfo_proc>();
  size_t proc_size = sizeof(struct kinfo_proc);
  if (sysctl(mib, 4, proc.get(), &proc_size, nullptr, 0) < 0) {
    FML_LOG(ERROR) << "Could not execute sysctl() to get current process info: "
                   << strerror(errno);
    return false;
  }

  return proc->kp_proc.p_flag & P_TRACED;
}

static void EnsureDebuggedIOS(const Settings& vm_settings) {
  if (DebuggedIOS(vm_settings)) {
    return;
  }

  if (ptrace(PT_TRACE_ME, 0, nullptr, 0) == -1) {
    FML_LOG(ERROR) << "Could not call ptrace(PT_TRACE_ME): " << strerror(errno);
    // No use trying PT_SIGEXC -- it's only needed if PT_TRACE_ME succeeds.
    return;
  }
  if (ptrace(PT_SIGEXC, 0, nullptr, 0) == -1) {
    FML_LOG(ERROR) << "Could not call ptrace(PT_SIGEXC): " << strerror(errno);
  }

  // The previous operation causes this process to not be reaped after it
  // terminates (even if PT_SIGEXC fails). Issue a warning to the console every
  // (approximiately) maxproc/10 leaks. See the links above for an explanation
  // of this issue.
  size_t maxproc = 0;
  size_t maxproc_size = sizeof(size_t);
  const int sysctl_result =
      sysctlbyname("kern.maxproc", &maxproc, &maxproc_size, nullptr, 0);
  if (sysctl_result < 0) {
    FML_LOG(ERROR)
        << "Could not execute sysctl() to determine process count limit: "
        << strerror(errno);
  }

  const char* warning =
      "_XXX_ TODO(sjindel): Make a wiki page about this warning. _XXX_";

  if (vm_settings.verbose_logging  // used for testing and also informative
      || sysctl_result < 0         // could not determine maximum process count
      || maxproc / 10 == 0         // avoid division (%) by 0
      || getpid() % (maxproc / 10) == 0)  // warning every ~maxproc/10 leaks
  {
    FML_LOG(ERROR) << warning;
  }
}
#endif  // (FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG)

DartVM::DartVM(std::shared_ptr<const DartVMData> vm_data,
               std::shared_ptr<IsolateNameServer> isolate_name_server)
    : settings_(vm_data->GetSettings()),
      concurrent_message_loop_(fml::ConcurrentMessageLoop::Create()),
      vm_data_(vm_data),
      isolate_name_server_(std::move(isolate_name_server)),
      service_protocol_(std::make_shared<ServiceProtocol>()) {
  TRACE_EVENT0("flutter", "DartVMInitializer");

  gVMLaunchCount++;

  FML_DCHECK(vm_data_);
  FML_DCHECK(isolate_name_server_);
  FML_DCHECK(service_protocol_);

  FML_DLOG(INFO) << "Attempting Dart VM launch for mode: "
                 << (IsRunningPrecompiledCode() ? "AOT" : "Interpreter");

  {
    TRACE_EVENT0("flutter", "dart::bin::BootstrapDartIo");
    dart::bin::BootstrapDartIo();

    if (!settings_.temp_directory_path.empty()) {
      dart::bin::SetSystemTempDirectory(settings_.temp_directory_path.c_str());
    }
  }

  std::vector<const char*> args;

  // Instruct the VM to ignore unrecognized flags.
  // There is a lot of diversity in a lot of combinations when it
  // comes to the arguments the VM supports. And, if the VM comes across a flag
  // it does not recognize, it exits immediately.
  args.push_back("--ignore-unrecognized-flags");

  for (auto* const profiler_flag :
       ProfilingFlags(settings_.enable_dart_profiling)) {
    args.push_back(profiler_flag);
  }

  PushBackAll(&args, kDartLanguageArgs, fml::size(kDartLanguageArgs));

  if (IsRunningPrecompiledCode()) {
    PushBackAll(&args, kDartPrecompilationArgs,
                fml::size(kDartPrecompilationArgs));
  }

  // Enable Dart assertions if we are not running precompiled code. We run non-
  // precompiled code only in the debug product mode.
  bool enable_asserts = !settings_.disable_dart_asserts;

#if !OS_FUCHSIA
  if (IsRunningPrecompiledCode()) {
    enable_asserts = false;
  }
#endif  // !OS_FUCHSIA

#if (FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG)
#if !OS_IOS
  // Debug mode uses the JIT, disable code page write protection to avoid
  // memory page protection changes before and after every compilation.
  PushBackAll(&args, kDartWriteProtectCodeArgs,
              fml::size(kDartWriteProtectCodeArgs));
#else
  EnsureDebuggedIOS(settings_);
#endif
#endif

  if (enable_asserts) {
    PushBackAll(&args, kDartAssertArgs, fml::size(kDartAssertArgs));
  }

  if (settings_.start_paused) {
    PushBackAll(&args, kDartStartPausedArgs, fml::size(kDartStartPausedArgs));
  }

  if (settings_.disable_service_auth_codes) {
    PushBackAll(&args, kDartDisableServiceAuthCodesArgs,
                fml::size(kDartDisableServiceAuthCodesArgs));
  }

  if (settings_.endless_trace_buffer || settings_.trace_startup) {
    // If we are tracing startup, make sure the trace buffer is endless so we
    // don't lose early traces.
    PushBackAll(&args, kDartEndlessTraceBufferArgs,
                fml::size(kDartEndlessTraceBufferArgs));
  }

  if (settings_.trace_systrace) {
    PushBackAll(&args, kDartSystraceTraceBufferArgs,
                fml::size(kDartSystraceTraceBufferArgs));
    PushBackAll(&args, kDartTraceStreamsArgs, fml::size(kDartTraceStreamsArgs));
  }

  if (settings_.trace_startup) {
    PushBackAll(&args, kDartTraceStartupArgs, fml::size(kDartTraceStartupArgs));
  }

#if defined(OS_FUCHSIA)
  PushBackAll(&args, kDartFuchsiaTraceArgs, fml::size(kDartFuchsiaTraceArgs));
  PushBackAll(&args, kDartTraceStreamsArgs, fml::size(kDartTraceStreamsArgs));
#endif

  for (size_t i = 0; i < settings_.dart_flags.size(); i++)
    args.push_back(settings_.dart_flags[i].c_str());

  char* flags_error = Dart_SetVMFlags(args.size(), args.data());
  if (flags_error) {
    FML_LOG(FATAL) << "Error while setting Dart VM flags: " << flags_error;
    ::free(flags_error);
  }

  DartUI::InitForGlobal();

  {
    TRACE_EVENT0("flutter", "Dart_Initialize");
    Dart_InitializeParams params = {};
    params.version = DART_INITIALIZE_PARAMS_CURRENT_VERSION;
    params.vm_snapshot_data = vm_data_->GetVMSnapshot().GetDataMapping();
    params.vm_snapshot_instructions =
        vm_data_->GetVMSnapshot().GetInstructionsMapping();
    params.create_group = reinterpret_cast<decltype(params.create_group)>(
        DartIsolate::DartIsolateGroupCreateCallback);
    params.shutdown_isolate =
        reinterpret_cast<decltype(params.shutdown_isolate)>(
            DartIsolate::DartIsolateShutdownCallback);
    params.cleanup_group = reinterpret_cast<decltype(params.cleanup_group)>(
        DartIsolate::DartIsolateGroupCleanupCallback);
    params.thread_exit = ThreadExitCallback;
    params.get_service_assets = GetVMServiceAssetsArchiveCallback;
    params.entropy_source = dart::bin::GetEntropy;
    char* init_error = Dart_Initialize(&params);
    if (init_error) {
      FML_LOG(FATAL) << "Error while initializing the Dart VM: " << init_error;
      ::free(init_error);
    }
    // Send the earliest available timestamp in the application lifecycle to
    // timeline. The difference between this timestamp and the time we render
    // the very first frame gives us a good idea about Flutter's startup time.
    // Use a duration event so about:tracing will consider this event when
    // deciding the earliest event to use as time 0.
    if (engine_main_enter_ts != 0) {
      Dart_TimelineEvent("FlutterEngineMainEnter",  // label
                         engine_main_enter_ts,      // timestamp0
                         engine_main_enter_ts,      // timestamp1_or_async_id
                         Dart_Timeline_Event_Duration,  // event type
                         0,                             // argument_count
                         nullptr,                       // argument_names
                         nullptr                        // argument_values
      );
    }
  }

  Dart_SetFileModifiedCallback(&DartFileModifiedCallback);

  // Allow streaming of stdout and stderr by the Dart vm.
  Dart_SetServiceStreamCallbacks(&ServiceStreamListenCallback,
                                 &ServiceStreamCancelCallback);

  Dart_SetEmbedderInformationCallback(&EmbedderInformationCallback);

  if (settings_.dart_library_sources_kernel != nullptr) {
    std::unique_ptr<fml::Mapping> dart_library_sources =
        settings_.dart_library_sources_kernel();
    // Set sources for dart:* libraries for debugging.
    Dart_SetDartLibrarySourcesKernel(dart_library_sources->GetMapping(),
                                     dart_library_sources->GetSize());
  }

  FML_DLOG(INFO) << "New Dart VM instance created. Instance count: "
                 << gVMLaunchCount;
}

DartVM::~DartVM() {
  if (Dart_CurrentIsolate() != nullptr) {
    Dart_ExitIsolate();
  }

  char* result = Dart_Cleanup();

  dart::bin::CleanupDartIo();

  FML_CHECK(result == nullptr)
      << "Could not cleanly shut down the Dart VM. Error: \"" << result
      << "\".";
  free(result);

  FML_DLOG(INFO) << "Dart VM instance destroyed. Instance count: "
                 << gVMLaunchCount;
}

std::shared_ptr<const DartVMData> DartVM::GetVMData() const {
  return vm_data_;
}

const Settings& DartVM::GetSettings() const {
  return settings_;
}

std::shared_ptr<ServiceProtocol> DartVM::GetServiceProtocol() const {
  return service_protocol_;
}

std::shared_ptr<IsolateNameServer> DartVM::GetIsolateNameServer() const {
  return isolate_name_server_;
}

std::shared_ptr<fml::ConcurrentTaskRunner>
DartVM::GetConcurrentWorkerTaskRunner() const {
  return concurrent_message_loop_->GetTaskRunner();
}

}  // namespace flutter
