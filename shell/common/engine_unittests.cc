// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// FLUTTER_NOLINT

#include "flutter/runtime/dart_vm_lifecycle.h"
#include "flutter/shell/common/engine.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/testing.h"
#include "gmock/gmock.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace flutter {

namespace {
class MockDelegate : public Engine::Delegate {
  MOCK_METHOD(void,
              OnEngineUpdateSemantics,
              (SemanticsNodeUpdates, CustomAccessibilityActionUpdates),
              (override));
  MOCK_METHOD(void,
              OnEngineHandlePlatformMessage,
              (fml::RefPtr<PlatformMessage>),
              (override));
  MOCK_METHOD(void, OnPreEngineRestart, (), (override));
  MOCK_METHOD(void,
              UpdateIsolateDescription,
              (const std::string, int64_t),
              (override));
  MOCK_METHOD(void, SetNeedsReportTimings, (bool), (override));

  MOCK_METHOD(std::unique_ptr<std::vector<std::string>>,
              ComputePlatformResolvedLocale,
              (const std::vector<std::string>&),
              (override));
};

class MockResponse : public PlatformMessageResponse {
 public:
  MOCK_METHOD(void, Complete, (std::unique_ptr<fml::Mapping> data), (override));
  MOCK_METHOD(void, CompleteEmpty, (), (override));
};

class MockRuntimeDelegate : public RuntimeDelegate {
 public:
  MOCK_METHOD(std::string, DefaultRouteName, (), (override));
  MOCK_METHOD(void, ScheduleFrame, (bool), (override));
  MOCK_METHOD(void, Render, (std::unique_ptr<flutter::LayerTree>), (override));
  MOCK_METHOD(void,
              UpdateSemantics,
              (SemanticsNodeUpdates, CustomAccessibilityActionUpdates),
              (override));
  MOCK_METHOD(void,
              HandlePlatformMessage,
              (fml::RefPtr<PlatformMessage>),
              (override));
  MOCK_METHOD(FontCollection&, GetFontCollection, (), (override));
  MOCK_METHOD(void,
              UpdateIsolateDescription,
              (const std::string, int64_t),
              (override));
  MOCK_METHOD(void, SetNeedsReportTimings, (bool), (override));

  MOCK_METHOD(std::unique_ptr<std::vector<std::string>>,
              ComputePlatformResolvedLocale,
              (const std::vector<std::string>&),
              (override));
};

class MockRuntimeController : public RuntimeController {
 public:
  MockRuntimeController(RuntimeDelegate& client, TaskRunners p_task_runners)
      : RuntimeController(client, p_task_runners) {}
  MOCK_METHOD(bool, IsRootIsolateRunning, (), (override, const));
  MOCK_METHOD(bool,
              DispatchPlatformMessage,
              (fml::RefPtr<PlatformMessage>),
              (override));
};

fml::RefPtr<PlatformMessage> MakePlatformMessage(
    const std::string& channel,
    const std::map<std::string, std::string>& values,
    fml::RefPtr<PlatformMessageResponse> response) {
  rapidjson::Document document;
  auto& allocator = document.GetAllocator();
  document.SetObject();

  for (const auto& pair : values) {
    rapidjson::Value key(pair.first.c_str(), strlen(pair.first.c_str()),
                         allocator);
    rapidjson::Value value(pair.second.c_str(), strlen(pair.second.c_str()),
                           allocator);
    document.AddMember(key, value, allocator);
  }

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);
  const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer.GetString());

  fml::RefPtr<PlatformMessage> message = fml::MakeRefCounted<PlatformMessage>(
      channel, std::vector<uint8_t>(data, data + buffer.GetSize()), response);
  return message;
}

class EngineTest : public ::testing::Test {
 public:
  EngineTest()
      : thread_host_("EngineTest",
                     ThreadHost::Type::Platform | ThreadHost::Type::IO |
                         ThreadHost::Type::UI | ThreadHost::Type::GPU),
        task_runners_({
            "EngineTest",
            thread_host_.platform_thread->GetTaskRunner(),  // platform
            thread_host_.raster_thread->GetTaskRunner(),    // raster
            thread_host_.ui_thread->GetTaskRunner(),        // ui
            thread_host_.io_thread->GetTaskRunner()         // io
        }) {}

  void PostUITaskSync(const std::function<void()>& function) {
    fml::AutoResetWaitableEvent latch;
    task_runners_.GetUITaskRunner()->PostTask([&] {
      function();
      latch.Signal();
    });
    latch.Wait();
  }

 protected:
  void SetUp() override {
    dispatcher_maker_ = [](PointerDataDispatcher::Delegate&) {
      return nullptr;
    };
  }

  MockDelegate delegate_;
  PointerDataDispatcherMaker dispatcher_maker_;
  ThreadHost thread_host_;
  TaskRunners task_runners_;
  Settings settings_;
  std::unique_ptr<Animator> animator_;
  fml::WeakPtr<IOManager> io_manager_;
  std::unique_ptr<RuntimeController> runtime_controller_;
  std::shared_ptr<fml::ConcurrentTaskRunner> image_decoder_task_runner_;
};
}  // namespace

TEST_F(EngineTest, Create) {
  PostUITaskSync([this] {
    auto engine = std::make_unique<Engine>(
        /*delegate=*/delegate_,
        /*dispatcher_maker=*/dispatcher_maker_,
        /*image_decoder_task_runner=*/image_decoder_task_runner_,
        /*task_runners=*/task_runners_,
        /*settings=*/settings_,
        /*animator=*/std::move(animator_),
        /*io_manager=*/io_manager_,
        /*runtime_controller=*/std::move(runtime_controller_));
    EXPECT_TRUE(engine);
  });
}

TEST_F(EngineTest, DispatchPlatformMessageUnknown) {
  PostUITaskSync([this] {
    MockRuntimeDelegate client;
    auto mock_runtime_controller =
        std::make_unique<MockRuntimeController>(client, task_runners_);
    EXPECT_CALL(*mock_runtime_controller, IsRootIsolateRunning())
        .WillRepeatedly(::testing::Return(false));
    auto engine = std::make_unique<Engine>(
        /*delegate=*/delegate_,
        /*dispatcher_maker=*/dispatcher_maker_,
        /*image_decoder_task_runner=*/image_decoder_task_runner_,
        /*task_runners=*/task_runners_,
        /*settings=*/settings_,
        /*animator=*/std::move(animator_),
        /*io_manager=*/io_manager_,
        /*runtime_controller=*/std::move(mock_runtime_controller));

    fml::RefPtr<PlatformMessageResponse> response =
        fml::MakeRefCounted<MockResponse>();
    fml::RefPtr<PlatformMessage> message =
        fml::MakeRefCounted<PlatformMessage>("foo", response);
    engine->DispatchPlatformMessage(message);
  });
}

TEST_F(EngineTest, DispatchPlatformMessageInitialRoute) {
  PostUITaskSync([this] {
    MockRuntimeDelegate client;
    auto mock_runtime_controller =
        std::make_unique<MockRuntimeController>(client, task_runners_);
    EXPECT_CALL(*mock_runtime_controller, IsRootIsolateRunning())
        .WillRepeatedly(::testing::Return(false));
    auto engine = std::make_unique<Engine>(
        /*delegate=*/delegate_,
        /*dispatcher_maker=*/dispatcher_maker_,
        /*image_decoder_task_runner=*/image_decoder_task_runner_,
        /*task_runners=*/task_runners_,
        /*settings=*/settings_,
        /*animator=*/std::move(animator_),
        /*io_manager=*/io_manager_,
        /*runtime_controller=*/std::move(mock_runtime_controller));

    fml::RefPtr<PlatformMessageResponse> response =
        fml::MakeRefCounted<MockResponse>();
    std::map<std::string, std::string> values{
        {"method", "setInitialRoute"},
        {"args", "test_initial_route"},
    };
    fml::RefPtr<PlatformMessage> message =
        MakePlatformMessage("flutter/navigation", values, response);
    engine->DispatchPlatformMessage(message);
    EXPECT_EQ(engine->InitialRoute(), "test_initial_route");
  });
}

TEST_F(EngineTest, DispatchPlatformMessageInitialRouteIgnored) {
  PostUITaskSync([this] {
    MockRuntimeDelegate client;
    auto mock_runtime_controller =
        std::make_unique<MockRuntimeController>(client, task_runners_);
    EXPECT_CALL(*mock_runtime_controller, IsRootIsolateRunning())
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*mock_runtime_controller, DispatchPlatformMessage(::testing::_))
        .WillRepeatedly(::testing::Return(true));
    auto engine = std::make_unique<Engine>(
        /*delegate=*/delegate_,
        /*dispatcher_maker=*/dispatcher_maker_,
        /*image_decoder_task_runner=*/image_decoder_task_runner_,
        /*task_runners=*/task_runners_,
        /*settings=*/settings_,
        /*animator=*/std::move(animator_),
        /*io_manager=*/io_manager_,
        /*runtime_controller=*/std::move(mock_runtime_controller));

    fml::RefPtr<PlatformMessageResponse> response =
        fml::MakeRefCounted<MockResponse>();
    std::map<std::string, std::string> values{
        {"method", "setInitialRoute"},
        {"args", "test_initial_route"},
    };
    fml::RefPtr<PlatformMessage> message =
        MakePlatformMessage("flutter/navigation", values, response);
    engine->DispatchPlatformMessage(message);
    EXPECT_EQ(engine->InitialRoute(), "");
  });
}

}  // namespace flutter
