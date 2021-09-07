// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/shell.h"

#include "flutter/benchmarking/benchmarking.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/dart_isolate_runner.h"
#include "flutter/testing/fixtures_base.h"
#include "flutter/testing/testing.h"
#include "runtime/dart_vm_lifecycle.h"

namespace flutter::testing {

class DartNativeBenchmarks : public FixturesBase, public benchmark::Fixture {
 public:
  DartNativeBenchmarks() : FixturesBase() {}

  void SetUp(const ::benchmark::State& state) {}

  void TearDown(const ::benchmark::State& state) {}

  void Wait() { latch_.Wait(); }

  void Signal() { latch_.Signal(); }

 private:
  fml::AutoResetWaitableEvent latch_;

  FML_DISALLOW_COPY_AND_ASSIGN(DartNativeBenchmarks);
};

BENCHMARK_F(DartNativeBenchmarks, DartNativeMessages)(benchmark::State& st) {
  while (st.KeepRunning()) {
    st.PauseTiming();
    ASSERT_FALSE(DartVMRef::IsInstanceRunning());
    AddNativeCallback(
        "NotifyNative",
        CREATE_NATIVE_ENTRY(([this](Dart_NativeArguments args) { Signal(); })));

    const auto settings = CreateSettingsForFixture();
    DartVMRef vm_ref = DartVMRef::Create(settings);

    ThreadHost thread_host("io.flutter.test.DartNativeBenchmarks.",
                           ThreadHost::Type::Platform | ThreadHost::Type::IO |
                               ThreadHost::Type::UI);
    TaskRunners task_runners(
        "test",
        thread_host.platform_thread->GetTaskRunner(),  // platform
        thread_host.platform_thread->GetTaskRunner(),  // raster
        thread_host.ui_thread->GetTaskRunner(),        // ui
        thread_host.io_thread->GetTaskRunner()         // io
    );

    {
      st.ResumeTiming();
      auto isolate =
          RunDartCodeInIsolate(vm_ref, settings, task_runners, "notifyNative",
                               {}, GetDefaultKernelFilePath());
      ASSERT_TRUE(isolate);
      ASSERT_EQ(isolate->get()->GetPhase(), DartIsolate::Phase::Running);
      Wait();
    }
  }
}

}  // namespace flutter::testing
