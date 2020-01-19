// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/runner.h"

#include <fcntl.h>
#include <lib/async/cpp/task.h>
#include <lib/trace/event.h>
#include <unistd.h>

#include <cstring>  // For strerror

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/shell/platform/fuchsia/utils/thread.h"

#if !defined(DART_PRODUCT)
#include "flutter/shell/platform/fuchsia/utils/files.h"
#include "flutter/shell/platform/fuchsia/utils/vmservice_object.h"
#include "third_party/dart/runtime/include/dart_api.h"
#endif  // !defined(DART_PRODUCT)

namespace flutter_runner {
namespace {

// Environment variable containing the path to the directory containing the
// timezone files.
constexpr char kICUTZEnv[] = "ICU_TIMEZONE_FILES_DIR";

// The data directory containing ICU timezone data files.
constexpr char kICUTZDataDir[] = "/config/data/tzdata/icu/44/le";

#if !defined(DART_PRODUCT)
// Register native symbol information for the Dart VM's profiler.
void RegisterProfilerSymbols(const char* symbols_path, const char* dso_name) {
  std::string* symbols = new std::string();
  if (fx::ReadFileToString(symbols_path, symbols)) {
    Dart_AddSymbols(dso_name, symbols->data(), symbols->size());
  } else {
    FX_LOG(ERROR) << "Failed to load " << symbols_path;
  }
}
#endif  // !defined(DART_PRODUCT)

void SetMainThreadAndProcessName() {
#if defined(DART_PRODUCT)
  std::string runner_type = "io.flutter.product_runner";
#else
  std::string runner_type = "io.flutter.runner";
#endif

  if (FlutterEngineRunsAOTCompiledDartCode()) {
    fx::Thread::SetProcessName(runner_type + ".aot");
  } else {
    fx::Thread::SetProcessName(runner_type + ".jit");
  }
  fx::Thread::SetCurrentThreadName(runner_type + ".main");
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
    FX_LOG(ERROR) << "Could not open '" << tzdata_dir.c_str()
                  << "', error: " << strerror(errno)
                  << "; proceeding without loading the timezone database.";
    return false;
  }
  if (!close(fd)) {
    FX_LOG(WARNING) << "Could not close '" << tzdata_dir.c_str()
                    << "', error: " << strerror(errno) << ".";
  }
  return true;
}

std::string DebugLabelForURL(const std::string& url) {
  auto found = url.rfind("/");
  if (found == std::string::npos) {
    return url;
  } else {
    return {url, found + 1};
  }
}

}  // namespace

Runner::Runner(Renderer::FactoryCallback renderer_factory, async::Loop* loop)
    : renderer_factory_(std::move(renderer_factory)),
      runner_context_(sys::ComponentContext::Create()),
      loop_(loop) {
#if !defined(DART_PRODUCT)
  StartTraceObserver();

  // The VM service isolate uses the process-wide namespace. It writes the
  // vm service protocol port under /tmp. The VMServiceObject exposes that
  // port number to The Hub.
  runner_context_->outgoing()->debug_dir()->AddEntry(
      fx::VMServiceObject::kPortDirName,
      std::make_unique<fx::VMServiceObject>());

  if (FlutterEngineRunsAOTCompiledDartCode()) {
    RegisterProfilerSymbols("/pkg/data/flutter_aot_runner.dartprofilersymbols",
                            "");
  } else {
    RegisterProfilerSymbols("/pkg/data/flutter_jit_runner.dartprofilersymbols",
                            "");
  }
#endif  // !defined(DART_PRODUCT)

  SetMainThreadAndProcessName();

  InitializeTZData();

  runner_context_->outgoing()->AddPublicService<fuchsia::sys::Runner>(
      [this](fidl::InterfaceRequest<fuchsia::sys::Runner> request) {
        runner_bindings_.AddBinding(this, std::move(request));
      });

  //   // Connect to the intl property provider.  If the connection fails, the
  //   // initialization of the engine will simply proceed, printing a warning
  //   // message.  The engine will be fully functional, except that the user's
  //   // locale preferences would not be communicated to flutter engine.
  //   {
  //     intl_property_provider_.set_error_handler([](zx_status_t status) {
  //       FX_LOG(WARNING) << "Failed to connect to " <<
  //       fuchsia::intl::PropertyProvider::Name_ << ": "
  //                       << zx_status_get_string(status) << "This is not a
  //                       fatal error, but the "
  //                       << "user locale preferences will not be forwarded to
  //                       Flutter apps.";
  //     });

  //     // Note that we're using the runner's services, not the component's.
  //     // Flutter locales should be updated regardless of whether the
  //     component has
  //     // direct access to the fuchsia.intl.PropertyProvider service.
  //     ZX_ASSERT(runner_incoming_services->Connect(
  //                   intl_property_provider_.NewRequest()) == ZX_OK);

  //     auto get_profile_callback = [this](const fuchsia::intl::Profile&
  //     profile) {
  //       if (!profile.has_locales()) {
  //         FX_LOG(WARNING) << "Got intl Profile without locales";
  //       }

  //       FX_VLOG(-1) << "Sending LocalizationPlatformMessage";
  //       std::vector<FlutterLocale> locales_holder(profile.locales().size());
  //       std::vector<const FlutterLocale*> locales;
  //       for (const auto& locale_id : profile.locales()) {
  //         UErrorCode error_code = U_ZERO_ERROR;
  //         icu::Locale icu_locale =
  //             icu::Locale::forLanguageTag(locale_id.id, error_code);
  //         if (U_FAILURE(error_code)) {
  //           FX_LOG(ERROR) << "Error parsing locale ID " <<  locale_id.id;
  //           continue;
  //         }

  //         std::string country =
  //             icu_locale.getCountry() != nullptr ? icu_locale.getCountry() :
  //             "";
  //         std::string script =
  //             icu_locale.getScript() != nullptr ? icu_locale.getScript() :
  //             "";
  //         std::string variant =
  //             icu_locale.getVariant() != nullptr ? icu_locale.getVariant() :
  //             "";
  //         // ICU4C capitalizes the variant for backward compatibility, even
  //         though
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

  //       FlutterEngineUpdateLocales(this, locales.data(), locales.size());
  //     };

  //     FX_VLOG(-1) << "Requesting intl Profile";

  //     // Make the initial request
  //     intl_property_provider_->GetProfile(get_profile_callback);

  //     // And register for changes
  //     intl_property_provider_.events().OnChange =
  //         [this, get_profile_callback]() {
  //           FX_VLOG(-1) << fuchsia::intl::PropertyProvider::Name_
  //                       << ": OnChange";
  //           intl_property_provider_->GetProfile(get_profile_callback);
  //         };
  //   }

  FX_DLOG(INFO) << "Flutter runner services initialized.";
}

Runner::~Runner() {
  FX_DLOG(INFO) << "Flutter runner services stopped.";

  runner_context_->outgoing()->RemovePublicService<fuchsia::sys::Runner>();

#if !defined(DART_PRODUCT)
  StopTraceObserver();
#endif  // !defined(DART_PRODUCT)
}

void Runner::StartComponent(
    fuchsia::sys::Package package,
    fuchsia::sys::StartupInfo startup_info,
    fidl::InterfaceRequest<fuchsia::sys::ComponentController>
        component_controller) {
  // TRACE_DURATION currently requires that the string data does not change
  // in the traced scope. Since |package.resolved_url| gets moved in the Create
  // call below, we make a copy to pass to TRACE_DURATION.
  // TODO(PT-169): Remove this copy when TRACE_DURATION reads string arguments
  // eagerly.
  std::string url_copy = package.resolved_url;
  TRACE_DURATION("flutter", "StartComponent", "url", url_copy.c_str());

  Component::Context component_context = {
      .startup_info = std::move(startup_info),
      .debug_label = DebugLabelForURL(package.resolved_url),
      .component_url = std::move(package.resolved_url),
      .controller_request = std::move(component_controller),
      .incoming_services = runner_context_->svc(),
      .renderer_factory_callback = renderer_factory_,
      .termination_callback =
          std::bind(&Runner::OnComponentTerminate, this, std::placeholders::_1),
  };

  // Create a new component (synchronously on its own thread) and add it to the
  // list of active components.
  components_.emplace_back(Component::Create(std::move(component_context)));
}

void Runner::OnComponentTerminate(const Component* component) {
  // The runner must remove the component on the runner's thread, but this
  // callback is fired from the component's platform thread.  Post a task to
  // the runner's thread to take care of things.
  async::PostTask(loop_->dispatcher(), [this, component]() {
    auto found_component =
        std::find_if(components_.begin(), components_.end(),
                     [component](const auto& active_component) -> bool {
                       return component == active_component.get();
                     });
    if (found_component == components_.end()) {
      FX_LOG(ERROR)
          << "The remote end of the component runner tried to terminate an "
             "component that has already been terminated, possibly because we "
             "initiated the termination.";
      return;
    }

    // Delete the list entry by swapping it with the back and popping.  The
    // component will destroy itself synchronously on its platform thread.
    FX_DCHECK(!components_.empty());
    if (components_.size() > 1) {
      std::swap(*found_component, components_.back());
    }
    components_.pop_back();
  });
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
      for (auto& component : components_) {
        component->WriteProfileToTrace();
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
