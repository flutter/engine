// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/async/cpp/wait.h>
#include <lib/fdio/namespace.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/zx/time.h>
#include <lib/zx/timer.h>
#include <zircon/types.h>

#include <memory>
#include <string>
#include <vector>

#include "flutter/shell/platform/fuchsia/utils/mapped_resource.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace dart_runner {

class DartComponentController : public fuchsia::sys::ComponentController {
 public:
  DartComponentController(
      fuchsia::sys::Package package,
      fuchsia::sys::StartupInfo startup_info,
      std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
      fidl::InterfaceRequest<fuchsia::sys::ComponentController> controller);
  DartComponentController(const DartComponentController&) = delete;
  DartComponentController(DartComponentController&&) = delete;
  ~DartComponentController() override;

  DartComponentController& operator=(const DartComponentController&) = delete;
  DartComponentController& operator=(DartComponentController&&) = delete;

  bool Setup();
  void Run();
  bool Main();
  void SendReturnCode();

 private:
  // |ComponentController|
  void Kill() override;
  void Detach() override;

  bool SetupNamespace();
  bool SetupFromKernel();
  bool SetupFromAppSnapshot();
  int SetupFileDescriptor(fuchsia::sys::FileDescriptorPtr fd);

  bool CreateIsolate(const uint8_t* isolate_snapshot_data,
                     const uint8_t* isolate_snapshot_instructions);

  // Idle notification.
  void MessageEpilogue(Dart_Handle result);
  void OnIdleTimer(async_dispatcher_t* dispatcher,
                   async::WaitBase* wait,
                   zx_status_t status,
                   const zx_packet_signal* signal);

  std::string label_;
  std::string url_;
  std::shared_ptr<sys::ServiceDirectory> runner_incoming_services_;
  std::string data_path_;
  std::unique_ptr<sys::ComponentContext> context_;

  fx::ElfSnapshot elf_snapshot_;                      // AOT snapshot
  fx::MappedResource isolate_snapshot_data_;          // JIT snapshot
  fx::MappedResource isolate_snapshot_instructions_;  // JIT snapshot
  std::vector<fx::MappedResource> kernel_peices_;

  zx::time idle_start_{0};
  zx::timer idle_timer_;
  async::WaitMethod<DartComponentController,
                    &DartComponentController::OnIdleTimer>
      idle_wait_{this};

  async::Loop loop_;  // Must come before binding_.
  fuchsia::sys::Package package_;
  fuchsia::sys::StartupInfo startup_info_;
  fidl::Binding<fuchsia::sys::ComponentController> binding_;

  fdio_ns_t* namespace_ = nullptr;
  int stdoutfd_ = -1;
  int stderrfd_ = -1;

  Dart_Isolate isolate_;
  int32_t return_code_ = 0;
};

}  // namespace dart_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_H_
