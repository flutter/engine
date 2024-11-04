// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/golden_tests/golden_playground_test.h"

#include "flutter/impeller/playground/playground_impl.h"
#include "flutter/third_party/abseil-cpp/absl/base/no_destructor.h"
#include "flutter/impeller/golden_tests/vulkan_screenshotter.h"

#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

namespace impeller {

namespace {
std::unique_ptr<PlaygroundImpl> MakeVulkanPlayground(bool enable_validations) {
  FML_CHECK(::glfwInit() == GLFW_TRUE);
  PlaygroundSwitches playground_switches;
  playground_switches.enable_vulkan_validation = enable_validations;
  return PlaygroundImpl::Create(PlaygroundBackend::kVulkan,
                                playground_switches);
}

// Returns a static instance to a playground that can be used across tests.
const std::unique_ptr<PlaygroundImpl>& GetSharedVulkanPlayground(
    bool enable_validations) {
  if (enable_validations) {
    static absl::NoDestructor<std::unique_ptr<PlaygroundImpl>>
        vulkan_validation_playground(
            MakeVulkanPlayground(/*enable_validations=*/true));
    // TODO(142237): This can be removed when the thread local storage is
    // removed.
    static fml::ScopedCleanupClosure context_cleanup(
        [&] { (*vulkan_validation_playground)->GetContext()->Shutdown(); });
    return *vulkan_validation_playground;
  } else {
    static absl::NoDestructor<std::unique_ptr<PlaygroundImpl>>
        vulkan_playground(MakeVulkanPlayground(/*enable_validations=*/false));
    // TODO(142237): This can be removed when the thread local storage is
    // removed.
    static fml::ScopedCleanupClosure context_cleanup(
        [&] { (*vulkan_playground)->GetContext()->Shutdown(); });
    return *vulkan_playground;
  }
}
}  // namespace

struct GoldenPlaygroundTest::GoldenPlaygroundTestImpl {
  std::unique_ptr<PlaygroundImpl> test_vulkan_playground;
  std::unique_ptr<PlaygroundImpl> test_opengl_playground;
  std::unique_ptr<testing::Screenshotter> screenshotter;
  ISize window_size = ISize{1024, 768};
};

GoldenPlaygroundTest::GoldenPlaygroundTest() = default;

GoldenPlaygroundTest::~GoldenPlaygroundTest() = default;

void GoldenPlaygroundTest::SetTypographerContext(
    std::shared_ptr<TypographerContext> typographer_context) {
  typographer_context_ = std::move(typographer_context);
};

void GoldenPlaygroundTest::TearDown() {}

void GoldenPlaygroundTest::SetUp() {
  GTEST_SKIP() << "GoldenPlaygroundTest doesn't support this backend type.";
}

PlaygroundBackend GoldenPlaygroundTest::GetBackend() const {
  return GetParam();
}

bool GoldenPlaygroundTest::OpenPlaygroundHere(
    const AiksDlPlaygroundCallback& callback) {
  return false;
}

bool GoldenPlaygroundTest::OpenPlaygroundHere(
    const sk_sp<flutter::DisplayList>& list) {
  return false;
}

std::shared_ptr<Texture> GoldenPlaygroundTest::CreateTextureForFixture(
    const char* fixture_name,
    bool enable_mipmapping) const {
  return nullptr;
}

sk_sp<flutter::DlImage> GoldenPlaygroundTest::CreateDlImageForFixture(
    const char* fixture_name,
    bool enable_mipmapping) const {
  return nullptr;
}

RuntimeStage::Map GoldenPlaygroundTest::OpenAssetAsRuntimeStage(
    const char* asset_name) const {
  return {};
}

std::shared_ptr<Context> GoldenPlaygroundTest::GetContext() const {
  return nullptr;
}

Point GoldenPlaygroundTest::GetContentScale() const {
  return Point();
}

Scalar GoldenPlaygroundTest::GetSecondsElapsed() const {
  return Scalar();
}

ISize GoldenPlaygroundTest::GetWindowSize() const {
  return ISize();
}

void GoldenPlaygroundTest::SetWindowSize(ISize size) {}

fml::Status GoldenPlaygroundTest::SetCapabilities(
    const std::shared_ptr<Capabilities>& capabilities) {
  return fml::Status(
      fml::StatusCode::kUnimplemented,
      "GoldenPlaygroundTest-Stub doesn't support SetCapabilities.");
}

std::unique_ptr<testing::Screenshot> GoldenPlaygroundTest::MakeScreenshot(
    const sk_sp<flutter::DisplayList>& list) {
  return nullptr;
}

bool GoldenPlaygroundTest::ImGuiBegin(const char* name,
                                      bool* p_open,
                                      ImGuiWindowFlags flags) {
  return false;
}

std::shared_ptr<Context> GoldenPlaygroundTest::MakeContext() const {
  if (GetParam() == PlaygroundBackend::kMetal) {
    FML_CHECK(false) << "not supported.";
  } else if (GetParam() == PlaygroundBackend::kVulkan) {
    bool enable_vulkan_validations = true;
    FML_CHECK(!pimpl_->test_vulkan_playground)
        << "We don't support creating multiple contexts for one test";
    pimpl_->test_vulkan_playground =
        MakeVulkanPlayground(enable_vulkan_validations);
    pimpl_->screenshotter = std::make_unique<testing::VulkanScreenshotter>(
        pimpl_->test_vulkan_playground);
    return pimpl_->test_vulkan_playground->GetContext();
  } else {
    /// On OpenGL we create a context for each test.
    return GetContext();
  }
}

}  // namespace impeller
