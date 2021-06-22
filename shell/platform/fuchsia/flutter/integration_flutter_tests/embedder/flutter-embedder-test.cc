// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ui/tests/integration_flutter_tests/embedder/flutter-embedder-test.h"

namespace flutter_embedder_test {

constexpr char kParentViewUrl[] = "fuchsia-pkg://fuchsia.com/parent-view#meta/parent-view.cmx";

constexpr scenic::Color kParentBackgroundColor = {0x00, 0x00, 0xFF, 0xFF};   // Blue
constexpr scenic::Color kParentTappedColor = {0x00, 0x00, 0x00, 0xFF};       // Black
constexpr scenic::Color kOverlayBackgroundColor = {0x00, 0xFF, 0x00, 0xFF};  // Green
constexpr scenic::Color kChildBackgroundColor = {0xFF, 0x00, 0xFF, 0xFF};    // Pink
constexpr scenic::Color kChildTappedColor = {0xFF, 0xFF, 0x00, 0xFF};        // Yellow

/// Defines a list of services that are injected into the test environment. Unlike the
/// injected-services in CMX which are injected per test package, these are injected per test and
/// result in a more hermetic test environment.
const std::vector<std::pair<const char*, const char*>> GetInjectedServices() {
  std::vector<std::pair<const char*, const char*>> injected_services = {{
      // clang-format off
    {
      "fuchsia.accessibility.semantics.SemanticsManager",
      "fuchsia-pkg://fuchsia.com/a11y-manager#meta/a11y-manager.cmx"
    },{
      "fuchsia.deprecatedtimezone.Timezone",
      "fuchsia-pkg://fuchsia.com/timezone#meta/timezone.cmx"
    },{
      "fuchsia.fonts.Provider",
      "fuchsia-pkg://fuchsia.com/fonts#meta/fonts.cmx"
    },{
      "fuchsia.hardware.display.Provider",
      "fuchsia-pkg://fuchsia.com/fake-hardware-display-controller-provider#meta/hdcp.cmx"
    },{
      "fuchsia.intl.PropertyProvider",
      "fuchsia-pkg://fuchsia.com/intl_property_manager#meta/intl_property_manager.cmx"
    },{
      "fuchsia.netstack.Netstack",
      "fuchsia-pkg://fuchsia.com/netstack#meta/netstack.cmx"
    },{
      "fuchsia.posix.socket.Provider",
      "fuchsia-pkg://fuchsia.com/netstack#meta/netstack.cmx"
    },{
      "fuchsia.tracing.provider.Registry",
      "fuchsia-pkg://fuchsia.com/trace_manager#meta/trace_manager.cmx"
    },{
      "fuchsia.ui.input.ImeService",
      "fuchsia-pkg://fuchsia.com/ime_service#meta/ime_service.cmx"
    },{
      "fuchsia.ui.input.ImeVisibilityService",
      "fuchsia-pkg://fuchsia.com/ime_service#meta/ime_service.cmx"
    },{
      "fuchsia.ui.scenic.Scenic",
      "fuchsia-pkg://fuchsia.com/scenic#meta/scenic.cmx"
    },{
      "fuchsia.ui.pointerinjector.Registry",
      "fuchsia-pkg://fuchsia.com/scenic#meta/scenic.cmx"
    },{
      "fuchsia.ui.policy.Presenter",
      "fuchsia-pkg://fuchsia.com/root_presenter#meta/root_presenter.cmx"
    },{
      "fuchsia.ui.input.InputDeviceRegistry",
      "fuchsia-pkg://fuchsia.com/root_presenter#meta/root_presenter.cmx"
    },
      // clang-format on
  }};
  return injected_services;
}

TEST_F(FlutterEmbedderTests, BasicLegacyEmbedding) {
  RunAppWithArgs(kParentViewUrl, {"--no-usePlatformView"});

  // Take screenshot until we see the child-view's embedded color.
  ASSERT_TRUE(
      TakeScreenshotUntil(kChildBackgroundColor, [](std::map<scenic::Color, size_t> histogram) {
        // Expect parent and child background colors, with parent color > child color.
        EXPECT_GT(histogram[kParentBackgroundColor], 0u);
        EXPECT_GT(histogram[kChildBackgroundColor], 0u);
        EXPECT_GT(histogram[kParentBackgroundColor], histogram[kChildBackgroundColor]);
      }));
}

TEST_F(FlutterEmbedderTests, HittestLegacyEmbedding) {
  RunAppWithArgs(kParentViewUrl, {"--no-usePlatformView"});

  // Take screenshot until we see the child-view's embedded color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildBackgroundColor));

  // Tap the center of child view.
  InjectInput();

  // Take screenshot until we see the child-view's tapped color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildTappedColor, [](std::map<scenic::Color, size_t> histogram) {
    // Expect parent and child background colors, with parent color > child color.
    EXPECT_GT(histogram[kParentBackgroundColor], 0u);
    EXPECT_EQ(histogram[kChildBackgroundColor], 0u);
    EXPECT_GT(histogram[kChildTappedColor], 0u);
    EXPECT_GT(histogram[kParentBackgroundColor], histogram[kChildTappedColor]);
  }));
}

TEST_F(FlutterEmbedderTests, HittestDisabledLegacyEmbedding) {
  RunAppWithArgs(kParentViewUrl, {"--no-hitTestable", "--no-usePlatformView"});

  // Take screenshots until we see the child-view's embedded color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildBackgroundColor));

  // Tap the center of child view. Since it's not hit-testable, the tap should go to the parent.
  InjectInput();

  // The parent-view should change color.
  ASSERT_TRUE(
      TakeScreenshotUntil(kParentTappedColor, [](std::map<scenic::Color, size_t> histogram) {
        // Expect parent and child background colors, with parent color > child color.
        EXPECT_EQ(histogram[kParentBackgroundColor], 0u);
        EXPECT_GT(histogram[kParentTappedColor], 0u);
        EXPECT_GT(histogram[kChildBackgroundColor], 0u);
        EXPECT_EQ(histogram[kChildTappedColor], 0u);
        EXPECT_GT(histogram[kParentTappedColor], histogram[kChildBackgroundColor]);
      }));
}

TEST_F(FlutterEmbedderTests, BasicLegacyEmbeddingWithOverlay) {
  RunAppWithArgs(kParentViewUrl, {"--showOverlay", "--no-usePlatformView"});

  // Take screenshot until we see the child-view's embedded color.
  ASSERT_TRUE(
      TakeScreenshotUntil(kChildBackgroundColor, [](std::map<scenic::Color, size_t> histogram) {
        // Expect parent, overlay and child background colors.
        // With parent color > child color and overlay color > child color.
        EXPECT_GT(histogram[kParentBackgroundColor], 0u);
        EXPECT_GT(histogram[kOverlayBackgroundColor], 0u);
        EXPECT_GT(histogram[kChildBackgroundColor], 0u);
        EXPECT_GT(histogram[kParentBackgroundColor], histogram[kChildBackgroundColor]);
        EXPECT_GT(histogram[kOverlayBackgroundColor], histogram[kChildBackgroundColor]);
      }));
}

TEST_F(FlutterEmbedderTests, HittestLegacyEmbeddingWithOverlay) {
  RunAppWithArgs(kParentViewUrl, {"--showOverlay", "--no-usePlatformView"});

  // Take screenshot until we see the child-view's embedded color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildBackgroundColor));

  // Tap the center of child view.
  InjectInput();

  // Take screenshot until we see the child-view's tapped color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildTappedColor, [](std::map<scenic::Color, size_t> histogram) {
    // Expect parent, overlay and child background colors.
    // With parent color > child color and overlay color > child color.
    EXPECT_GT(histogram[kParentBackgroundColor], 0u);
    EXPECT_GT(histogram[kOverlayBackgroundColor], 0u);
    EXPECT_EQ(histogram[kChildBackgroundColor], 0u);
    EXPECT_GT(histogram[kChildTappedColor], 0u);
    EXPECT_GT(histogram[kParentBackgroundColor], histogram[kChildTappedColor]);
    EXPECT_GT(histogram[kOverlayBackgroundColor], histogram[kChildTappedColor]);
  }));
}

TEST_F(FlutterEmbedderTests, DISABLED_BasicPlatformViewEmbedding) {
  RunAppWithArgs(kParentViewUrl);

  // Take screenshot until we see the child-view's embedded color.
  ASSERT_TRUE(
      TakeScreenshotUntil(kChildBackgroundColor, [](std::map<scenic::Color, size_t> histogram) {
        // Expect parent and child background colors, with parent color > child color.
        EXPECT_GT(histogram[kParentBackgroundColor], 0u);
        EXPECT_GT(histogram[kChildBackgroundColor], 0u);
        EXPECT_GT(histogram[kParentBackgroundColor], histogram[kChildBackgroundColor]);
      }));
}

TEST_F(FlutterEmbedderTests, DISABLED_HittestPlatformViewEmbedding) {
  RunAppWithArgs(kParentViewUrl);

  // Take screenshot until we see the child-view's embedded color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildBackgroundColor));

  // Tap the center of child view.
  InjectInput();

  // Take screenshot until we see the child-view's tapped color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildTappedColor, [](std::map<scenic::Color, size_t> histogram) {
    // Expect parent and child background colors, with parent color > child color.
    EXPECT_GT(histogram[kParentBackgroundColor], 0u);
    EXPECT_EQ(histogram[kChildBackgroundColor], 0u);
    EXPECT_GT(histogram[kChildTappedColor], 0u);
    EXPECT_GT(histogram[kParentBackgroundColor], histogram[kChildTappedColor]);
  }));
}

TEST_F(FlutterEmbedderTests, DISABLED_HittestDisabledPlatformViewEmbedding) {
  RunAppWithArgs(kParentViewUrl, {"--no-hitTestable"});

  // Take screenshots until we see the child-view's embedded color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildBackgroundColor));

  // Tap the center of child view. Since it's not hit-testable, the tap should go to the parent.
  InjectInput();

  // The parent-view should change color.
  ASSERT_TRUE(
      TakeScreenshotUntil(kParentTappedColor, [](std::map<scenic::Color, size_t> histogram) {
        // Expect parent and child background colors, with parent color > child color.
        EXPECT_EQ(histogram[kParentBackgroundColor], 0u);
        EXPECT_GT(histogram[kParentTappedColor], 0u);
        EXPECT_GT(histogram[kChildBackgroundColor], 0u);
        EXPECT_EQ(histogram[kChildTappedColor], 0u);
        EXPECT_GT(histogram[kParentTappedColor], histogram[kChildBackgroundColor]);
      }));
}

TEST_F(FlutterEmbedderTests, DISABLED_BasicPlatformViewEmbeddingWithOverlay) {
  RunAppWithArgs(kParentViewUrl, {"--showOverlay"});

  // Take screenshot until we see the child-view's embedded color.
  ASSERT_TRUE(
      TakeScreenshotUntil(kChildBackgroundColor, [](std::map<scenic::Color, size_t> histogram) {
        // Expect parent, overlay and child background colors.
        // With parent color > child color and overlay color > child color.
        EXPECT_GT(histogram[kParentBackgroundColor], 0u);
        EXPECT_GT(histogram[kOverlayBackgroundColor], 0u);
        EXPECT_GT(histogram[kChildBackgroundColor], 0u);
        EXPECT_GT(histogram[kParentBackgroundColor], histogram[kChildBackgroundColor]);
        EXPECT_GT(histogram[kOverlayBackgroundColor], histogram[kChildBackgroundColor]);
      }));
}

TEST_F(FlutterEmbedderTests, DISABLED_HittestPlatformViewEmbeddingWithOverlay) {
  RunAppWithArgs(kParentViewUrl, {"--showOverlay"});

  // Take screenshot until we see the child-view's embedded color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildBackgroundColor));

  // Tap the center of child view.
  InjectInput();

  // Take screenshot until we see the child-view's tapped color.
  ASSERT_TRUE(TakeScreenshotUntil(kChildTappedColor, [](std::map<scenic::Color, size_t> histogram) {
    // Expect parent, overlay and child background colors.
    // With parent color > child color and overlay color > child color.
    // EXPECT_EQ((std::map<scenic::Color, size_t>){}, histogram) << "Unexpected colors";
    EXPECT_GT(histogram[kParentBackgroundColor], 0u);
    EXPECT_GT(histogram[kOverlayBackgroundColor], 0u);
    EXPECT_EQ(histogram[kChildBackgroundColor], 0u);
    EXPECT_GT(histogram[kChildTappedColor], 0u);
    EXPECT_GT(histogram[kParentBackgroundColor], histogram[kChildTappedColor]);
    EXPECT_GT(histogram[kOverlayBackgroundColor], histogram[kChildTappedColor]);
  }));
}

}  // namespace flutter_embedder_test
