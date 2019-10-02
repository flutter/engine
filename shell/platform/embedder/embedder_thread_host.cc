// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is why we can't yet export the UI thread to embedders.
#define FML_USED_ON_EMBEDDER

#include "flutter/shell/platform/embedder/embedder_thread_host.h"

#include <optional>

#include "flutter/fml/message_loop.h"
#include "flutter/shell/platform/embedder/embedder_safe_access.h"

namespace flutter {

//------------------------------------------------------------------------------
/// @brief      Attempts to create a task runner from an embedder task runner
///             description. The null task runner is a valid return value. When
///             the optional contains a null RefPtr, it means the embedder did
///             not specify a task runner (a fallback may need to be used in
///             such cases). On the other hand, if a nullopt is returned, it
///             means that the embedder attempted to give the Engine a task
///             runner but messed up one of the required fields. In this case,
///             the engine must fail early and not attempt to pick a fallback.
///
/// @param[in]  description  The description
///
/// @return     An optional reference to a potentially null task runner.
///
static std::optional<fml::RefPtr<EmbedderTaskRunner>> CreateEmbedderTaskRunner(
    const FlutterTaskRunnerDescription* description) {
  if (description == nullptr) {
    return {nullptr};
  }

  if (SAFE_ACCESS(description, runs_task_on_current_thread_callback, nullptr) ==
      nullptr) {
    FML_LOG(ERROR) << "FlutterTaskRunnerDescription.runs_task_on_current_"
                      "thread_callback was nullptr.";
    return std::nullopt;
  }

  if (SAFE_ACCESS(description, post_task_callback, nullptr) == nullptr) {
    FML_LOG(ERROR)
        << "FlutterTaskRunnerDescription.post_task_callback was nullptr.";
    return std::nullopt;
  }

  auto user_data = SAFE_ACCESS(description, user_data, nullptr);

  // ABI safety checks have been completed.
  auto post_task_callback_c = description->post_task_callback;
  auto runs_task_on_current_thread_callback_c =
      description->runs_task_on_current_thread_callback;

  EmbedderTaskRunner::DispatchTable task_runner_dispatch_table = {
      // .post_task_callback
      [post_task_callback_c, user_data](EmbedderTaskRunner* task_runner,
                                        uint64_t task_baton,
                                        fml::TimePoint target_time) -> void {
        FlutterTask task = {
            // runner
            reinterpret_cast<FlutterTaskRunner>(task_runner),
            // task
            task_baton,
        };
        post_task_callback_c(task, target_time.ToEpochDelta().ToNanoseconds(),
                             user_data);
      },
      // runs_task_on_current_thread_callback
      [runs_task_on_current_thread_callback_c, user_data]() -> bool {
        return runs_task_on_current_thread_callback_c(user_data);
      }};

  return fml::MakeRefCounted<EmbedderTaskRunner>(task_runner_dispatch_table);
}

std::unique_ptr<EmbedderThreadHost>
EmbedderThreadHost::CreateEmbedderOrEngineManagedThreadHost(
    const FlutterCustomTaskRunners* custom_task_runners) {
  {
    auto host = CreateEmbedderManagedThreadHost(custom_task_runners);
    if (host && host->IsValid()) {
      return host;
    }
  }

  // Only attempt to create the engine managed host if the embedder did not
  // specify a custom configuration. We don't want to fallback to the engine
  // managed configuration if the embedder attempted to specify a configuration
  // but messed up with an incorrect configuration.
  if (custom_task_runners == nullptr) {
    auto host = CreateEngineManagedThreadHost();
    if (host && host->IsValid()) {
      return host;
    }
  }

  return nullptr;
}

static fml::RefPtr<fml::TaskRunner> GetCurrentThreadTaskRunner() {
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  return fml::MessageLoop::GetCurrent().GetTaskRunner();
}

constexpr const char* kFlutterThreadName = "io.flutter";

// static
std::unique_ptr<EmbedderThreadHost>
EmbedderThreadHost::CreateEmbedderManagedThreadHost(
    const FlutterCustomTaskRunners* custom_task_runners) {
  if (custom_task_runners == nullptr) {
    return nullptr;
  }

  // The UI and IO threads are always created by the engine and the embedder has
  // no opportunity to specify task runners for the same.
  // TODO(chinmaygarde): If we ever decide to expose more task runners to engine
  // managed threads, this mask will need to be updated.
  uint64_t engine_thread_host_mask =
      ThreadHost::Type::UI | ThreadHost::Type::IO;

  const auto optional_platform_task_runner = CreateEmbedderTaskRunner(
      SAFE_ACCESS(custom_task_runners, platform_task_runner, nullptr));
  const auto optional_render_task_runner = CreateEmbedderTaskRunner(
      SAFE_ACCESS(custom_task_runners, render_task_runner, nullptr));

  if (!optional_platform_task_runner.has_value() ||
      !optional_render_task_runner.has_value()) {
    // User error while supplying a custom task runner. Return an invalid thread
    // host. This will abort engine initialization. We don't want to fallback to
    // defaults if the user wanted to specify a task runner but just messed up
    // instead.
    return nullptr;
  }

  // If the embedder has not supplied a GPU task runner, we need to create one.
  if (!optional_render_task_runner.value()) {
    engine_thread_host_mask |= ThreadHost::Type::GPU;
  }

  // Create a thread host with just the threads we have to manage. The embedder
  // has provided the rest.
  ThreadHost thread_host(kFlutterThreadName, engine_thread_host_mask);

  // If the embedder has supplied a platform task runner, use that. If not, use
  // the current thread task runner.
  auto platform_task_runner = optional_platform_task_runner.value()
                                  ? static_cast<fml::RefPtr<fml::TaskRunner>>(
                                        optional_platform_task_runner.value())
                                  : GetCurrentThreadTaskRunner();

  // If the embedder has supplied a GPU task runner, use that. If not, use the
  // one from our thread host.
  auto gpu_task_runner = optional_render_task_runner.value()
                             ? static_cast<fml::RefPtr<fml::TaskRunner>>(
                                   optional_render_task_runner.value())
                             : thread_host.gpu_thread->GetTaskRunner();

  flutter::TaskRunners task_runners(
      kFlutterThreadName,
      platform_task_runner,                    // platform
      gpu_task_runner,                         // gpu
      thread_host.ui_thread->GetTaskRunner(),  // ui
      thread_host.io_thread->GetTaskRunner()   // io
  );

  if (!task_runners.IsValid()) {
    return nullptr;
  }

  std::set<fml::RefPtr<EmbedderTaskRunner>> embedder_task_runners;

  if (optional_platform_task_runner.value()) {
    embedder_task_runners.insert(optional_platform_task_runner.value());
  }

  if (optional_render_task_runner.value()) {
    embedder_task_runners.insert(optional_render_task_runner.value());
  }

  auto embedder_host = std::make_unique<EmbedderThreadHost>(
      std::move(thread_host), std::move(task_runners),
      std::move(embedder_task_runners));

  if (embedder_host->IsValid()) {
    return embedder_host;
  }

  return nullptr;
}

// static
std::unique_ptr<EmbedderThreadHost>
EmbedderThreadHost::CreateEngineManagedThreadHost() {
  // Create a thread host with the current thread as the platform thread and all
  // other threads managed.
  ThreadHost thread_host(kFlutterThreadName, ThreadHost::Type::GPU |
                                                 ThreadHost::Type::IO |
                                                 ThreadHost::Type::UI);

  // For embedder platforms that don't have native message loop interop, this
  // will reference a task runner that points to a null message loop
  // implementation.
  auto platform_task_runner = GetCurrentThreadTaskRunner();

  flutter::TaskRunners task_runners(
      kFlutterThreadName,
      platform_task_runner,                     // platform
      thread_host.gpu_thread->GetTaskRunner(),  // gpu
      thread_host.ui_thread->GetTaskRunner(),   // ui
      thread_host.io_thread->GetTaskRunner()    // io
  );

  if (!task_runners.IsValid()) {
    return nullptr;
  }

  std::set<fml::RefPtr<EmbedderTaskRunner>> empty_embedder_task_runners;

  auto embedder_host = std::make_unique<EmbedderThreadHost>(
      std::move(thread_host), std::move(task_runners),
      empty_embedder_task_runners);

  if (embedder_host->IsValid()) {
    return embedder_host;
  }

  return nullptr;
}

EmbedderThreadHost::EmbedderThreadHost(
    ThreadHost host,
    flutter::TaskRunners runners,
    std::set<fml::RefPtr<EmbedderTaskRunner>> embedder_task_runners)
    : host_(std::move(host)), runners_(std::move(runners)) {
  for (const auto& runner : embedder_task_runners) {
    runners_map_[reinterpret_cast<int64_t>(runner.get())] = runner;
  }
}

EmbedderThreadHost::~EmbedderThreadHost() = default;

bool EmbedderThreadHost::IsValid() const {
  return runners_.IsValid();
}

const flutter::TaskRunners& EmbedderThreadHost::GetTaskRunners() const {
  return runners_;
}

bool EmbedderThreadHost::PostTask(int64_t runner, uint64_t task) const {
  auto found = runners_map_.find(runner);
  if (found == runners_map_.end()) {
    return false;
  }
  return found->second->PostTask(task);
}

}  // namespace flutter
