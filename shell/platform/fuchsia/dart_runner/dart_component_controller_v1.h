// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_V1_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_V1_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/async/cpp/wait.h>
#include "dart_component_controller.h"

namespace dart_runner {

class DartComponentControllerV1 : public DartComponentController,
                                  public fuchsia::sys::ComponentController {
 public:
  DartComponentControllerV1(
      fuchsia::sys::Package package,
      fuchsia::sys::StartupInfo startup_info,
      std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
      fidl::InterfaceRequest<fuchsia::sys::ComponentController> controller);
  ~DartComponentControllerV1() override;

 private:
  // |ComponentController|
  void Kill() override;
  void Detach() override;

  void SendReturnCode() override;
  fdio_ns_t* PrepareNamespace() override;

  int GetStdoutFileDescriptor() override;
  int GetStderrFileDescriptor() override;

  bool PrepareBuiltinLibraries() override;

  std::vector<std::string> GetArguments() override;

  int SetupFileDescriptor(fuchsia::sys::FileDescriptorPtr fd);

  fuchsia::sys::Package package_;
  fuchsia::sys::StartupInfo startup_info_;

  fidl::Binding<fuchsia::sys::ComponentController> binding_;

  // Disallow copy and assignment.
  DartComponentControllerV1(const DartComponentControllerV1&) = delete;
  DartComponentControllerV1& operator=(const DartComponentControllerV1&) =
      delete;
};

}  // namespace dart_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_V1_H_
