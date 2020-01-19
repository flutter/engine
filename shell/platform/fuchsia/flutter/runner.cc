// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/runner.h"

#include <fuchsia/mem/cpp/fidl.h>
#include <lib/async/default.h>
#include <lib/async/cpp/task.h>
#include <lib/zx/process.h>
#include <lib/zx/thread.h>
#include <lib/zx/vmar.h>
#include <zircon/status.h>
#include <zircon/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/files.h"
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/vmo.h"
#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/icu/source/common/unicode/udata.h"
#include "third_party/skia/include/core/SkGraphics.h"

namespace flutter_runner {
namespace {

constexpr char kIcuDataPath[] = "/pkg/data/icudtl.dat";

// Environment variable containing the path to the directory containing the
// timezone files.
constexpr char kICUTZEnv[] = "ICU_TIMEZONE_FILES_DIR";

// The data directory containing ICU timezone data files.
constexpr char kICUTZDataDir[] = "/config/data/tzdata/icu/44/le";

// Map the memory into the process and return a pointer to the memory.
uintptr_t GetICUData(const fuchsia::mem::Buffer& icu_data) {
  uint64_t data_size = icu_data.size;
  if (data_size > std::numeric_limits<size_t>::max())
    return 0u;

  uintptr_t data = 0u;
  zx_status_t status = zx::vmar::root_self()->map(
      0, icu_data.vmo, 0, static_cast<size_t>(data_size), ZX_VM_PERM_READ,
      &data);
  if (status == ZX_OK) {
    return data;
  }

  return 0u;
}

// Initializes the timezone data if available.  Timezone data file in Fuchsia
// is at a fixed directory path.  Returns true on success.  As a side effect
// sets the value of the environment variable "ICU_TIMEZONE_FILES_DIR" to a
// fixed value which is fuchsia-specific.
bool InitializeTZData() {
  // We need the ability to change the env variable for testing, so not
  // overwriting if set.
  setenv(kICUTZEnv, kICUTZDataDir, 0 /* No overwrite */);

  const std::string tzdata_dir = getenv(kICUTZEnv);
  // Try opening the path to check if present.  No need to verify that it is a
  // directory since ICU loading will return an error if the TZ data path is
  // wrong.
  int fd = openat(AT_FDCWD, tzdata_dir.c_str(), O_RDONLY);
  if (fd < 0) {
    FML_LOG(INFO) << "Could not open: '" << tzdata_dir
                  << "', proceeding without loading the timezone database: "
                  << strerror(errno);
    return false;
  }
  if (!close(fd)) {
    FML_LOG(WARNING) << "Could not close: " << tzdata_dir << ": "
                     << strerror(errno);
  }
  return true;
}

// Return value indicates if initialization was successful.
bool InitializeICU() {
  const char* data_path = kIcuDataPath;

  fuchsia::mem::Buffer icu_data;
  if (!dart_utils::VmoFromFilename(data_path, &icu_data)) {
    return false;
  }

  uintptr_t data = GetICUData(icu_data);
  if (!data) {
    return false;
  }

  // If the loading fails, soldier on.  The loading is optional as we don't
  // want to crash the engine in transition.
  InitializeTZData();

  // Pass the data to ICU.
  UErrorCode err = U_ZERO_ERROR;
  udata_setCommonData(reinterpret_cast<const char*>(data), &err);
  if (err != U_ZERO_ERROR) {
    FML_LOG(ERROR) << "error loading ICU data: " << err;
    return false;
  }
  return true;
}

void SetupICU() {
  InitializeTZData();
  if (!InitializeICU()) {
    FML_LOG(ERROR) << "Could not initialize ICU data.";
  }
}

void SetProcessName() {
  std::stringstream stream;
#if defined(DART_PRODUCT)
  stream << "io.flutter.product_runner.";
#else
  stream << "io.flutter.runner.";
#endif
  if (FlutterEngineRunsAOTCompiledDartCode()) {
    stream << "aot";
  } else {
    stream << "jit";
  }
  const auto name = stream.str();
  zx::process::self()->set_property(ZX_PROP_NAME, name.c_str(), name.size());
}

void SetThreadName(const std::string& thread_name) {
  zx::thread::self()->set_property(ZX_PROP_NAME, thread_name.c_str(),
                                   thread_name.size());
}

#if !defined(DART_PRODUCT)
// Register native symbol information for the Dart VM's profiler.
void RegisterProfilerSymbols(const char* symbols_path, const char* dso_name) {
  std::string* symbols = new std::string();
  if (dart_utils::ReadFileToString(symbols_path, symbols)) {
    Dart_AddSymbols(dso_name, symbols->data(), symbols->size());
  } else {
    FML_LOG(ERROR) << "Failed to load " << symbols_path;
  }
}
#endif  // !defined(DART_PRODUCT)

}  // namespace

Runner::Runner(async::Loop* loop)
    : context_(sys::ComponentContext::Create()), loop_(loop) {
#if !defined(DART_PRODUCT)
  StartTraceObserver();

  // The VM service isolate uses the process-wide namespace. It writes the
  // vm service protocol port under /tmp. The VMServiceObject exposes that
  // port number to The Hub.
  context_->outgoing()->debug_dir()->AddEntry(
      dart_utils::VMServiceObject::kPortDirName,
      std::make_unique<dart_utils::VMServiceObject>());

  if (Dart_IsPrecompiledRuntime()) {
    RegisterProfilerSymbols("pkg/data/flutter_aot_runner.dartprofilersymbols",
                            "");
  } else {
    RegisterProfilerSymbols("pkg/data/flutter_jit_runner.dartprofilersymbols",
                            "");
  }
#endif  // !defined(DART_PRODUCT)

  SkGraphics::Init();

  SetupICU();

  SetProcessName();

  SetThreadName("io.flutter.runner.main");

  context_->outgoing()->AddPublicService<fuchsia::sys::Runner>(
      [this](fidl::InterfaceRequest<fuchsia::sys::Runner> request) {
        runner_bindings_.AddBinding(this, std::move(request));
      });
}

Runner::~Runner() {
  context_->outgoing()->RemovePublicService<fuchsia::sys::Runner>();

#if !defined(DART_PRODUCT)
  StopTraceObserver();
#endif  // !defined(DART_PRODUCT)
}

void Runner::StartComponent(
    fuchsia::sys::Package package,
    fuchsia::sys::StartupInfo startup_info,
    fidl::InterfaceRequest<fuchsia::sys::ComponentController> controller) {
  // TRACE_DURATION currently requires that the string data does not change
  // in the traced scope. Since |package| gets moved in the CreateComponent
  // call below, we cannot ensure that |package.resolved_url| does not move or
  // change, so we make a copy to pass to TRACE_DURATION.
  // TODO(PT-169): Remove this copy when TRACE_DURATION reads string arguments
  // eagerly.
  std::string url_copy = package.resolved_url;
  TRACE_EVENT1("flutter", "StartComponent", "url", url_copy.c_str());

  // Note on component termination: Component typically terminate on the thread
  // on which they were created. This means using the thread was specifically
  // created to host the component. However, we want to ensure that access to
  // the active components collection is only made on the runner thread.
  // Therefore, we capture this and post a callback when the component is
  // terminated.
  Component::TerminationCallback termination_callback =
      [this, dispatcher = loop_->dispatcher()](const Component* component) {
         async::PostTask(dispatcher, [this, component]() {
          OnComponentTerminate(component);
        });
      };

  // Post a task to actually create the component on the new thread.
  ActiveComponent new_component = {
    .thread = std::make_unique<Thread>(),
  };
  fml::AutoResetWaitableEvent latch;
  async::PostTask(new_component.thread->dispatcher(), [&]() mutable {
    new_component.component = std::make_unique<Component>(
        std::move(termination_callback), std::move(package),
        std::move(startup_info), context_->svc(),
        std::move(controller));
    latch.Signal();
  });
  latch.Wait();

  active_components_.emplace_back(std::move(new_component));
}

void Runner::OnComponentTerminate(const Component* component) {
  auto active_component = std::find_if(active_components_.begin(),
                                       active_components_.end(),
                                       [component](const auto& active_component) -> bool {
                                         return component == active_component.component.get();
                                       });
  if (active_component == active_components_.end()) {
    FML_LOG(INFO)
        << "The remote end of the component runner tried to terminate an "
           "component that has already been terminated, possibly because we "
           "initiated the termination.";
    return;
  }

  // Grab the items out of the entry because we will have to rethread the
  // destruction.
  auto zombie_component = std::move(active_component->component);
  auto zombie_thread = std::move(active_component->thread);

  // Delete the entry by swapping it with the back.
  if (active_components_.size() > 1) {
    std::iter_swap(active_component, active_components_.end() - 1);
  }
  active_components_.pop_back();

  // Post the task to destroy the component and quit its message loop.
  async::PostTask(
      zombie_thread->dispatcher(),
      [&zombie_component, thread = zombie_thread.get()]() mutable {
        zombie_component.reset();
        thread->Quit();
      });

  // This works because we just posted the quit task on the zombie thread.
  zombie_thread->Join();
}

#if !defined(DART_PRODUCT)
void Runner::StartTraceObserver() {
  trace_observer_.Start(loop_->dispatcher(), [this]() {
    if (!trace_is_category_enabled("dart:profiler")) {
      return;
    }
    if (trace_state() == TRACE_STARTED) {
      prolonged_context_ = trace_acquire_prolonged_context();
      Dart_StartProfiling();
    } else if (trace_state() == TRACE_STOPPING) {
      for (auto& it : active_components_) {
        fml::AutoResetWaitableEvent latch;
        async::PostTask(it.thread->dispatcher(), [&]() {
          //it.component->WriteProfileToTrace();
          latch.Signal();
        });
        latch.Wait();
      }
      Dart_StopProfiling();
      trace_release_prolonged_context(prolonged_context_);
    }
  });
}

void Runner::StopTraceObserver() {
  trace_observer_.Stop();
}
#endif  // !defined(DART_PRODUCT)

}  // namespace flutter_runner
