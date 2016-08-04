// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/testing/test_runner.h"

#include <iostream>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_util.h"
#include "sky/shell/platform_view.h"
#include "sky/shell/shell.h"
#include "sky/shell/testing/platform_view_test.h"

namespace sky {
namespace shell {

TestRunner::TestRunner()
    : platform_view_(new PlatformViewTest()), weak_ptr_factory_(this) {
  ViewportMetricsPtr metrics = ViewportMetrics::New();

  metrics->physical_width = 800;
  metrics->physical_height = 600;

  platform_view_->GetEnginePtr()->OnViewportMetricsChanged(metrics.Pass());
}

TestRunner::~TestRunner() = default;

TestRunner& TestRunner::Shared() {
  static TestRunner* g_test_runner = nullptr;
  if (!g_test_runner)
    g_test_runner = new TestRunner();
  return *g_test_runner;
}

void TestRunner::Run(const TestDescriptor& test) {
  platform_view_->GetEnginePtr()->RunFromFile(test.path, test.packages, "");
}

}  // namespace shell
}  // namespace sky
