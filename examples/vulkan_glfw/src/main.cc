// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"  // GLFW header from the GLFW repository's include path.

#include "embedder.h"  // Flutter's Embedder ABI.

static const bool g_enable_validation_layers = true;
static const size_t kInitialWindowWidth = 800;
static const size_t kInitialWindowHeight = 600;

static_assert(FLUTTER_ENGINE_VERSION == 1,
              "This Flutter Embedder was authored against the stable Flutter "
              "API at version 1. There has been a serious breakage in the "
              "API. Please read the ChangeLog and take appropriate action "
              "before updating this assertion");

/// Global struct for holding the Window+Vulkan state.
struct {
  GLFWwindow* window;
  VkInstance instance;
  VkSurfaceKHR surface;
} g_state;

void GLFW_ErrorCallback(int error, const char* description) {
  std::cout << "GLFW Error: (" << error << ") " << description << std::endl;
}

void printUsage() {
  std::cout << "usage: flutter_glfw <path to project> <path to icudtl.dat>"
            << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    printUsage();
    return 1;
  }

  std::string project_path = argv[1];
  std::string icudtl_path = argv[2];

  /// --------------------------------------------------------------------------
  /// Create a GLFW Window.
  /// --------------------------------------------------------------------------

  glfwSetErrorCallback(GLFW_ErrorCallback);

  int result = glfwInit();
  assert(result == GLFW_TRUE);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  g_state.window = glfwCreateWindow(kInitialWindowWidth, kInitialWindowHeight,
                                    "Flutter", nullptr, nullptr);
  assert(g_state.window != nullptr);

  /// --------------------------------------------------------------------------
  /// Create a Vulkan instance.
  /// --------------------------------------------------------------------------
  {
    uint32_t instance_extensions_count;
    const char** instance_extensions =
        glfwGetRequiredInstanceExtensions(&instance_extensions_count);

    std::cout << "GLFW requires " << instance_extensions_count
              << " Vulkan extensions:\n";
    for (int i = 0; i < instance_extensions_count; i++) {
      std::cout << " - " << instance_extensions[i] << "\n";
    }

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Flutter",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_MAKE_VERSION(1, 1, 0),
    };
    VkInstanceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.flags = 0;
    info.pApplicationInfo = &app_info;
    info.enabledExtensionCount = instance_extensions_count;
    info.ppEnabledExtensionNames = instance_extensions;
    if (g_enable_validation_layers) {
      uint32_t layer_count;
      vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
      std::vector<VkLayerProperties> available_layers(layer_count);
      vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

      const char* layer = "VK_LAYER_KHRONOS_validation";
      for (const auto& l : available_layers) {
        if (strcmp(l.layerName, layer) == 0) {
          info.enabledLayerCount = 1;
          info.ppEnabledLayerNames = &layer;
          break;
        }
      }
    }

    assert(vkCreateInstance(&info, nullptr, &g_state.instance) == VK_SUCCESS);
  }

  /// --------------------------------------------------------------------------
  /// Create surface.
  /// --------------------------------------------------------------------------
  assert(glfwCreateWindowSurface(g_state.instance, g_state.window, NULL,
                                 &g_state.surface) == VK_SUCCESS);

  std::cout << "Success.\n";

  while (!glfwWindowShouldClose(g_state.window)) {
    glfwPollEvents();
  }

  glfwDestroyWindow(g_state.window);
  glfwTerminate();
  return 0;
}
