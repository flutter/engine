// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RUNNER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RUNNER_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/trace-engine/context.h>
#include <lib/trace/observer.h>

#include <memory>
#include <vector>

#include "flutter/shell/platform/fuchsia/flutter/component.h"
#include "flutter/shell/platform/fuchsia/flutter/thread.h"
#if !defined(DART_PRODUCT)
#include "flutter/shell/platform/fuchsia/runtime/dart/utils/vmservice_object.h"
#endif

namespace flutter_runner {

// This embedder component publishes the |fuchsia::sys::Runner| service and
// runs Flutter components on demand.
//
// Each component is assigned to its own dedicated thread.
class Runner final : public fuchsia::sys::Runner {
 public:
  explicit Runner(async::Loop* loop);
  ~Runner();
  Runner(const Runner&) = delete;
  Runner& operator=(const Runner&) = delete;

 private:
  // Represents a running |Component| along with its host thread.
  struct ActiveComponent {
    std::unique_ptr<Component> component;
    std::unique_ptr<Thread> thread;
  };

  // |fuchsia::sys::Runner|
  void StartComponent(fuchsia::sys::Package package,
                      fuchsia::sys::StartupInfo startup_info,
                      fidl::InterfaceRequest<fuchsia::sys::ComponentController>
                          controller) override;

  void OnComponentTerminate(const Component* component);

#if !defined(DART_PRODUCT)
  void StartTraceObserver();
  void StopTraceObserver();
#endif  // !defined(DART_PRODUCT)

  fidl::BindingSet<fuchsia::sys::Runner> runner_bindings_;

  std::vector<ActiveComponent> active_components_;

  std::unique_ptr<sys::ComponentContext> context_;

  async::Loop* loop_;

#if !defined(DART_PRODUCT)
  trace::TraceObserver trace_observer_;
  trace_prolonged_context_t* prolonged_context_;

  // The connection between the Dart VM service and The Hub.
  std::unique_ptr<dart_utils::VMServiceObject> vmservice_object_;
#endif  // !defined(DART_PRODUCT)
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RUNNER_H_
