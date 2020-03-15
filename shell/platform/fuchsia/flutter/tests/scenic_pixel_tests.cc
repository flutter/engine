// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuchsia/ui/policy/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/fit/function.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>
#include <lib/zx/clock.h>
#include <lib/zx/time.h>
#include <zircon/status.h>
#include "flutter/shell/platform/fuchsia/flutter/tests/real_loop_fixture.h"
#include "gtest/gtest.h"

#include <iomanip>
#include <map>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include "flutter/fml/logging.h"
#include "flutter/shell/platform/fuchsia/flutter/tests/embedded_view_utils.h"
#include "flutter/shell/platform/fuchsia/flutter/tests/embedder_view.h"

namespace {

// Max time to wait in failure cases before bailing.
constexpr zx::duration kScreenshotTimeout = zx::sec(15);
constexpr zx::duration kTestTimeout = zx::sec(60);

// Utility class, combined with << operator below, for printing a list
// of colors.
struct Colors {
  const std::vector<uint32_t>& colors;
};

std::ostream& operator<<(std::ostream& os, const Colors& c) {
  for (size_t i = 0; i < c.colors.size(); i++) {
    os << "#" << std::hex << c.colors[i];
    if (i != c.colors.size() - 1) {
      os << ", ";
    }
  }
  return os;
}

template <typename Container>
bool ContainerFromVmo(const zx::vmo& buffer,
                      uint64_t num_bytes,
                      Container* container_ptr) {
  FML_CHECK(container_ptr);

  container_ptr->resize(num_bytes);

  if (num_bytes == 0) {
    return true;
  }

  zx_status_t status = buffer.read(&(*container_ptr)[0], 0, num_bytes);
  if (status < 0) {
    FML_LOG(ERROR) << "zx::vmo::read failed";
    return false;
  }

  return true;
}

bool VectorFromVmo(const fuchsia::mem::Buffer& vmo_transport,
                   std::vector<uint8_t>* vector_ptr) {
  size_t vmo_size;
  zx_status_t zx_status = vmo_transport.vmo.get_size(&vmo_size);
  if (zx_status != ZX_OK) {
    FML_LOG(ERROR) << "Unable to get VMO size";
    return false;
  }
  if (vmo_size < vmo_transport.size) {
    return false;
  }
  return ContainerFromVmo<std::vector<uint8_t>>(vmo_transport.vmo,
                                                vmo_transport.size, vector_ptr);
}

// Given a screenshot, return a histogram of color values.
std::map</*color*/ uint32_t, /*count*/ size_t> Histogram(
    const fuchsia::ui::scenic::ScreenshotData& screenshot) {
  EXPECT_GT(screenshot.info.width, 0u);
  EXPECT_GT(screenshot.info.height, 0u);

  std::vector<uint8_t> data;
  EXPECT_TRUE(VectorFromVmo(screenshot.data, &data)) << "Failed to read screenshot";

  std::map<uint32_t, size_t> histogram;
  const uint32_t* bitmap = reinterpret_cast<const uint32_t*>(data.data());
  const size_t size = screenshot.info.width * screenshot.info.height;
  EXPECT_EQ(size * sizeof(uint32_t), data.size());
  for (size_t i = 0; i < size; ++i) {
    ++histogram[bitmap[i]];
  }

  return histogram;
}

// Base fixture for pixel tests, containing Scenic and presentation setup, and
// screenshot utilities.
class PixelTest : public gtest::RealLoopFixture {
 protected:
  PixelTest() : context_(sys::ComponentContext::Create()) {
    scenic_ = context_->svc()->Connect<fuchsia::ui::scenic::Scenic>();
    scenic_.set_error_handler([](zx_status_t status) {
      FAIL() << "Lost connection to Scenic: " << zx_status_get_string(status);
    });
  }

  sys::ComponentContext* context() { return context_.get(); }
  fuchsia::ui::scenic::Scenic* scenic() { return scenic_.get(); }

  // Gets a view token for presentation by |RootPresenter|.
  fuchsia::ui::views::ViewToken CreatePresentationViewToken() {
    auto [view_token, view_holder_token] = scenic::ViewTokenPair::New();

    auto presenter = context_->svc()->Connect<fuchsia::ui::policy::Presenter>();
    presenter.set_error_handler(
        [](zx_status_t status) { FAIL() << "presenter: " << zx_status_get_string(status); });
    presenter->PresentView(std::move(view_holder_token), nullptr);

    return std::move(view_token);
  }

  bool ScreenshotUntil(fit::function<bool(fuchsia::ui::scenic::ScreenshotData, bool)> condition,
                       zx::duration timeout = kScreenshotTimeout) {
    zx::time start = zx::clock::get_monotonic();
    while (zx::clock::get_monotonic() - start <= timeout) {
      fuchsia::ui::scenic::ScreenshotData screenshot;
      bool ok;
      scenic_->TakeScreenshot(
          [this, &screenshot, &ok](fuchsia::ui::scenic::ScreenshotData screenshot_in, bool status) {
            ok = status;
            screenshot = std::move(screenshot_in);
            QuitLoop();
          });

      if (!RunLoopWithTimeout(timeout) && condition(std::move(screenshot), ok)) {
        return true;
      }
    }

    return false;
  }

  // Check that the most frequent colors in the screenshot match those specified in |colors|.
  // If |colors_ranked_by_frequency|, then |colors| starts with the most frequent color
  // and then decreases in frequency. The screenshot distribution must match this order.
  // If |colors_ranked_by_frequency| is false, then just check that the set of top colors
  // matches |colors|.
  void ExpectTopColors(std::vector<uint32_t> colors, bool colors_ranked_by_frequency) {
    std::vector<uint32_t> top_colors;
    EXPECT_TRUE(ScreenshotUntil([colors_ranked_by_frequency, colors, &top_colors](
                                    fuchsia::ui::scenic::ScreenshotData screenshot,
                                    bool status) mutable {
      if (!status)
        return false;

      std::map<uint32_t, size_t> histogram = Histogram(screenshot);

      if (colors.size() > histogram.size()) {
        return false;
      }

      std::multimap<size_t, uint32_t> inverse_histogram;
      for (const auto entry : histogram) {
        inverse_histogram.emplace(entry.second, entry.first);
      }
      EXPECT_TRUE(inverse_histogram.size() >= colors.size());

      top_colors.clear();
      size_t i = 0;
      for (auto it = inverse_histogram.rbegin();
           it != inverse_histogram.rend() && i < colors.size(); it++, i++) {
        top_colors.push_back(it->second);
      }

      EXPECT_EQ(colors.size(), top_colors.size());

      if (!colors_ranked_by_frequency) {
        // Sort both lists so that we are doing a set equality comparison.
        std::sort(top_colors.begin(), top_colors.end());
        std::sort(colors.begin(), colors.end());
      }

      return top_colors == colors;
    })) << "Top colors were: "
        << Colors{top_colors};
  }

 private:
  std::unique_ptr<sys::ComponentContext> context_;
  fuchsia::sys::ComponentControllerPtr runner_ctrl_;
  fuchsia::ui::scenic::ScenicPtr scenic_;
};

using FlutterPixelTest = PixelTest;

// Launches "flutter_checkerboard_app" using root_presenter and, indirectly,
// Scenic. The test app displays a checkerboard pattern. The test verifies that the
// two most frequent colors match the checkerboard colors.
TEST_F(FlutterPixelTest, Static) {
  std::vector<uint32_t> kExpectedTopTwoColors = {0xFF4dac26, 0xFFd01c8b};

  fuchsia::sys::ComponentControllerPtr controller;
  fuchsia::sys::LauncherPtr launcher;
  context()->svc()->Connect(launcher.NewRequest());

  auto info = scenic::LaunchComponentAndCreateView(
      launcher,
      "fuchsia-pkg://fuchsia.com/flutter_checkerboard_app#meta/"
      "flutter_checkerboard_app.cmx");

  info.controller.events().OnTerminated = [](int64_t code, fuchsia::sys::TerminationReason reason) {
    FAIL() << "Component terminated.";
  };

  // Present the view.
  scenic::EmbedderView embedder_view({
      .session_and_listener_request = scenic::CreateScenicSessionPtrAndListenerRequest(scenic()),
      .view_token = CreatePresentationViewToken(),
  });

  embedder_view.EmbedView(
      std::move(info),
      /*view_state_changed_callback=*/[this](fuchsia::ui::gfx::ViewState view_state) {
        EXPECT_TRUE(view_state.is_rendering);
        QuitLoop();
      });

  // Wait to get a signal that the view is being rendered.
  EXPECT_FALSE(RunLoopWithTimeout(kTestTimeout))
      << "Timed out waiting for a ViewStateChanged event.";

  // Take screenshots until we timeout, or we see the top two expected colors.
  ExpectTopColors(kExpectedTopTwoColors, /*colors_ranked_by_frequency=*/false);
}

}  // namespace
