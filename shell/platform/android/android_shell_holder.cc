// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/shell/platform/android/android_shell_holder.h"

#include <pthread.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <memory>
#include <optional>

#include <sstream>
#include <string>
#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/native_library.h"
#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/lib/ui/painting/image_generator_registry.h"
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/common/run_configuration.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/shell/platform/android/android_display.h"
#include "flutter/shell/platform/android/android_image_generator.h"
#include "flutter/shell/platform/android/context/android_context.h"
#include "flutter/shell/platform/android/platform_view_android.h"

namespace flutter {

/// The CPU Affinity provides a hint to the operating system on which cores a
/// particular thread should be scheduled on. The operating system may or may
/// not honor these requests.
enum class CpuAffinity {
  /// @brief Request CPU affinity for the performance cores.
  ///
  ///        Generally speaking, only the UI and Raster thread should
  ///        use this option.
  kPerformance,

  /// @brief Request CPU affinity for the efficiency cores.
  kEfficiency,

  /// @brief Request affinity for all non-performance cores.
  kNotPerformance,
};

struct CpuIndexAndSpeed {
  size_t index;
  int32_t speed;
};

class CPUSpeedTracker {
 public:
  explicit CPUSpeedTracker(std::vector<CpuIndexAndSpeed> data)
      : cpu_speeds_(data) {
    std::optional<int32_t> max_speed = std::nullopt;
    std::optional<int32_t> min_speed = std::nullopt;
    for (const auto& data : cpu_speeds_) {
      if (!max_speed.has_value() || data.speed > max_speed.value()) {
        max_speed = data.speed;
      }
      if (!min_speed.has_value() || data.speed < min_speed.value()) {
        min_speed = data.speed;
      }
    }
    if (!max_speed.has_value() || !min_speed.has_value() ||
        min_speed.value() == max_speed.value()) {
      return;
    }
    for (const auto& data : cpu_speeds_) {
      if (data.speed == max_speed.value()) {
        performance_.push_back(data.index);
      } else {
        not_performance_.push_back(data.index);
      }
      if (data.speed == min_speed.value()) {
        efficiency_.push_back(data.index);
      }
    }

    valid_ = true;
  }

  bool IsValid() const { return valid_; }

  const std::vector<size_t>& GetIndexes(CpuAffinity affinity) const {
    switch (affinity) {
      case CpuAffinity::kPerformance:
        return performance_;
      case CpuAffinity::kEfficiency:
        return efficiency_;
      case CpuAffinity::kNotPerformance:
        return not_performance_;
    }
  }

 private:
  bool valid_ = false;
  std::vector<CpuIndexAndSpeed> cpu_speeds_;
  std::vector<size_t> efficiency_;
  std::vector<size_t> performance_;
  std::vector<size_t> not_performance_;
};

std::once_flag cpu_tracker_flag_;
static CPUSpeedTracker* tracker_;

void InitCPUInfo(size_t cpu_count) {
  std::vector<CpuIndexAndSpeed> cpu_speeds;

  // Get the size of the cpuinfo file by reading it until the end. This is
  // required because files under /proc do not always return a valid size
  // when using fseek(0, SEEK_END) + ftell(). Nor can they be mmap()-ed.
  for (auto i = 0u; i < cpu_count; i++) {
    int data_length = 0;
    auto path = "/sys/devices/system/cpu/cpu" + std::to_string(i) +
                "/cpufreq/cpuinfo_max_freq";
    FILE* fp = fopen(path.c_str(), "r");
    if (fp != nullptr) {
      for (;;) {
        char buffer[256];
        size_t n = fread(buffer, 1, sizeof(buffer), fp);
        if (n == 0) {
          break;
        }
        data_length += n;
      }
      fclose(fp);
    }

    // Read the contents of the cpuinfo file.
    char* data = reinterpret_cast<char*>(malloc(data_length + 1));
    fp = fopen(path.c_str(), "r");
    if (fp != nullptr) {
      for (intptr_t offset = 0; offset < data_length;) {
        size_t n = fread(data + offset, 1, data_length - offset, fp);
        if (n == 0) {
          break;
        }
        offset += n;
      }
      fclose(fp);
    }

    // Parse the CPU info numbers.
    if (data != nullptr) {
      auto speed = std::stoi(data);
      if (speed > 0) {
        cpu_speeds.push_back({.index = i, .speed = speed});
      }
    }
  }

  tracker_ = new CPUSpeedTracker(cpu_speeds);
}

bool RequestAffinity(CpuAffinity affinity) {
  // Populate CPU Info if undefined.
  auto count = std::thread::hardware_concurrency();
  std::call_once(cpu_tracker_flag_, [count]() {
    InitCPUInfo(count);
  });
  if (tracker_ == nullptr) {
    return true;
  }

  // Determine physical core count and speed.

  // If we either cannot determine slow versus fast cores, or if there is no
  // distinction in core speed, return without setting affinity.
  if (!tracker_->IsValid()) {
    return true;
  }

  cpu_set_t set;
  CPU_ZERO(&set);
  for (const auto index : tracker_->GetIndexes(affinity)) {
    CPU_SET(index, &set);
  }
  return sched_setaffinity(gettid(), sizeof(set), &set) == 0;
}

/// Inheriting ThreadConfigurer and use Android platform thread API to configure
/// the thread priorities
static void AndroidPlatformThreadConfigSetter(
    const fml::Thread::ThreadConfig& config) {
  // set thread name
  fml::Thread::SetCurrentThreadName(config);
  // set thread priority
  switch (config.priority) {
    case fml::Thread::ThreadPriority::BACKGROUND: {
      RequestAffinity(CpuAffinity::kEfficiency);
      if (::setpriority(PRIO_PROCESS, 0, 10) != 0) {
        FML_LOG(ERROR) << "Failed to set IO task runner priority";
      }
      break;
    }
    case fml::Thread::ThreadPriority::DISPLAY: {
      RequestAffinity(CpuAffinity::kPerformance);
      if (::setpriority(PRIO_PROCESS, 0, -1) != 0) {
        FML_LOG(ERROR) << "Failed to set UI task runner priority";
      }
      break;
    }
    case fml::Thread::ThreadPriority::RASTER: {
      RequestAffinity(CpuAffinity::kPerformance);
      // Android describes -8 as "most important display threads, for
      // compositing the screen and retrieving input events". Conservatively
      // set the raster thread to slightly lower priority than it.
      if (::setpriority(PRIO_PROCESS, 0, -5) != 0) {
        // Defensive fallback. Depending on the OEM, it may not be possible
        // to set priority to -5.
        if (::setpriority(PRIO_PROCESS, 0, -2) != 0) {
          FML_LOG(ERROR) << "Failed to set raster task runner priority";
        }
      }
      break;
    }
    default:
      RequestAffinity(CpuAffinity::kNotPerformance);
      if (::setpriority(PRIO_PROCESS, 0, 0) != 0) {
        FML_LOG(ERROR) << "Failed to set priority";
      }
  }
}
static PlatformData GetDefaultPlatformData() {
  PlatformData platform_data;
  platform_data.lifecycle_state = "AppLifecycleState.detached";
  return platform_data;
}

AndroidShellHolder::AndroidShellHolder(
    const flutter::Settings& settings,
    std::shared_ptr<PlatformViewAndroidJNI> jni_facade)
    : settings_(settings), jni_facade_(jni_facade) {
  static size_t thread_host_count = 1;
  auto thread_label = std::to_string(thread_host_count++);

  auto mask =
      ThreadHost::Type::UI | ThreadHost::Type::RASTER | ThreadHost::Type::IO;

  flutter::ThreadHost::ThreadHostConfig host_config(
      thread_label, mask, AndroidPlatformThreadConfigSetter);
  host_config.ui_config = fml::Thread::ThreadConfig(
      flutter::ThreadHost::ThreadHostConfig::MakeThreadName(
          flutter::ThreadHost::Type::UI, thread_label),
      fml::Thread::ThreadPriority::DISPLAY);
  host_config.raster_config = fml::Thread::ThreadConfig(
      flutter::ThreadHost::ThreadHostConfig::MakeThreadName(
          flutter::ThreadHost::Type::RASTER, thread_label),
      fml::Thread::ThreadPriority::RASTER);
  host_config.io_config = fml::Thread::ThreadConfig(
      flutter::ThreadHost::ThreadHostConfig::MakeThreadName(
          flutter::ThreadHost::Type::IO, thread_label),
      fml::Thread::ThreadPriority::NORMAL);

  thread_host_ = std::make_shared<ThreadHost>(host_config);

  fml::WeakPtr<PlatformViewAndroid> weak_platform_view;
  Shell::CreateCallback<PlatformView> on_create_platform_view =
      [&jni_facade, &weak_platform_view](Shell& shell) {
        std::unique_ptr<PlatformViewAndroid> platform_view_android;
        platform_view_android = std::make_unique<PlatformViewAndroid>(
            shell,                   // delegate
            shell.GetTaskRunners(),  // task runners
            jni_facade,              // JNI interop
            shell.GetSettings()
                .enable_software_rendering,   // use software rendering
            shell.GetSettings().msaa_samples  // msaa sample count
        );
        weak_platform_view = platform_view_android->GetWeakPtr();
        return platform_view_android;
      };

  Shell::CreateCallback<Rasterizer> on_create_rasterizer = [](Shell& shell) {
    return std::make_unique<Rasterizer>(shell);
  };

  // The current thread will be used as the platform thread. Ensure that the
  // message loop is initialized.
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  fml::RefPtr<fml::TaskRunner> raster_runner;
  fml::RefPtr<fml::TaskRunner> ui_runner;
  fml::RefPtr<fml::TaskRunner> io_runner;
  fml::RefPtr<fml::TaskRunner> platform_runner =
      fml::MessageLoop::GetCurrent().GetTaskRunner();
  raster_runner = thread_host_->raster_thread->GetTaskRunner();
  ui_runner = thread_host_->ui_thread->GetTaskRunner();
  io_runner = thread_host_->io_thread->GetTaskRunner();

  flutter::TaskRunners task_runners(thread_label,     // label
                                    platform_runner,  // platform
                                    raster_runner,    // raster
                                    ui_runner,        // ui
                                    io_runner         // io
  );

  shell_ =
      Shell::Create(GetDefaultPlatformData(),  // window data
                    task_runners,              // task runners
                    settings_,                 // settings
                    on_create_platform_view,   // platform view create callback
                    on_create_rasterizer       // rasterizer create callback
      );

  if (shell_) {
    shell_->GetDartVM()->GetConcurrentMessageLoop()->PostTaskToAllWorkers([]() {
      if (::setpriority(PRIO_PROCESS, gettid(), 1) != 0) {
        FML_LOG(ERROR) << "Failed to set Workers task runner priority";
      }
    });

    shell_->RegisterImageDecoder(
        [runner = task_runners.GetIOTaskRunner()](sk_sp<SkData> buffer) {
          return AndroidImageGenerator::MakeFromData(std::move(buffer), runner);
        },
        -1);
    FML_DLOG(INFO) << "Registered Android SDK image decoder (API level 28+)";
  }

  platform_view_ = weak_platform_view;
  FML_DCHECK(platform_view_);
  is_valid_ = shell_ != nullptr;
}

AndroidShellHolder::AndroidShellHolder(
    const Settings& settings,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade,
    const std::shared_ptr<ThreadHost>& thread_host,
    std::unique_ptr<Shell> shell,
    std::unique_ptr<APKAssetProvider> apk_asset_provider,
    const fml::WeakPtr<PlatformViewAndroid>& platform_view)
    : settings_(settings),
      jni_facade_(jni_facade),
      platform_view_(platform_view),
      thread_host_(thread_host),
      shell_(std::move(shell)),
      apk_asset_provider_(std::move(apk_asset_provider)) {
  FML_DCHECK(jni_facade);
  FML_DCHECK(shell_);
  FML_DCHECK(shell_->IsSetup());
  FML_DCHECK(platform_view_);
  FML_DCHECK(thread_host_);
  is_valid_ = shell_ != nullptr;
}

AndroidShellHolder::~AndroidShellHolder() {
  shell_.reset();
  thread_host_.reset();
}

bool AndroidShellHolder::IsValid() const {
  return is_valid_;
}

const flutter::Settings& AndroidShellHolder::GetSettings() const {
  return settings_;
}

std::unique_ptr<AndroidShellHolder> AndroidShellHolder::Spawn(
    std::shared_ptr<PlatformViewAndroidJNI> jni_facade,
    const std::string& entrypoint,
    const std::string& libraryUrl,
    const std::string& initial_route,
    const std::vector<std::string>& entrypoint_args) const {
  FML_DCHECK(shell_ && shell_->IsSetup())
      << "A new Shell can only be spawned "
         "if the current Shell is properly constructed";

  // Pull out the new PlatformViewAndroid from the new Shell to feed to it to
  // the new AndroidShellHolder.
  //
  // It's a weak pointer because it's owned by the Shell (which we're also)
  // making below. And the AndroidShellHolder then owns the Shell.
  fml::WeakPtr<PlatformViewAndroid> weak_platform_view;

  // Take out the old AndroidContext to reuse inside the PlatformViewAndroid
  // of the new Shell.
  PlatformViewAndroid* android_platform_view = platform_view_.get();
  // There's some indirection with platform_view_ being a weak pointer but
  // we just checked that the shell_ exists above and a valid shell is the
  // owner of the platform view so this weak pointer always exists.
  FML_DCHECK(android_platform_view);
  std::shared_ptr<flutter::AndroidContext> android_context =
      android_platform_view->GetAndroidContext();
  FML_DCHECK(android_context);

  // This is a synchronous call, so the captures don't have race checks.
  Shell::CreateCallback<PlatformView> on_create_platform_view =
      [&jni_facade, android_context, &weak_platform_view](Shell& shell) {
        std::unique_ptr<PlatformViewAndroid> platform_view_android;
        platform_view_android = std::make_unique<PlatformViewAndroid>(
            shell,                   // delegate
            shell.GetTaskRunners(),  // task runners
            jni_facade,              // JNI interop
            android_context          // Android context
        );
        weak_platform_view = platform_view_android->GetWeakPtr();
        return platform_view_android;
      };

  Shell::CreateCallback<Rasterizer> on_create_rasterizer = [](Shell& shell) {
    return std::make_unique<Rasterizer>(shell);
  };

  // TODO(xster): could be worth tracing this to investigate whether
  // the IsolateConfiguration could be cached somewhere.
  auto config = BuildRunConfiguration(entrypoint, libraryUrl, entrypoint_args);
  if (!config) {
    // If the RunConfiguration was null, the kernel blob wasn't readable.
    // Fail the whole thing.
    return nullptr;
  }

  std::unique_ptr<flutter::Shell> shell =
      shell_->Spawn(std::move(config.value()), initial_route,
                    on_create_platform_view, on_create_rasterizer);

  return std::unique_ptr<AndroidShellHolder>(new AndroidShellHolder(
      GetSettings(), jni_facade, thread_host_, std::move(shell),
      apk_asset_provider_->Clone(), weak_platform_view));
}

void AndroidShellHolder::Launch(
    std::unique_ptr<APKAssetProvider> apk_asset_provider,
    const std::string& entrypoint,
    const std::string& libraryUrl,
    const std::vector<std::string>& entrypoint_args) {
  if (!IsValid()) {
    return;
  }

  apk_asset_provider_ = std::move(apk_asset_provider);
  auto config = BuildRunConfiguration(entrypoint, libraryUrl, entrypoint_args);
  if (!config) {
    return;
  }
  UpdateDisplayMetrics();
  shell_->RunEngine(std::move(config.value()));
}

Rasterizer::Screenshot AndroidShellHolder::Screenshot(
    Rasterizer::ScreenshotType type,
    bool base64_encode) {
  if (!IsValid()) {
    return {nullptr, SkISize::MakeEmpty(), ""};
  }
  return shell_->Screenshot(type, base64_encode);
}

fml::WeakPtr<PlatformViewAndroid> AndroidShellHolder::GetPlatformView() {
  FML_DCHECK(platform_view_);
  return platform_view_;
}

void AndroidShellHolder::NotifyLowMemoryWarning() {
  FML_DCHECK(shell_);
  shell_->NotifyLowMemoryWarning();
}

std::optional<RunConfiguration> AndroidShellHolder::BuildRunConfiguration(
    const std::string& entrypoint,
    const std::string& libraryUrl,
    const std::vector<std::string>& entrypoint_args) const {
  std::unique_ptr<IsolateConfiguration> isolate_configuration;
  if (flutter::DartVM::IsRunningPrecompiledCode()) {
    isolate_configuration = IsolateConfiguration::CreateForAppSnapshot();
  } else {
    std::unique_ptr<fml::Mapping> kernel_blob =
        fml::FileMapping::CreateReadOnly(
            GetSettings().application_kernel_asset);
    if (!kernel_blob) {
      FML_DLOG(ERROR) << "Unable to load the kernel blob asset.";
      return std::nullopt;
    }
    isolate_configuration =
        IsolateConfiguration::CreateForKernel(std::move(kernel_blob));
  }

  RunConfiguration config(std::move(isolate_configuration));
  config.AddAssetResolver(apk_asset_provider_->Clone());

  {
    if (!entrypoint.empty() && !libraryUrl.empty()) {
      config.SetEntrypointAndLibrary(entrypoint, libraryUrl);
    } else if (!entrypoint.empty()) {
      config.SetEntrypoint(entrypoint);
    }
    if (!entrypoint_args.empty()) {
      config.SetEntrypointArgs(entrypoint_args);
    }
  }
  return config;
}

void AndroidShellHolder::UpdateDisplayMetrics() {
  std::vector<std::unique_ptr<Display>> displays;
  displays.push_back(std::make_unique<AndroidDisplay>(jni_facade_));
  shell_->OnDisplayUpdates(std::move(displays));
}

void AndroidShellHolder::SetIsRenderingToImageView(bool value) {
  platform_view_->SetIsRenderingToImageView(value);
}

}  // namespace flutter
