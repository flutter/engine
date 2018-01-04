// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_COMMON_TASK_RUNNERS_H_
#define FLUTTER_COMMON_TASK_RUNNERS_H_

#include <string>

#include "flutter/fml/task_runner.h"
#include "lib/fxl/macros.h"

namespace blink {

class TaskRunners {
 public:
  TaskRunners(std::string label,
              fxl::RefPtr<fml::TaskRunner> platform,
              fxl::RefPtr<fml::TaskRunner> gpu,
              fxl::RefPtr<fml::TaskRunner> ui,
              fxl::RefPtr<fml::TaskRunner> io);

  ~TaskRunners();

  const std::string& GetLabel() const;

  fxl::RefPtr<fml::TaskRunner> GetPlatformTaskRunner() const;

  fxl::RefPtr<fml::TaskRunner> GetUITaskRunner() const;

  fxl::RefPtr<fml::TaskRunner> GetIOTaskRunner() const;

  fxl::RefPtr<fml::TaskRunner> GetGPUTaskRunner() const;

  bool IsValid() const;

 private:
  const std::string label_;
  fxl::RefPtr<fml::TaskRunner> platform_;
  fxl::RefPtr<fml::TaskRunner> gpu_;
  fxl::RefPtr<fml::TaskRunner> ui_;
  fxl::RefPtr<fml::TaskRunner> io_;
};
}  // namespace blink

#endif  // FLUTTER_COMMON_TASK_RUNNERS_H_
