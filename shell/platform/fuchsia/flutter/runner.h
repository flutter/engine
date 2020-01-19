// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RUNNER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RUNNER_H_

#include <fuchsia/intl/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/sys/cpp/component_context.h>

#include <memory>
#include <vector>

#include "flutter/shell/platform/fuchsia/flutter/component.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer.h"

#if !defined(DART_PRODUCT)
#include <lib/trace-engine/context.h>
#include <lib/trace/observer.h>
#endif  // !defined(DART_PRODUCT)

namespace flutter_runner {

// This object publishes the |fuchsia::sys::Runner| service and runs Flutter
// components on demand.
//
// This object is non-copyable, and non-movable.
class Runner final : public fuchsia::sys::Runner {
 public:
  Runner(Renderer::FactoryCallback renderer_factory, async::Loop* loop);
  Runner(const Runner&) = delete;
  Runner(Runner&&) = delete;
  ~Runner();

  Runner& operator=(const Runner&) = delete;
  Runner& operator=(Runner&&) = delete;

 private:
  // |fuchsia::sys::Runner|
  void StartComponent(fuchsia::sys::Package package,
                      fuchsia::sys::StartupInfo startup_info,
                      fidl::InterfaceRequest<fuchsia::sys::ComponentController>
                          component_controller) override;

  void OnComponentTerminate(const Component* component);

#if !defined(DART_PRODUCT)
  void StartTraceObserver();
  void StopTraceObserver();
#endif  // !defined(DART_PRODUCT)

  Renderer::FactoryCallback renderer_factory_;

  std::vector<std::unique_ptr<Component, Component::Deleter>> components_;
  std::unique_ptr<sys::ComponentContext> runner_context_;

  fidl::BindingSet<fuchsia::sys::Runner> runner_bindings_;
  fuchsia::intl::PropertyProviderPtr intl_property_provider_;

  async::Loop* loop_;

#if !defined(DART_PRODUCT)
  trace::TraceObserver trace_observer_;
  trace_prolonged_context_t* prolonged_context_;
#endif  // !defined(DART_PRODUCT)
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RUNNER_H_
