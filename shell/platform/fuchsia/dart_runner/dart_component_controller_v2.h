// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_V2_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_V2_H_

#include <fuchsia/component/runner/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/async/cpp/wait.h>
#include "dart_component_controller.h"

namespace dart_runner {

class DartComponentControllerV2
    : public DartComponentController,
      public fuchsia::component::runner::ComponentController {
 public:
  DartComponentControllerV2(
      fuchsia::component::runner::ComponentStartInfo start_info,
      std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
      fidl::InterfaceRequest<fuchsia::component::runner::ComponentController>
          controller);
  ~DartComponentControllerV2() override;

 private:
  // |fuchsia::component::runner::ComponentController|
  void Kill() override;
  void Stop() override;

  void SendReturnCode() override;
  fdio_ns_t* PrepareNamespace() override;

  int GetStdoutFileDescriptor() override;
  int GetStderrFileDescriptor() override;

  bool PrepareBuiltinLibraries() override;

  std::vector<std::string> GetArguments() override;

  fuchsia::component::runner::ComponentStartInfo start_info_;

  fidl::Binding<fuchsia::component::runner::ComponentController> binding_;

  // Disallow copy and assignment.
  DartComponentControllerV2(const DartComponentControllerV2&) = delete;
  DartComponentControllerV2& operator=(const DartComponentControllerV2&) =
      delete;
};

}  // namespace dart_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_V2_H_
