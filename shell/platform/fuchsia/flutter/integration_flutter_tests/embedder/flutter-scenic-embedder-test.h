// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_TESTS_INTEGRATION_FLUTTER_TESTS_EMBEDDER_FLUTTER_SCENIC_EMBEDDER_TEST_H_
#define SRC_UI_TESTS_INTEGRATION_FLUTTER_TESTS_EMBEDDER_FLUTTER_SCENIC_EMBEDDER_TEST_H_

#include <fuchsia/ui/policy/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <gtest/gtest.h>
#include <lib/async/cpp/task.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/sys/cpp/testing/test_with_environment.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>
#include <lib/zx/clock.h>
#include <zircon/status.h>
#include <zircon/time.h>
#include "flutter/fml/logging.h"

#include <vector>

#include "src/lib/ui/base_view/embedded_view_utils.h"
#include "src/ui/testing/views/color.h"
#include "src/ui/testing/views/embedder_view.h"

namespace flutter_scenic_embedder_test {

/// Defines a list of services that are injected into the test environment.
/// Unlike the injected-services in CMX which are injected per test package,
/// these are injected per test and result in a more hermetic test environment.
const std::vector<std::pair<const char*, const char*>> GetInjectedServices();

// Timeout when waiting on Scenic API calls like |GetDisplayInfo|.
constexpr zx::duration kCallTimeout = zx::sec(5);
// Timeout for Scenic's |TakeScreenshot| FIDL call.
constexpr zx::duration kScreenshotTimeout = zx::sec(10);
// Timeout to fail the test if it goes beyond this duration.
constexpr zx::duration kTestTimeout = zx::min(1);

class FlutterScenicEmbedderTestsBase : public sys::testing::TestWithEnvironment,
                                       public ::testing::Test {
 public:
  explicit FlutterScenicEmbedderTestsBase(
      const std::vector<std::pair<const char*, const char*>> injected_services)
      : injected_services_(std::move(injected_services)) {}

  // |testing::Test|
  void SetUp() override {
    Test::SetUp();
    // This is done in |SetUp| as opposed to the constructor to allow subclasses
    // the opportunity to override |CreateServices()|.
    auto services = TestWithEnvironment::CreateServices();
    CreateServices(services);

    // Add test-specific launchable services.
    for (const auto& service_info : injected_services_) {
      zx_status_t status = services->AddServiceWithLaunchInfo(
          {.url = service_info.second}, service_info.first);
      FML_CHECK(status == ZX_OK)
          << "Failed to add service " << service_info.first;
    }

    environment_ = CreateNewEnclosingEnvironment(
        "flutter-scenic-embedder-tests", std::move(services),
        {.inherit_parent_services = true});
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

  // Configures services available to the test environment. This method is
  // called by |SetUp()|. It shadows but calls
  // |TestWithEnvironment::CreateServices()|.
  virtual void CreateServices(
      std::unique_ptr<sys::testing::EnvironmentServices>& services) {}

  sys::testing::EnclosingEnvironment* environment() {
    return environment_.get();
  }

  fuchsia::ui::views::ViewToken CreatePresentationViewToken() {
    auto [view_token, view_holder_token] = scenic::ViewTokenPair::New();

    auto presenter =
        environment()->ConnectToService<fuchsia::ui::policy::Presenter>();
    presenter.set_error_handler([](zx_status_t status) {
      FAIL() << "presenter: " << zx_status_get_string(status);
    });
    presenter->PresentView(std::move(view_holder_token), nullptr);

    return std::move(view_token);
  }

 private:
  const std::vector<std::pair<const char*, const char*>> injected_services_;
  std::unique_ptr<sys::ComponentContext> const component_context_;
  std::unique_ptr<sys::testing::EnclosingEnvironment> environment_;
};

class FlutterScenicEmbedderTests : public FlutterScenicEmbedderTestsBase {
 public:
  FlutterScenicEmbedderTests()
      : FlutterScenicEmbedderTestsBase(GetInjectedServices()) {}

  // |testing::Test|
  void SetUp() override {
    ASSERT_NO_FATAL_FAILURE(FlutterScenicEmbedderTestsBase::SetUp());

    // Connect to scenic to ensure it is up and running.
    auto scenic =
        environment()->ConnectToService<fuchsia::ui::scenic::Scenic>();
    scenic->GetDisplayInfo(
        //        [this](fuchsia::ui::gfx::DisplayInfo info) { QuitLoop(); });
        // TODO(richkadel): remove this version and uncomment above
        [this](fuchsia::ui::gfx::DisplayInfo info) {
          FML_VLOG(fml::LOG_INFO) << "Got DisplayInfo from scenic";
          QuitLoop();
          FML_VLOG(fml::LOG_INFO) << "quitted loop?";
        });
    // TODO(richkadel): remove this log
    FML_VLOG(fml::LOG_INFO) << "Calling RunLoopWithTimeout()";
    RunLoopWithTimeout(kCallTimeout);
    // TODO(richkadel): remove this log
    FML_VLOG(fml::LOG_INFO) << "Exited RunLoopWithTimeout()";
  }

  void RunAppWithArgs(const std::string& component_url,
                      const std::vector<std::string>& component_args = {}) {
    fuchsia::ui::scenic::ScenicPtr scenic;
    environment()->ConnectToService(scenic.NewRequest());
    scenic.set_error_handler([](zx_status_t status) {
      FAIL() << "Lost connection to Scenic: " << zx_status_get_string(status);
    });

    // TODO(richkadel): remove this log
    FML_VLOG(fml::LOG_INFO)
        << "Launching flutter_runner, component_url=" << component_url;
    scenic::EmbeddedViewInfo flutter_runner =
        scenic::LaunchComponentAndCreateView(environment()->launcher_ptr(),
                                             component_url, component_args);
    /*

    [2021-08-16 21:58:31][15084][15086][pkg-resolver] INFO: Fetching blobs for
    fuchsia-pkg://google3-devhost/parent-view/0: [] [2021-08-16
    21:58:31][15084][15086][pkg-resolver] INFO: resolved
    fuchsia-pkg://fuchsia.com/parent-view/0 as
    fuchsia-pkg://google3-devhost/parent-view/0 to
    22634f985109075d736ca3c3090350c4cb04c093d512ceea84f5520742ed1640 with TUF
    [2021-08-16 21:58:31][20145][20147][sysmgr.cmx] ERROR:
    [src/sys/sysmgr/package_updating_loader.cc(86)] Could not load package
    resource meta/parent_view.cmx from
    fuchsia-pkg://fuchsia.com/parent-view#meta/parent_view.cmx

    but the FML_VLOG above prints:
    [259863.637267][36607967][36607969][flutter_scenic_embedder_test.cm] INFO:
    [flutter-scenic-embedder-test.h(146)] Launching flutter_runner,
    component_url=fuchsia-pkg://fuchsia.com/parent-view#meta/parent_view.cmx

    fuchsia-pkg://google3-devhost/parent-view/0
    fuchsia-pkg://fuchsia.com/parent-view#meta/parent_view.cmx

    Compare to these successful logs, JUST before trying to resolve parent_view:

    [2021-08-16 22:24:22][15084][15086][pkg-resolver] INFO: Fetching blobs for
    fuchsia-pkg://google3-devhost/ktrace_provider/0: [] [2021-08-16
    22:24:22][15084][15086][pkg-resolver] INFO: resolved
    fuchsia-pkg://fuchsia.com/ktrace_provider/0 as
    fuchsia-pkg://google3-devhost/ktrace_provider/0 to
    53eba946b3fb22d0347755e88eaa7ef10cc8552809017363337e8b98fb0e9e41 with TUF
    */
    // flutter_runner.controller.events().OnTerminated = [](auto...) { FAIL();
    // };
    // TODO(richkadel): remove this assignment to OnTerminated and uncomment
    // above
    flutter_runner.controller.events().OnTerminated = [](auto...) {
      // TODO(richkadel): remove this log
      FML_VLOG(fml::LOG_INFO)
          << "flutter_runner.controller terminated. Failing test.";
      FAIL();
    };

    // Present the view.
    embedder_view_.emplace(scenic::ViewContext{
        .session_and_listener_request =
            scenic::CreateScenicSessionPtrAndListenerRequest(scenic.get()),
        .view_token = CreatePresentationViewToken(),
    });

    // Embed the view.
    bool is_rendering = false;
    embedder_view_->EmbedView(
        std::move(flutter_runner),
        [&is_rendering](fuchsia::ui::gfx::ViewState view_state) {
          is_rendering = view_state.is_rendering;
        });
    RunLoopUntil([&is_rendering] { return is_rendering; });
    FML_LOG(INFO) << "Launched component: " << component_url;
  }

  scenic::Screenshot TakeScreenshot() {
    fuchsia::ui::scenic::ScenicPtr scenic;
    environment()->ConnectToService(scenic.NewRequest());
    scenic.set_error_handler([](zx_status_t status) {
      FAIL() << "Lost connection to Scenic: " << zx_status_get_string(status);
    });

    FML_LOG(INFO) << "Taking screenshot... ";
    fuchsia::ui::scenic::ScreenshotData screenshot_out;
    scenic->TakeScreenshot(
        [this, &screenshot_out](fuchsia::ui::scenic::ScreenshotData screenshot,
                                bool status) {
          EXPECT_TRUE(status) << "Failed to take screenshot";
          screenshot_out = std::move(screenshot);
          QuitLoop();
        });
    EXPECT_FALSE(RunLoopWithTimeout(kScreenshotTimeout))
        << "Timed out waiting for screenshot.";
    FML_LOG(INFO) << "Screenshot captured.";

    return scenic::Screenshot(screenshot_out);
  }

  bool TakeScreenshotUntil(
      scenic::Color color,
      fit::function<void(std::map<scenic::Color, size_t>)> callback = nullptr,
      zx::duration timeout = kTestTimeout) {
    return RunLoopWithTimeoutOrUntil(
        [this, &callback, &color] {
          // TODO(richkadel): remove this log
          FML_VLOG(fml::LOG_INFO) << "Calling TakeScreenshot()";
          auto screenshot = TakeScreenshot();
          // TODO(richkadel): remove this log
          FML_VLOG(fml::LOG_INFO) << "Exited from TakeScreenshot()";
          auto histogram = screenshot.Histogram();
          // TODO(richkadel): remove this log
          FML_VLOG(fml::LOG_INFO) << "Exited from screenshot.Histogram()";

          bool color_found = histogram[color] > 0;
          if (color_found && callback != nullptr) {
            callback(std::move(histogram));
          }
          // TODO(richkadel): remove this log
          FML_VLOG(fml::LOG_INFO) << "returning color_found=" << color_found;
          return color_found;
        },
        timeout);
  }

  // Inject directly into Root Presenter, using fuchsia.ui.input FIDLs.
  void InjectInput() {
    using fuchsia::ui::input::InputReport;
    // Device parameters
    auto parameters = fuchsia::ui::input::TouchscreenDescriptor::New();
    *parameters = {.x = {.range = {.min = -1000, .max = 1000}},
                   .y = {.range = {.min = -1000, .max = 1000}},
                   .max_finger_id = 10};

    FML_LOG(INFO) << "Injecting input... ";
    // Register it against Root Presenter.
    fuchsia::ui::input::DeviceDescriptor device{.touchscreen =
                                                    std::move(parameters)};
    auto registry =
        environment()
            ->ConnectToService<fuchsia::ui::input::InputDeviceRegistry>();
    fuchsia::ui::input::InputDevicePtr connection;
    registry->RegisterDevice(std::move(device), connection.NewRequest());

    {
      // Inject one input report, then a conclusion (empty) report.
      auto touch = fuchsia::ui::input::TouchscreenReport::New();
      *touch = {
          .touches = {{.finger_id = 1, .x = 0, .y = 0}}};  // center of display
      InputReport report{.event_time = TimeToUint(MonotonicNow()),
                         .touchscreen = std::move(touch)};
      connection->DispatchReport(std::move(report));
    }

    {
      auto touch = fuchsia::ui::input::TouchscreenReport::New();
      InputReport report{.event_time = TimeToUint(MonotonicNow()),
                         .touchscreen = std::move(touch)};
      connection->DispatchReport(std::move(report));
    }
    FML_LOG(INFO) << "Input dispatched.";
  }

 private:
  zx::time MonotonicNow() { return zx::clock::get_monotonic(); };

  template <typename TimeT>
  uint64_t TimeToUint(const TimeT& time) {
    FML_CHECK(time.get() >= 0);
    return static_cast<uint64_t>(time.get());
  };
  // Wrapped in optional since the view is not created until the middle of SetUp
  std::optional<scenic::EmbedderView> embedder_view_;
};

}  // namespace flutter_scenic_embedder_test

#endif  // SRC_UI_TESTS_INTEGRATION_FLUTTER_TESTS_EMBEDDER_FLUTTER_SCENIC_EMBEDDER_TEST_H_
