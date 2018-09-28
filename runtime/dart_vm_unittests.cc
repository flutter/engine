// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/testing.h"
#include "flutter/testing/thread_test.h"
#include "gtest/gtest.h"

namespace blink {

static Settings GetTestSettings() {
  Settings settings;
  settings.task_observer_add = [](intptr_t, fml::closure) {};
  settings.task_observer_remove = [](intptr_t) {};
  return settings;
}

TEST(DartVM, SimpleInitialization) {
  auto vm = DartVM::ForProcess(GetTestSettings());
  ASSERT_TRUE(vm);
  ASSERT_EQ(vm, DartVM::ForProcess(GetTestSettings()));
  ASSERT_FALSE(DartVM::IsRunningPrecompiledCode());
}

TEST(DartVM, SimpleIsolateNameServer) {
  auto vm = DartVM::ForProcess(GetTestSettings());
  auto ns = vm->GetIsolateNameServer();
  ASSERT_EQ(ns->LookupIsolatePortByName("foobar"), ILLEGAL_PORT);
  ASSERT_FALSE(ns->RemoveIsolateNameMapping("foobar"));
  ASSERT_TRUE(ns->RegisterIsolatePortWithName(123, "foobar"));
  ASSERT_FALSE(ns->RegisterIsolatePortWithName(123, "foobar"));
  ASSERT_EQ(ns->LookupIsolatePortByName("foobar"), 123);
  ASSERT_TRUE(ns->RemoveIsolateNameMapping("foobar"));
}

TEST(DartVM, CanReinitializeVMOverAndOver) {
  for (size_t i = 0; i < 1000; ++i) {
    FML_LOG(INFO) << "Run " << i + 1;
    // VM should not already be running.
    ASSERT_FALSE(DartVM::ForProcessIfInitialized());
    auto vm = DartVM::ForProcess(GetTestSettings());
    ASSERT_TRUE(vm);
    ASSERT_TRUE(DartVM::ForProcessIfInitialized());
  }
}

using DartVMThreadTest = ::testing::ThreadTest;

TEST_F(DartVMThreadTest, CanRunIsolatesInANewVM) {
  for (size_t i = 0; i < 1000; ++i) {
    FML_LOG(INFO) << "Run " << i + 1;
    // VM should not already be running.
    ASSERT_FALSE(DartVM::ForProcessIfInitialized());
    auto vm = DartVM::ForProcess(GetTestSettings());
    ASSERT_TRUE(vm);
    ASSERT_TRUE(DartVM::ForProcessIfInitialized());

    Settings settings = {};

    settings.task_observer_add = [](intptr_t, fml::closure) {};
    settings.task_observer_remove = [](intptr_t) {};

    auto labels = testing::GetCurrentTestName() + std::to_string(i);
    shell::ThreadHost host(labels, shell::ThreadHost::Type::UI |
                                       shell::ThreadHost::Type::GPU |
                                       shell::ThreadHost::Type::IO);

    TaskRunners task_runners(
        labels,                            // task runner labels
        GetCurrentTaskRunner(),            // platform task runner
        host.gpu_thread->GetTaskRunner(),  // GPU task runner
        host.ui_thread->GetTaskRunner(),   // UI task runner
        host.io_thread->GetTaskRunner()    // IO task runner
    );

    auto weak_isolate = DartIsolate::CreateRootIsolate(
        vm.get(),                  // vm
        vm->GetIsolateSnapshot(),  // isolate snapshot
        vm->GetSharedSnapshot(),   // shared snapshot
        std::move(task_runners),   // task runners
        nullptr,                   // window
        {},                        // snapshot delegate
        {},                        // resource context
        nullptr,                   // unref qeueue
        "main.dart",               // advisory uri
        "main"                     // advisory entrypoint
    );

    auto root_isolate = weak_isolate.lock();
    ASSERT_TRUE(root_isolate);
    ASSERT_EQ(root_isolate->GetPhase(), DartIsolate::Phase::LibrariesSetup);
    ASSERT_TRUE(root_isolate->Shutdown());
  }
}

}  // namespace blink
