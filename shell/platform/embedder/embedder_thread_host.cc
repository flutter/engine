// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/shell/platform/embedder/embedder_thread_host.h"

#include <algorithm>

#include "flutter/fml/message_loop.h"
#include "flutter/shell/platform/embedder/embedder_safe_access.h"

namespace flutter {

//------------------------------------------------------------------------------
/// @brief      Attempts to create a task runner from an embedder task runner
///             description. The first boolean in the pair indicate whether the
///             embedder specified an invalid task runner description. In this
///             case, engine launch must be aborted. If the embedder did not
///             specify any task runner, an engine managed task runner and
///             thread must be selected instead.
///
/// @param[in]  description  The description
///
/// @return     A pair that returns if the embedder has specified a task runner
///             (null otherwise) and whether to terminate further engine launch.
///
static std::pair<bool, fml::RefPtr<EmbedderTaskRunner>>
CreateEmbedderTaskRunner(const FlutterTaskRunnerDescription* description) {
  if (description == nullptr) {
    // This is not embedder error. The embedder API will just have to create a
    // plain old task runner (and create a thread for it) instead of using a
    // task runner provided to us by the embedder.
    return {true, {}};
  }

  if (SAFE_ACCESS(description, runs_task_on_current_thread_callback, nullptr) ==
      nullptr) {
    FML_LOG(ERROR) << "FlutterTaskRunnerDescription.runs_task_on_current_"
                      "thread_callback was nullptr.";
    return {false, {}};
  }

  if (SAFE_ACCESS(description, post_task_callback, nullptr) == nullptr) {
    FML_LOG(ERROR)
        << "FlutterTaskRunnerDescription.post_task_callback was nullptr.";
    return {false, {}};
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

  return {true, fml::MakeRefCounted<EmbedderTaskRunner>(
                    task_runner_dispatch_table,
                    SAFE_ACCESS(description, identifier, 0u))};
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
  // specify a custom configuration. Don't fallback to the engine managed
  // configuration if the embedder attempted to specify a configuration but
  // messed up with an incorrect configuration.
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

  // The engine will create any threads left unspecified by the embedder.
  std::set<fml::RefPtr<EmbedderTaskRunner>> embedder_task_runners;
  uint64_t engine_thread_host_mask = 0;

  auto [platform_valid, platform_task_runner] = CreateEmbedderTaskRunner(
      SAFE_ACCESS(custom_task_runners, platform_task_runner, nullptr));
  auto [render_valid, render_task_runner] = CreateEmbedderTaskRunner(
      SAFE_ACCESS(custom_task_runners, render_task_runner, nullptr));
  auto [ui_valid, ui_task_runner] = CreateEmbedderTaskRunner(
      SAFE_ACCESS(custom_task_runners, ui_task_runner, nullptr));
  auto [io_valid, io_task_runner] = CreateEmbedderTaskRunner(
      SAFE_ACCESS(custom_task_runners, io_task_runner, nullptr));

  if (!platform_valid || !render_valid || !ui_valid || !io_valid) {
    // One or more custom task runner specifications contained a user error.
    // Return an invalid thread host, which will abort engine initialization.
    // Don't fallback to defaults if the user wanted to specify a task runner
    // but just messed up instead.
    return nullptr;
  }

  // TODO(dworsham): Ask chinmay why this matters.  iOS?
  // If both the platform task runner and the GPU task runner are specified and
  // have the same identifier, store only one.
  // if (platform_task_runner && render_task_runner) {
  //   if (platform_task_runner->GetEmbedderIdentifier() ==
  //       render_task_runner->GetEmbedderIdentifier()) {
  //     render_task_runner = platform_task_runner;
  //   }
  // }

  // If the embedder has not supplied any task runners, they need to be created.
  // The current thread will be used in the case of the platform task runner.
  if (platform_task_runner) {
    embedder_task_runners.insert(platform_task_runner);
  }
  if (!ui_task_runner) {
    engine_thread_host_mask |= ThreadHost::Type::UI;
  } else {
    embedder_task_runners.insert(ui_task_runner);
  }
  if (!io_task_runner) {
    engine_thread_host_mask |= ThreadHost::Type::IO;
  } else {
    embedder_task_runners.insert(io_task_runner);
  }
  if (!render_task_runner) {
    engine_thread_host_mask |= ThreadHost::Type::Raster;
  } else {
    embedder_task_runners.insert(render_task_runner);
  }

  // Create a thread host with just the threads that need to be managed by the
  // engine. The embedder has provided the rest.
  ThreadHost thread_host(kFlutterThreadName, engine_thread_host_mask);
  flutter::TaskRunners task_runners(
      kFlutterThreadName,
      platform_task_runner
          ? static_cast<fml::RefPtr<fml::TaskRunner>>(platform_task_runner)
          : GetCurrentThreadTaskRunner(),
      render_task_runner
          ? static_cast<fml::RefPtr<fml::TaskRunner>>(render_task_runner)
          : thread_host.raster_thread->GetTaskRunner(),
      ui_task_runner ? static_cast<fml::RefPtr<fml::TaskRunner>>(ui_task_runner)
                     : thread_host.ui_thread->GetTaskRunner(),
      io_task_runner ? static_cast<fml::RefPtr<fml::TaskRunner>>(io_task_runner)
                     : thread_host.io_thread->GetTaskRunner());

  if (!task_runners.IsValid()) {
    return nullptr;
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
  ThreadHost thread_host(kFlutterThreadName, ThreadHost::Type::Raster |
                                                 ThreadHost::Type::IO |
                                                 ThreadHost::Type::UI);

  // For embedder platforms that don't have native message loop interop, this
  // will reference a task runner that points to a null message loop
  // implementation.
  auto platform_task_runner = GetCurrentThreadTaskRunner();

  flutter::TaskRunners task_runners(
      kFlutterThreadName,
      platform_task_runner,                        // platform
      thread_host.raster_thread->GetTaskRunner(),  // raster
      thread_host.ui_thread->GetTaskRunner(),      // ui
      thread_host.io_thread->GetTaskRunner()       // io
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
