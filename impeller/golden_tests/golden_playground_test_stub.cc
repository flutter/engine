// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/golden_tests/golden_playground_test.h"

#include <filesystem>

#include "flutter/impeller/golden_tests/golden_digest.h"
#include "flutter/impeller/golden_tests/vulkan_screenshotter.h"
#include "flutter/impeller/playground/playground_impl.h"
#include "flutter/third_party/abseil-cpp/absl/base/no_destructor.h"
#include "impeller/display_list/dl_dispatcher.h"
#include "impeller/display_list/dl_image_impeller.h"

#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

namespace impeller {

namespace {
std::string GetTestName() {
  std::string suite_name =
      ::testing::UnitTest::GetInstance()->current_test_suite()->name();
  std::string test_name =
      ::testing::UnitTest::GetInstance()->current_test_info()->name();
  std::stringstream ss;
  ss << "impeller_" << suite_name << "_" << test_name;
  std::string result = ss.str();
  // Make sure there are no slashes in the test name.
  std::replace(result.begin(), result.end(), '/', '_');
  return result;
}

std::string GetGoldenFilename(const std::string& postfix) {
  return GetTestName() + postfix + ".png";
}
}  // namespace

struct GoldenPlaygroundTest::GoldenPlaygroundTestImpl {
  std::unique_ptr<PlaygroundImpl> test_vulkan_playground;
  std::unique_ptr<PlaygroundImpl> test_opengl_playground;
  std::unique_ptr<testing::Screenshotter> screenshotter;
  ISize window_size = ISize{1024, 768};
};

GoldenPlaygroundTest::GoldenPlaygroundTest()
    : pimpl_(new GoldenPlaygroundTest::GoldenPlaygroundTestImpl()) {}

GoldenPlaygroundTest::~GoldenPlaygroundTest() = default;

void GoldenPlaygroundTest::SetTypographerContext(
    std::shared_ptr<TypographerContext> typographer_context) {
  typographer_context_ = std::move(typographer_context);
};

void GoldenPlaygroundTest::TearDown() {}

void GoldenPlaygroundTest::SetUp() {
  std::filesystem::path testing_assets_path =
      flutter::testing::GetTestingAssetsPath();
  std::filesystem::path target_path = testing_assets_path.parent_path()
                                          .parent_path()
                                          .parent_path()
                                          .parent_path();
  std::filesystem::path icd_path = target_path / "vk_swiftshader_icd.json";
  setenv("VK_ICD_FILENAMES", icd_path.c_str(), 1);

  std::string test_name = GetTestName();
  bool enable_wide_gamut = test_name.find("WideGamut_") != std::string::npos;
  switch (GetParam()) {
    case PlaygroundBackend::kMetal:
      GTEST_SKIP() << "Platform doesn't support metal.";
      break;
    case PlaygroundBackend::kVulkan: {
      GTEST_SKIP() << "Platform doesn't support metal.";
      break;
    }
    case PlaygroundBackend::kOpenGLES: {
      if (enable_wide_gamut) {
        GTEST_SKIP() << "OpenGLES doesn't support wide gamut golden tests.";
      }
      FML_CHECK(::glfwInit() == GLFW_TRUE);
      PlaygroundSwitches playground_switches;
      playground_switches.use_angle = true;
      pimpl_->test_opengl_playground = PlaygroundImpl::Create(
          PlaygroundBackend::kOpenGLES, playground_switches);
      pimpl_->screenshotter = std::make_unique<testing::VulkanScreenshotter>(
          pimpl_->test_opengl_playground);
      break;
    }
  }

  testing::GoldenDigest::Instance()->AddDimension(
      "gpu_string", GetContext()->DescribeGpuModel());
}

PlaygroundBackend GoldenPlaygroundTest::GetBackend() const {
  return GetParam();
}

bool GoldenPlaygroundTest::OpenPlaygroundHere(
    const AiksDlPlaygroundCallback& callback) {
  AiksContext renderer(GetContext(), typographer_context_);

  std::unique_ptr<testing::Screenshot> screenshot;
  for (int i = 0; i < 2; ++i) {
    auto display_list = callback();
    auto texture =
        DisplayListToTexture(display_list, pimpl_->window_size, renderer);
    screenshot = pimpl_->screenshotter->MakeScreenshot(renderer, texture);
  }
  return SaveScreenshot(std::move(screenshot));
}

bool GoldenPlaygroundTest::OpenPlaygroundHere(
    const sk_sp<flutter::DisplayList>& list) {
  return OpenPlaygroundHere([&list]() { return list; });
}

std::shared_ptr<Texture> GoldenPlaygroundTest::CreateTextureForFixture(
    const char* fixture_name,
    bool enable_mipmapping) const {
  std::shared_ptr<fml::Mapping> mapping =
      flutter::testing::OpenFixtureAsMapping(fixture_name);
  auto result = Playground::CreateTextureForMapping(GetContext(), mapping,
                                                    enable_mipmapping);
  if (result) {
    result->SetLabel(fixture_name);
  }
  return result;
}

sk_sp<flutter::DlImage> GoldenPlaygroundTest::CreateDlImageForFixture(
    const char* fixture_name,
    bool enable_mipmapping) const {
  std::shared_ptr<Texture> texture =
      CreateTextureForFixture(fixture_name, enable_mipmapping);
  return DlImageImpeller::Make(texture);
}

RuntimeStage::Map GoldenPlaygroundTest::OpenAssetAsRuntimeStage(
    const char* asset_name) const {
  const std::shared_ptr<fml::Mapping> fixture =
      flutter::testing::OpenFixtureAsMapping(asset_name);
  if (!fixture || fixture->GetSize() == 0) {
    return {};
  }
  return RuntimeStage::DecodeRuntimeStages(fixture);
}

std::shared_ptr<Context> GoldenPlaygroundTest::GetContext() const {
  return pimpl_->screenshotter->GetPlayground().GetContext();
}

Point GoldenPlaygroundTest::GetContentScale() const {
  return pimpl_->screenshotter->GetPlayground().GetContentScale();
}

Scalar GoldenPlaygroundTest::GetSecondsElapsed() const {
  return Scalar();
}

ISize GoldenPlaygroundTest::GetWindowSize() const {
  return pimpl_->window_size;
}

void GoldenPlaygroundTest::SetWindowSize(ISize size) {
  pimpl_->window_size = size;
}

fml::Status GoldenPlaygroundTest::SetCapabilities(
    const std::shared_ptr<Capabilities>& capabilities) {
  return pimpl_->screenshotter->GetPlayground().SetCapabilities(capabilities);
}

std::unique_ptr<testing::Screenshot> GoldenPlaygroundTest::MakeScreenshot(
    const sk_sp<flutter::DisplayList>& list) {
  AiksContext renderer(GetContext(), typographer_context_);

  return pimpl_->screenshotter->MakeScreenshot(
      renderer, DisplayListToTexture(list, pimpl_->window_size, renderer));
}

bool GoldenPlaygroundTest::ImGuiBegin(const char* name,
                                      bool* p_open,
                                      ImGuiWindowFlags flags) {
  return false;
}

std::shared_ptr<Context> GoldenPlaygroundTest::MakeContext() const {
  if (GetParam() == PlaygroundBackend::kMetal) {
    FML_CHECK(false) << "not supported.";
    return nullptr;
  } else if (GetParam() == PlaygroundBackend::kVulkan) {
    FML_CHECK(false) << "not supported.";
    return nullptr;
  } else {
    /// On OpenGL we create a context for each test.
    return GetContext();
  }
}

bool GoldenPlaygroundTest::SaveScreenshot(
    std::unique_ptr<testing::Screenshot> screenshot,
    const std::string& postfix) {
  if (!screenshot || !screenshot->GetBytes()) {
    FML_LOG(ERROR) << "Failed to collect screenshot for test " << GetTestName();
    return false;
  }
  std::string test_name = GetTestName();
  std::string filename = GetGoldenFilename(postfix);
  testing::GoldenDigest::Instance()->AddImage(
      test_name, filename, screenshot->GetWidth(), screenshot->GetHeight());
  if (!screenshot->WriteToPNG(
          testing::WorkingDirectory::Instance()->GetFilenamePath(filename))) {
    FML_LOG(ERROR) << "Failed to write screenshot to " << filename;
    return false;
  }
  return true;
}

}  // namespace impeller
