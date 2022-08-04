// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/playground/backend/vulkan/playground_impl_vk.h"

#include "impeller/renderer/backend/vulkan/vk.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/mapping.h"
#include "impeller/entity/vk/entity_shaders_vk.h"
#include "impeller/fixtures/vk/fixtures_shaders_vk.h"
#include "impeller/playground/imgui/vk/imgui_shaders_vk.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"
#include "impeller/renderer/backend/vulkan/surface_vk.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"

namespace impeller {

static std::vector<std::shared_ptr<fml::Mapping>>
ShaderLibraryMappingsForPlayground() {
  return {
      std::make_shared<fml::NonOwnedMapping>(impeller_entity_shaders_vk_data,
                                             impeller_entity_shaders_vk_length),
      std::make_shared<fml::NonOwnedMapping>(
          impeller_fixtures_shaders_vk_data,
          impeller_fixtures_shaders_vk_length),
      std::make_shared<fml::NonOwnedMapping>(impeller_imgui_shaders_vk_data,
                                             impeller_imgui_shaders_vk_length),

  };
}

void PlaygroundImplVK::DestroyWindowHandle(WindowHandle handle) {
  if (!handle) {
    return;
  }
  ::glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(handle));
}

PlaygroundImplVK::PlaygroundImplVK()
    : handle_(nullptr, &DestroyWindowHandle),
      // TODO (kaushikiska): remove "1"
      concurrent_loop_(fml::ConcurrentMessageLoop::Create(1)) {
  if (!::glfwVulkanSupported()) {
    VALIDATION_LOG << "Attempted to initialize a Vulkan playground on a system "
                      "that does not support Vulkan.";
    return;
  }

  ::glfwDefaultWindowHints();
  ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  ::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  // TODO (kaushikiska): consider making this resizable.
  auto window = ::glfwCreateWindow(800, 600, "Test vkWindow", nullptr, nullptr);
  if (!window) {
    VALIDATION_LOG << "Unable to create glfw window";
    return;
  }

  handle_.reset(window);

  auto context = ContextVK::Create(reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                                       &::glfwGetInstanceProcAddress),    //
                                   ShaderLibraryMappingsForPlayground(),  //
                                   nullptr,                               //
                                   concurrent_loop_->GetTaskRunner(),     //
                                   "Playground Library"                   //
  );

  if (!context || !context->IsValid()) {
    VALIDATION_LOG << "Could not create Vulkan context in the playground.";
    return;
  }

  context_ = context;

  SetupSwapchain();
}

PlaygroundImplVK::~PlaygroundImplVK() = default;

// |PlaygroundImpl|
std::shared_ptr<Context> PlaygroundImplVK::GetContext() const {
  return context_;
}

// |PlaygroundImpl|
PlaygroundImpl::WindowHandle PlaygroundImplVK::GetWindowHandle() const {
  return handle_.get();
}

ContextVK* PlaygroundImplVK::GetContextVK() const {
  return reinterpret_cast<ContextVK*>(context_.get());
}

void PlaygroundImplVK::SetupSwapchain() {
  ContextVK* context_vk = GetContextVK();

  auto window = reinterpret_cast<GLFWwindow*>(handle_.get());
  ::glfwCreateWindowSurface(context_vk->GetInstance(), window, nullptr,
                            &surface_);

  swapchain_ = context_vk->CreateSwapchain(surface_);
}

// |PlaygroundImpl|
std::unique_ptr<Surface> PlaygroundImplVK::AcquireSurfaceFrame(
    std::shared_ptr<Context> context) {
  return std::make_shared<SurfaceVK>()
}

}  // namespace impeller
