// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/interop/playground_test.h"

#include "impeller/toolkit/interop/impeller.hpp"

namespace IMPELLER_HPP_NAMESPACE {
ProcTable gGlobalProcTable;
}  // namespace IMPELLER_HPP_NAMESPACE

namespace impeller::interop::testing {

static void SetupImpellerHPPProcTableOnce() {
  static std::once_flag sOnceFlag;
  std::call_once(sOnceFlag, []() {
    std::map<std::string, void*> proc_map;
#define IMPELLER_HPP_PROC(name) \
  proc_map[#name] = reinterpret_cast<void*>(&name);
    IMPELLER_HPP_EACH_PROC(IMPELLER_HPP_PROC)
#undef IMPELLER_HPP_PROC
    hpp::gGlobalProcTable.Initialize(
        [&](auto name) { return proc_map.at(name); });
  });
}

PlaygroundTest::PlaygroundTest() {
  SetupImpellerHPPProcTableOnce();
}

PlaygroundTest::~PlaygroundTest() = default;

// |PlaygroundTest|
void PlaygroundTest::SetUp() {
  ::impeller::PlaygroundTest::SetUp();
}

// |PlaygroundTest|
void PlaygroundTest::TearDown() {
  ::impeller::PlaygroundTest::TearDown();
}

ScopedObject<Context> PlaygroundTest::CreateContext() const {
  switch (GetBackend()) {
    case PlaygroundBackend::kMetal:
      return Adopt<Context>(
          ImpellerContextCreateMetalNew(ImpellerGetVersion()));
    case PlaygroundBackend::kOpenGLES: {
      Playground::GLProcAddressResolver playground_gl_proc_address_callback =
          CreateGLProcAddressResolver();
      ImpellerProcAddressCallback gl_proc_address_callback =
          [](const char* proc_name, void* user_data) -> void* {
        return (*reinterpret_cast<Playground::GLProcAddressResolver*>(
            user_data))(proc_name);
      };
      return Adopt<Context>(ImpellerContextCreateOpenGLESNew(
          ImpellerGetVersion(), gl_proc_address_callback,
          &playground_gl_proc_address_callback));
    }
    case PlaygroundBackend::kVulkan:
      ImpellerContextVulkanSettings settings = {};
      struct UserData {
        Playground::VKProcAddressResolver resolver;
      } user_data;
      user_data.resolver = CreateVKProcAddressResolver();
      settings.user_data = &user_data;
      settings.enable_vulkan_validation = switches_.enable_vulkan_validation;
      settings.proc_address_callback = [](void* instance,         //
                                          const char* proc_name,  //
                                          void* user_data         //
                                          ) -> void* {
        auto resolver = reinterpret_cast<UserData*>(user_data)->resolver;
        if (resolver) {
          return resolver(instance, proc_name);
        } else {
          return nullptr;
        }
      };
      return Adopt<Context>(
          ImpellerContextCreateVulkanNew(ImpellerGetVersion(), &settings));
  }
  FML_UNREACHABLE();
}

bool PlaygroundTest::OpenPlaygroundHere(InteropPlaygroundCallback callback) {
  auto interop_context = GetInteropContext();
  if (!interop_context) {
    return false;
  }
  return Playground::OpenPlaygroundHere([&](RenderTarget& target) -> bool {
    return false;
    // auto impeller_surface = std::make_shared<impeller::Surface>(target);
    // auto surface = Create<Surface>(*interop_context.Get(), impeller_surface);
    // if (!surface) {
    //   VALIDATION_LOG << "Could not wrap test surface as an interop surface.";
    //   return false;
    // }
    // return callback(interop_context, surface);
  });
}

ScopedObject<Context> PlaygroundTest::GetInteropContext() {
  if (interop_context_) {
    return interop_context_;
  }
  auto context = CreateContext();
  if (!context) {
    return nullptr;
  }
  interop_context_ = std::move(context);
  return interop_context_;
}

}  // namespace impeller::interop::testing
