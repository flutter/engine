// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuchsia/sys/cpp/fidl.h>
#include <test/placeholders/cpp/fidl.h>
#include <lib/sys/cpp/testing/test_with_environment.h>

#include "fake_echo.h"

// This test file demostrates how to use |TestWithEnvironment|.

namespace echo {
namespace testing {
namespace {

using fuchsia::sys::LaunchInfo;
using test::placeholders::EchoPtr;
using sys::testing::EnclosingEnvironment;
using sys::testing::EnvironmentServices;
using sys::testing::TestWithEnvironment;

const char kEnvironment[] = "environment_test";
const auto kTimeout = zx::sec(5);
const char kFakeEchoUrl[] =
    "fuchsia-pkg://fuchsia.com/test_with_environment_example_test#meta/"
    "fake_echo_app.cmx";

class TestWithEnvironmentExampleTest : public TestWithEnvironment {
 protected:
  std::unique_ptr<EnclosingEnvironment> enclosing_environment_;
  std::string answer_ = "Goodbye World!";
};

// Demonstrates use adding fake service to EnclosingEnvironment.
TEST_F(TestWithEnvironmentExampleTest, AddFakeEchoAsService) {
  // Start enclosing environment with an injected service.
  std::unique_ptr<EnvironmentServices> services = CreateServices();
  FakeEcho fake_echo;
  services->AddService(fake_echo.GetHandler());
  enclosing_environment_ = CreateNewEnclosingEnvironment(kEnvironment, std::move(services));

  fidl::StringPtr message = "bogus";
  fake_echo.SetAnswer(answer_);
  EchoPtr echo_ptr;
  // You can also launch your component which connects to echo service using
  // enclosing_environment_.CreateComponent(..).
  enclosing_environment_->ConnectToService(echo_ptr.NewRequest());
  echo_ptr->EchoString("Hello World!", [&](::fidl::StringPtr retval) { message = retval; });
  EXPECT_TRUE(RunLoopWithTimeoutOrUntil([&] { return message.value_or("") == answer_; }, kTimeout));
}

// Demonstrates use adding fake service as component to EnclosingEnvironment.
// |enclosing_environment_| launches kFakeEchoUrl when anything tries to connect
// to echo service in |enclosing_environment_|;
TEST_F(TestWithEnvironmentExampleTest, AddFakeEchoAsServiceComponent) {
  // Start enclosing environment with an injected service served by a
  // component.
  std::unique_ptr<EnvironmentServices> services = CreateServices();
  LaunchInfo launch_info;
  launch_info.url = kFakeEchoUrl;
  launch_info.arguments.emplace({answer_});
  services->AddServiceWithLaunchInfo(std::move(launch_info), Echo::Name_);
  enclosing_environment_ = CreateNewEnclosingEnvironment(kEnvironment, std::move(services));

  fidl::StringPtr message = "bogus";
  EchoPtr echo_ptr;
  // You can also launch your component which connects to echo service using
  // enclosing_environment_.CreateComponent(..).
  enclosing_environment_->ConnectToService(echo_ptr.NewRequest());
  echo_ptr->EchoString("Hello World!", [&](::fidl::StringPtr retval) { message = retval; });
  EXPECT_TRUE(RunLoopWithTimeoutOrUntil([&] { return message.value_or("") == answer_; }, kTimeout));
}

}  // namespace
}  // namespace testing
}  // namespace echo
