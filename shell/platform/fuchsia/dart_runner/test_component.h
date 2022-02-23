// Copyright 2022 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_TEST_COMPONENT_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_TEST_COMPONENT_H_

#include <fuchsia/component/runner/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/sys/cpp/service_directory.h>

#include <memory>

#include "suite.h"

using ComponentController = fuchsia::component::runner::ComponentController;

struct TestComponentArgs {
  std::string legacy_url;
  zx::channel outgoing_dir;
  fuchsia::sys::EnvironmentPtr parent_env;
  std::shared_ptr<sys::ServiceDirectory> parent_env_svc;
  std::shared_ptr<sys::ServiceDirectory> test_component_svc;

  fidl::InterfaceRequest<ComponentController> request;
  async_dispatcher_t* dispatcher;
};

/// Implements component controller on behalf of the runner and also
/// stores/controls running test component.
class TestComponent final : public ComponentController {
  using DoneCallback = fit::function<void(TestComponent*)>;

 public:
  explicit TestComponent(TestComponentArgs args, DoneCallback done_callback);
  ~TestComponent() override;

  void Stop() override;

  void Kill() override;

 private:
  async_dispatcher_t* dispatcher_;
  fidl::Binding<ComponentController> binding_;

  /// For safe keeping while the component is running.
  std::unique_ptr<Suite> suite_;

  /// Exposes suite protocol on behalf of test component.
  std::unique_ptr<sys::ComponentContext> suite_context_;

  DoneCallback done_callback_;
};

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_TEST_COMPONENT_H_
