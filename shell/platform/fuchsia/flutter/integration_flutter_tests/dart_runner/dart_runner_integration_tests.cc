// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuchsia/sys/cpp/fidl.h>
#include <lib/async/cpp/task.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/sys/cpp/testing/test_with_environment.h>
#include <lib/zx/time.h>
#include <zircon/errors.h>
#include <zircon/status.h>
#include <optional>

#include "flutter/fml/logging.h"
#include "gtest/gtest.h"

namespace dart_runner::testing {
namespace {

// The URL of the dart application to launch.
// TODO(dworsham): To run the test serving the runner and test packages from
// the flutter/engine package server (via
// `//flutter/tools/fuchsia/devshell/serve.sh`), change `fuchsia.com` to
// `engine`.
constexpr char kDartAppUrl[] =
    "fuchsia-pkg://fuchsia.com/dart_runner_integration_tests#meta/"
    "fixture.cmx";

// Timeout to fail the test if it goes beyond this duration.
constexpr zx::duration kTestTimeout = zx::min(1);

// The list of services that are injected into the test environment.
const std::vector<std::pair<const char*, const char*>> GetInjectedServices() {
  // TODO "fuchsia.device.NameProvider",  -> appmgr / thus mock out
  // TODO "fuchsia.feedback.CrashReporter", -> forensics / thus mock out
  // TODO "fuchsia.process.Launcher",  -> appmgr / thus mock out
  // TODO "fuchsia.process.Resolver",  -> appmgr / thus mock out
  // TODO "fuchsia.intl.PropertyProvider", -> mock out
  // TODO "fuchsia.net.name.Lookup",  -> netstack / thus mock out
  // TODO "fuchsia.posix.socket.Provider",  -> netstack / thus mock out
  // TODO "fuchsia.process.Resolver",  -> appmgr / thus mock out

  // TODO "fuchsia.logger.LogSink",  -> syslog / thus InheritFromParent
  // TODO "fuchsia.tracing.provider.Registry",  -> trace_manager / thus
  // InheritFromParent
  // std::vector<std::pair<const char*, const char*>> injected_services = {};
  return {};  // injected_services;
}

// Allow these global services from outside the test environment.
const std::vector<std::string> GlobalServices() {
  return {"fuchsia.tracing.provider.Registry"};
}

std::string GetCurrentTestName() {
  return ::testing::UnitTest::GetInstance()->current_test_info()->name();
}

};  // namespace

class DartRunnerIntegrationTest : public ::testing::Test,
                                  public sys::testing::TestWithEnvironment {
 public:
  sys::testing::EnclosingEnvironment* environment() {
    return environment_.get();
  }

  // |testing::Test|
  void SetUp() override {
    Test::SetUp();

    // Create test-specific launchable services.
    auto services = TestWithEnvironment::CreateServices();
    for (const auto& service_info : GetInjectedServices()) {
      zx_status_t status = services->AddServiceWithLaunchInfo(
          {.url = service_info.second}, service_info.first);
      FML_CHECK(status == ZX_OK)
          << "Failed to add service " << service_info.first;
    }

    // Enable services from outside this test.
    for (const auto& service : GlobalServices()) {
      const zx_status_t is_ok = services->AllowParentService(service);
      FML_CHECK(is_ok == ZX_OK) << "Failed to add service " << service;
    }

    environment_ = CreateNewEnclosingEnvironment(GetCurrentTestName(),
                                                 std::move(services));
    WaitForEnclosingEnvToStart(environment());

    FML_VLOG(fml::LOG_INFO) << "Created test environment.";

    // Post a "just in case" quit task, if the test hangs.
    async::PostDelayedTask(
        dispatcher(),
        [] {
          FML_LOG(FATAL)
              << "\n\n>> Test did not complete in time, terminating.  <<\n\n";
        },
        kTestTimeout);
  }

  void RunAppWithArgs(const std::string& component_url,
                      const std::vector<std::string>& component_args = {},
                      int64_t expected_return_code = 0) {
    fuchsia::sys::LaunchInfo launch_info;
    launch_info.url = component_url;
    launch_info.arguments = fidl::VectorPtr(
        std::vector<std::string>(component_args.begin(), component_args.end()));
    auto app_services = sys::ServiceDirectory::CreateWithRequest(
        &launch_info.directory_request);

    std::optional<fuchsia::sys::TerminationReason> termination_reason;
    std::optional<int64_t> return_code;
    fuchsia::sys::ComponentControllerPtr controller;
    controller.events().OnTerminated =
        [&termination_reason, &return_code](
            int64_t retcode, fuchsia::sys::TerminationReason reason) {
          ASSERT_FALSE(termination_reason.has_value());
          termination_reason = reason;
          return_code = retcode;
        };
    environment()->launcher_ptr()->CreateComponent(std::move(launch_info),
                                                   controller.NewRequest());
    FML_LOG(INFO) << "Launched component: " << component_url;

    RunLoopUntil([&termination_reason, &return_code] {
      return (termination_reason.has_value() && return_code.has_value());
    });
    FML_LOG(INFO) << "Component exited (term="
                  << static_cast<uint32_t>(termination_reason.value())
                  << "), ret=" << return_code.value() << "): " << component_url;
    ASSERT_TRUE(termination_reason.has_value());
    ASSERT_TRUE(return_code.has_value());
    EXPECT_EQ(termination_reason.value(),
              fuchsia::sys::TerminationReason::EXITED);
    EXPECT_EQ(return_code.value(), expected_return_code);
  }

 private:
  const std::unique_ptr<sys::ComponentContext> component_context_;
  std::unique_ptr<sys::testing::EnclosingEnvironment> environment_;
};

TEST_F(DartRunnerIntegrationTest, StartupShutdown) {
  constexpr int64_t kStartupShutdownReturnCode = 42;
  RunAppWithArgs(kDartAppUrl, {GetCurrentTestName()},
                 kStartupShutdownReturnCode);
}

TEST_F(DartRunnerIntegrationTest, RunProcess) {
  RunAppWithArgs(kDartAppUrl, {GetCurrentTestName()});
}

}  // namespace dart_runner::testing
