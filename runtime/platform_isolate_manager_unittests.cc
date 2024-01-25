// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/platform_isolate_manager.h"

#include "flutter/fml/thread_local.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/runtime/dart_vm_lifecycle.h"
#include "flutter/testing/fixture_test.h"
#include "flutter/testing/testing.h"

namespace flutter {
namespace testing {

struct IsolateData {
  PlatformIsolateManager* mgr;
  Dart_Isolate isolate = nullptr;
  bool is_shutdown = false;
  bool is_registered = false;
  IsolateData(PlatformIsolateManager* _mgr) : mgr(_mgr) {}
};

// The IsolateDataMap is a map from Dart_Isolate to a *vector* of IsolateData,
// because Dart_Isolates are frequently reused after shutdown, and we want the
// IsolateData objects to live as long as the map itself. The last element of
// the vector is always the currently active IsolateData, and the other elements
// refer to isolates that have been shutdown.
using IsolateDataMap =
    std::unordered_map<Dart_Isolate, std::vector<std::unique_ptr<IsolateData>>>;

// Using a thread local isolate data map so that MultithreadedCreation test
// can avoid using locks while creating isolates on multiple threads. A lock
// would sync up the threads, so would defeat the purpose of the test.
FML_THREAD_LOCAL fml::ThreadLocalUniquePtr<IsolateDataMap> isolate_data_map_;

class PlatformIsolateManagerTest : public FixtureTest {
 public:
  PlatformIsolateManagerTest() {}

  void TestWithRootIsolate(std::function<void()> test) {
    ASSERT_FALSE(DartVMRef::IsInstanceRunning());
    auto settings = CreateSettingsForFixture();
    auto vm_ref = DartVMRef::Create(settings);
    ASSERT_TRUE(vm_ref);
    auto vm_data = vm_ref.GetVMData();
    ASSERT_TRUE(vm_data);

    Dart_IsolateFlags flags;
    Dart_IsolateFlagsInitialize(&flags);
    char* error = nullptr;
    root_isolate_ = Dart_CreateIsolateGroup(
        "main.dart", "RootIsolate",
        vm_data->GetIsolateSnapshot()->GetDataMapping(),
        vm_data->GetIsolateSnapshot()->GetInstructionsMapping(), &flags,
        nullptr, nullptr, &error);
    ASSERT_TRUE(root_isolate_);
    Dart_ExitIsolate();

    test();

    Dart_EnterIsolate(root_isolate_);
    Dart_ShutdownIsolate();
  }

  Dart_Isolate CreateAndRegisterIsolate(PlatformIsolateManager* mgr) {
    if (isolate_data_map_.get() == nullptr) {
      isolate_data_map_.reset(new IsolateDataMap());
    }

    IsolateData* isolate_data = new IsolateData(mgr);
    char* error = nullptr;
    Dart_Isolate isolate =
        Dart_CreateIsolateInGroup(root_isolate_, "TestIsolate", OnShutdown,
                                  nullptr, isolate_data, &error);
    isolate_data->isolate = isolate;
    EXPECT_TRUE(isolate);
    Dart_ExitIsolate();

    (*isolate_data_map_.get())[isolate].push_back(
        std::unique_ptr<IsolateData>(isolate_data));
    isolate_data->is_registered = mgr->RegisterPlatformIsolate(isolate);

    return isolate;
  }

  bool IsolateIsShutdown(Dart_Isolate isolate) {
    EXPECT_EQ(1u, isolate_data_map_.get()->count(isolate));
    EXPECT_LT(0u, (*isolate_data_map_.get())[isolate].size());
    return (*isolate_data_map_.get())[isolate].back()->is_shutdown;
  }

  bool IsolateIsRegistered(Dart_Isolate isolate) {
    EXPECT_EQ(1u, isolate_data_map_.get()->count(isolate));
    EXPECT_LT(0u, (*isolate_data_map_.get())[isolate].size());
    return (*isolate_data_map_.get())[isolate].back()->is_registered;
  }

 private:
  Dart_Isolate root_isolate_ = nullptr;

  static void OnShutdown(void*, void* raw_isolate_data) {
    IsolateData* isolate_data =
        reinterpret_cast<IsolateData*>(raw_isolate_data);
    EXPECT_TRUE(isolate_data->isolate);
    EXPECT_FALSE(isolate_data->is_shutdown);
    isolate_data->is_shutdown = true;
    if (isolate_data->is_registered) {
      isolate_data->mgr->RemovePlatformIsolate(isolate_data->isolate);
      isolate_data->is_registered = false;
    }
  }

  FML_DISALLOW_COPY_AND_ASSIGN(PlatformIsolateManagerTest);
};

TEST_F(PlatformIsolateManagerTest, OrdinaryFlow) {
  TestWithRootIsolate([this]() {
    PlatformIsolateManager mgr;
    EXPECT_FALSE(mgr.IsShutdown());

    Dart_Isolate isolateA = CreateAndRegisterIsolate(&mgr);
    ASSERT_TRUE(isolateA);
    EXPECT_FALSE(IsolateIsShutdown(isolateA));
    EXPECT_TRUE(IsolateIsRegistered(isolateA));
    EXPECT_TRUE(mgr.IsRegistered(isolateA));

    Dart_Isolate isolateB = CreateAndRegisterIsolate(&mgr);
    ASSERT_TRUE(isolateB);
    EXPECT_FALSE(IsolateIsShutdown(isolateB));
    EXPECT_TRUE(IsolateIsRegistered(isolateB));
    EXPECT_TRUE(mgr.IsRegistered(isolateB));

    mgr.ShutdownPlatformIsolates();
    EXPECT_TRUE(mgr.IsShutdown());

    EXPECT_TRUE(IsolateIsShutdown(isolateA));
    EXPECT_FALSE(IsolateIsRegistered(isolateA));
    EXPECT_FALSE(mgr.IsRegistered(isolateA));
    EXPECT_TRUE(IsolateIsShutdown(isolateB));
    EXPECT_FALSE(IsolateIsRegistered(isolateB));
    EXPECT_FALSE(mgr.IsRegistered(isolateB));
  });
}

TEST_F(PlatformIsolateManagerTest, EarlyShutdown) {
  TestWithRootIsolate([this]() {
    PlatformIsolateManager mgr;
    EXPECT_FALSE(mgr.IsShutdown());

    Dart_Isolate isolateA = CreateAndRegisterIsolate(&mgr);
    ASSERT_TRUE(isolateA);
    EXPECT_FALSE(IsolateIsShutdown(isolateA));
    EXPECT_TRUE(IsolateIsRegistered(isolateA));
    EXPECT_TRUE(mgr.IsRegistered(isolateA));

    Dart_Isolate isolateB = CreateAndRegisterIsolate(&mgr);
    ASSERT_TRUE(isolateB);
    EXPECT_FALSE(IsolateIsShutdown(isolateB));
    EXPECT_TRUE(IsolateIsRegistered(isolateB));
    EXPECT_TRUE(mgr.IsRegistered(isolateB));

    Dart_EnterIsolate(isolateA);
    Dart_ShutdownIsolate();
    EXPECT_TRUE(IsolateIsShutdown(isolateA));
    EXPECT_FALSE(IsolateIsRegistered(isolateA));
    EXPECT_FALSE(mgr.IsRegistered(isolateA));

    Dart_EnterIsolate(isolateB);
    Dart_ShutdownIsolate();
    EXPECT_TRUE(IsolateIsShutdown(isolateB));
    EXPECT_FALSE(IsolateIsRegistered(isolateB));
    EXPECT_FALSE(mgr.IsRegistered(isolateB));

    mgr.ShutdownPlatformIsolates();
    EXPECT_TRUE(mgr.IsShutdown());

    EXPECT_TRUE(IsolateIsShutdown(isolateA));
    EXPECT_FALSE(IsolateIsRegistered(isolateA));
    EXPECT_FALSE(mgr.IsRegistered(isolateA));
    EXPECT_TRUE(IsolateIsShutdown(isolateB));
    EXPECT_FALSE(IsolateIsRegistered(isolateB));
    EXPECT_FALSE(mgr.IsRegistered(isolateB));
  });
}

TEST_F(PlatformIsolateManagerTest, RegistrationAfterShutdown) {
  TestWithRootIsolate([this]() {
    PlatformIsolateManager mgr;
    EXPECT_FALSE(mgr.IsShutdown());

    Dart_Isolate isolateA = CreateAndRegisterIsolate(&mgr);
    ASSERT_TRUE(isolateA);
    EXPECT_FALSE(IsolateIsShutdown(isolateA));
    EXPECT_TRUE(IsolateIsRegistered(isolateA));
    EXPECT_TRUE(mgr.IsRegistered(isolateA));

    mgr.ShutdownPlatformIsolates();
    EXPECT_TRUE(mgr.IsShutdown());

    EXPECT_TRUE(IsolateIsShutdown(isolateA));
    EXPECT_FALSE(IsolateIsRegistered(isolateA));
    EXPECT_FALSE(mgr.IsRegistered(isolateA));

    Dart_Isolate isolateB = CreateAndRegisterIsolate(&mgr);
    ASSERT_TRUE(isolateB);
    EXPECT_FALSE(IsolateIsShutdown(isolateB));
    EXPECT_FALSE(IsolateIsRegistered(isolateB));
    EXPECT_FALSE(mgr.IsRegistered(isolateB));

    Dart_EnterIsolate(isolateB);
    Dart_ShutdownIsolate();
    EXPECT_TRUE(IsolateIsShutdown(isolateB));
    EXPECT_FALSE(IsolateIsRegistered(isolateB));
    EXPECT_FALSE(mgr.IsRegistered(isolateB));
  });
}

TEST_F(PlatformIsolateManagerTest, MultithreadedCreation) {
  // The goal of this test is to hit the second is_shutdown_ check in
  // PlatformIsolateManager::RegisterPlatformIsolate. There's no way to do this
  // reliably, so just try to generate race conditions by creating Isolates on
  // multiple threads, while shutting down the manager.
  TestWithRootIsolate([this]() {
    PlatformIsolateManager mgr;
    EXPECT_FALSE(mgr.IsShutdown());

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
      threads.push_back(std::thread([this, &mgr]() {
        for (int j = 0; j < 100; ++j) {
          Dart_Isolate isolate = CreateAndRegisterIsolate(&mgr);
          ASSERT_TRUE(isolate);
          EXPECT_FALSE(IsolateIsShutdown(isolate));

          if (!IsolateIsRegistered(isolate)) {
            Dart_EnterIsolate(isolate);
            Dart_ShutdownIsolate();
          }
        }
      }));
    }

    mgr.ShutdownPlatformIsolates();
    EXPECT_TRUE(mgr.IsShutdown());

    for (auto& thread : threads) {
      thread.join();
    }
  });
}

}  // namespace testing
}  // namespace flutter
