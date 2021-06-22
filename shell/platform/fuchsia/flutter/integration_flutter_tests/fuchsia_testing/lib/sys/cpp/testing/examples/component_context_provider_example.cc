// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/async/dispatcher.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/gtest/test_loop_fixture.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/sys/cpp/testing/component_context_provider.h>
#include <lib/sys/cpp/testing/fake_component.h>
#include <lib/sys/cpp/testing/fake_launcher.h>

#include <memory>

#include "garnet/examples/fidl/echo_client_cpp/echo_client_app.h"

// This test file demonstrates how to use |ComponentContextProvider|.

namespace echo {
namespace testing {

using fidl::examples::echo::Echo;
using fidl::examples::echo::EchoPtr;

// Fake server, which the client under test will be used against
class FakeEcho : public Echo {
 public:
  static const char kURL[];

  fidl::InterfaceRequestHandler<Echo> GetHandler() { return bindings_.GetHandler(this); }

  FakeEcho() { component_.AddPublicService(bindings_.GetHandler(this)); }

  // Fake implementation of server-side logic
  void EchoString(fidl::StringPtr value, EchoStringCallback callback) { callback(answer_); }

  void SetAnswer(fidl::StringPtr answer) { answer_ = answer; }

  // Register to be launched with a fake URL
  void Register(sys::testing::FakeLauncher& fake_launcher) {
    component_.Register(kURL, fake_launcher);
  }

 private:
  sys::testing::FakeComponent component_;
  fidl::BindingSet<Echo> bindings_;
  fidl::StringPtr answer_;
};

const char FakeEcho::kURL[] = "fake-echo";

class EchoClientAppForTest : public EchoClientApp {
 public:
  // Expose injecting constructor so we can pass an instrumented Context
  EchoClientAppForTest(std::unique_ptr<sys::ComponentContext> context)
      : EchoClientApp(std::move(context)) {}
};

class TestWithContextExampleTest : public gtest::TestLoopFixture {
 public:
  // Creates a fake echo component and registers it with fake launcher so that
  // when app under test tries to launch echo server, it launches our fake
  // component.
  void SetUp() override {
    TestLoopFixture::SetUp();
    echoClientApp_.reset(new EchoClientAppForTest(provider_.TakeContext()));
    provider_.service_directory_provider()->AddService(fake_launcher_.GetHandler());

    fake_echo_.reset(new FakeEcho());
    fake_echo_->Register(fake_launcher_);
  }

  void TearDown() override {
    echoClientApp_.reset();
    TestLoopFixture::TearDown();
  }

 protected:
  void Start(std::string server_url) { echoClientApp_->Start(server_url); }
  EchoPtr& echo() { return echoClientApp_->echo(); }
  void SetAnswer(fidl::StringPtr answer) { fake_echo_->SetAnswer(answer); }

 private:
  std::unique_ptr<EchoClientAppForTest> echoClientApp_;
  std::unique_ptr<FakeEcho> fake_echo_;
  sys::testing::FakeLauncher fake_launcher_;
  sys::testing::ComponentContextProvider provider_;
};

// Demonstrates use of fake component and launcher when component is not
// actually started.
TEST_F(TestWithContextExampleTest, EchoString_HelloWorld_GoodbyeWorld) {
  fidl::StringPtr message = "bogus";
  Start(FakeEcho::kURL);
  SetAnswer("Goodbye World!");
  echo()->EchoString("Hello World!", [&](::fidl::StringPtr retval) { message = retval; });
  RunLoopUntilIdle();
  EXPECT_EQ("Goodbye World!", message);
}

// Demonstrates coorect use of fake component and fake launcher.
// In this we are not starting echo service so we will not get any reply from
// server.
TEST_F(TestWithContextExampleTest, EchoString_NoStart) {
  fidl::StringPtr message = "bogus";
  echo()->EchoString("Hello World!", [&](::fidl::StringPtr retval) { message = retval; });
  RunLoopUntilIdle();
  EXPECT_EQ("bogus", message);
}

// This fixture will directly put fake service inside incoming service of mocked
// start up context. This way client under test can directly connect to that
// service using component context.
class FakeEchoInContextExampleTest : public gtest::TestLoopFixture {
 public:
  // Adds a fake echo service to incoming service of mocked component context.
  void SetUp() override {
    TestLoopFixture::SetUp();
    fake_echo_.reset(new FakeEcho());
    provider_.service_directory_provider()->AddService(fake_echo_->GetHandler());
  }

  void TearDown() override { TestLoopFixture::TearDown(); }

 protected:
  void SetAnswer(fidl::StringPtr answer) { fake_echo_->SetAnswer(answer); }
  EchoPtr echo() { return provider_.context()->svc()->Connect<Echo>(); }

 private:
  std::unique_ptr<FakeEcho> fake_echo_;
  sys::testing::ComponentContextProvider provider_;
};

// Demonstrates how to directly add fake services to incoming directory of
// mocked out context and then how to connect to it to use it.
//
// This example can be used to test apps which connect to services using
// component context.
TEST_F(FakeEchoInContextExampleTest, EchoString_HelloWorld_GoodbyeWorld) {
  fidl::StringPtr message = "bogus";
  SetAnswer("Goodbye World!");
  auto echo_ptr = echo();
  echo_ptr->EchoString("Hello World!", [&](::fidl::StringPtr retval) { message = retval; });
  RunLoopUntilIdle();
  EXPECT_EQ("Goodbye World!", message);
}

}  // namespace testing
}  // namespace echo
