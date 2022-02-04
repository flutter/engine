// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_THREAD_H_
#define FLUTTER_FML_THREAD_H_

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "flutter/fml/macros.h"
#include "flutter/fml/task_runner.h"

namespace fml {

class Thread {
 public:
  /// Valid values for priority of Thread.
  enum class ThreadPriority : int {
    /// Suitable for threads that shouldn't disrupt high priority work.
    BACKGROUND,
    /// Default priority level.
    NORMAL,
    /// Suitable for threads which generate data for the display.
    DISPLAY,
    /// Suitable for thread which raster data.
    RASTER,
  };

  /// The ThreadConfig is used for setting thread perorities.
  class ThreadConfig {
   public:
    explicit ThreadConfig(const std::string& name = "",
                          ThreadPriority priority = ThreadPriority::NORMAL);

    static std::unique_ptr<ThreadConfig> MakeDefaultConfigure(
        const std::string& name = "") {
      return std::make_unique<ThreadConfig>(name);
    }

    ThreadPriority GetThreadPriority() const { return thread_priority_; }

    const std::string& GetThreadName() const { return thread_name_; }

    /// Set current thread name.
    virtual void SetCurrentThreadName() const;

    /// default do nothing, which mean user can use platform api to set priority
    /// example: iOS might use pthread_qos set thread priority, Android might
    /// use ::setPriority set thread priority
    virtual void SetCurrentThreadPriority() const;

    virtual ~ThreadConfig() = default;

   private:
    const std::string thread_name_;
    ThreadPriority thread_priority_;
  };

  explicit Thread(const std::string& name);

  explicit Thread(std::unique_ptr<ThreadConfig> config =
                      ThreadConfig::MakeDefaultConfigure());

  ~Thread();

  fml::RefPtr<fml::TaskRunner> GetTaskRunner() const;

  void Join();

  static void SetCurrentThreadName(const std::string& name);

 private:
  std::unique_ptr<std::thread> thread_;
  fml::RefPtr<fml::TaskRunner> task_runner_;
  std::atomic_bool joined_;

  FML_DISALLOW_COPY_AND_ASSIGN(Thread);
};

}  // namespace fml

#endif  // FLUTTER_FML_THREAD_H_
