// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dart_component_controller_v1.h"

namespace dart_runner {

DartComponentControllerV1::DartComponentControllerV1(
    fuchsia::sys::Package package,
    fuchsia::sys::StartupInfo startup_info,
    std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
    fidl::InterfaceRequest<fuchsia::sys::ComponentController> controller)
    : DartComponentController::DartComponentController(
          std::move(package.resolved_url),
          runner_incoming_services),
      package_(std::move(package)),
      startup_info_(std::move(startup_info)),
      binding_(this) {
  for (size_t i = 0; i < startup_info_.program_metadata->size(); ++i) {
    auto pg = startup_info_.program_metadata->at(i);
    if (pg.key.compare(kDataKey) == 0) {
      data_path_ = "pkg/" + pg.value;
    }
  }

  if (data_path_.empty()) {
    FX_LOGF(ERROR, LOG_TAG, "Could not find a /pkg/data directory for %s",
            url_.c_str());
    return;
  }
  if (controller.is_valid()) {
    binding_.Bind(std::move(controller));
    binding_.set_error_handler([this](zx_status_t status) { Kill(); });
  }
}

DartComponentControllerV1::~DartComponentControllerV1() {}

void DartComponentControllerV1::Kill() {
  Shutdown();
}

void DartComponentControllerV1::Detach() {
  binding_.set_error_handler([](zx_status_t status) {});
}

void DartComponentControllerV1::SendReturnCode() {
  binding_.events().OnTerminated(return_code_,
                                 fuchsia::sys::TerminationReason::EXITED);
}

fdio_ns_t* DartComponentControllerV1::PrepareNamespace() {
  fdio_ns_t* ns = nullptr;
  zx_status_t status = fdio_ns_create(&ns);
  if (status != ZX_OK) {
    return nullptr;
  }

  fuchsia::sys::FlatNamespace* flat = &startup_info_.flat_namespace;

  dart_utils::RunnerTemp::SetupComponent(ns);

  for (size_t i = 0; i < flat->paths.size(); ++i) {
    if (flat->paths.at(i) == kTmpPath) {
      // /tmp is covered by the local memfs.
      continue;
    }

    zx::channel dir;
    if (flat->paths.at(i) == kServiceRootPath) {
      // clone /svc so component_context can still use it below
      dir = zx::channel(fdio_service_clone(flat->directories.at(i).get()));
    } else {
      dir = std::move(flat->directories.at(i));
    }

    zx_handle_t dir_handle = dir.release();
    const char* path = flat->paths.at(i).data();
    zx_status_t status = fdio_ns_bind(ns, path, dir_handle);
    if (status != ZX_OK) {
      FX_LOGF(ERROR, LOG_TAG, "Failed to bind %s to namespace: %s",
              flat->paths.at(i).c_str(), zx_status_get_string(status));
      zx_handle_close(dir_handle);
      return nullptr;
    }
  }

  return ns;
}

int DartComponentControllerV1::GetStdoutFileDescriptor() {
  return SetupFileDescriptor(std::move(startup_info_.launch_info.out));
}

int DartComponentControllerV1::GetStderrFileDescriptor() {
  return SetupFileDescriptor(std::move(startup_info_.launch_info.err));
}

int DartComponentControllerV1::SetupFileDescriptor(
    fuchsia::sys::FileDescriptorPtr fd) {
  if (!fd) {
    return -1;
  }
  // fd->handle1 and fd->handle2 are no longer used.
  int outfd = -1;
  zx_status_t status = fdio_fd_create(fd->handle0.release(), &outfd);
  if (status != ZX_OK) {
    FX_LOGF(ERROR, LOG_TAG, "Failed to extract output fd: %s",
            zx_status_get_string(status));
    return -1;
  }
  return outfd;
}

bool DartComponentControllerV1::PrepareBuiltinLibraries() {
  auto directory_request =
      std::move(startup_info_.launch_info.directory_request);

  auto* flat = &startup_info_.flat_namespace;
  std::unique_ptr<sys::ServiceDirectory> svc;
  for (size_t i = 0; i < flat->paths.size(); ++i) {
    zx::channel dir;
    if (flat->paths.at(i) == kServiceRootPath) {
      svc = std::make_unique<sys::ServiceDirectory>(
          std::move(flat->directories.at(i)));
      break;
    }
  }
  if (!svc) {
    return false;
  }

  fidl::InterfaceHandle<fuchsia::sys::Environment> environment;
  svc->Connect(environment.NewRequest());

  InitBuiltinLibrariesForIsolate(
      url_, namespace_, stdoutfd_, stderrfd_, std::move(environment),
      std::move(directory_request), false /* service_isolate */);

  return true;
}

std::vector<std::string> DartComponentControllerV1::GetArguments() {
  return startup_info_.launch_info.arguments.value_or(
      std::vector<std::string>());
}

}  // namespace dart_runner
