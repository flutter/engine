// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dart_component_controller_v2.h"

namespace dart_runner {

DartComponentControllerV2::DartComponentControllerV2(
    fuchsia::component::runner::ComponentStartInfo start_info,
    std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
    fidl::InterfaceRequest<fuchsia::component::runner::ComponentController>
        controller)
    : DartComponentController::DartComponentController(
          std::move(start_info.resolved_url()),
          runner_incoming_services),
      start_info_(std::move(start_info)),
      binding_(this) {
  auto name = GetComponentNameFromUrl(url_);
  data_path_ = "pkg/data/" + name;

  if (controller.is_valid()) {
    binding_.Bind(std::move(controller));
    binding_.set_error_handler([this](zx_status_t status) { Kill(); });
  }
}

DartComponentControllerV2::~DartComponentControllerV2() {}

void DartComponentControllerV2::Kill() {
  binding_.set_error_handler([](zx_status_t status) {});
  Shutdown();
}

void DartComponentControllerV2::Stop() {
  Kill();
}

void DartComponentControllerV2::SendReturnCode() {
  if (binding_.is_bound()) {
    binding_.Close(return_code_);
  }
}

fdio_ns_t* DartComponentControllerV2::PrepareNamespace() {
  fdio_ns_t* ns;
  zx_status_t status = fdio_ns_create(&ns);
  if (status != ZX_OK) {
    return nullptr;
  }

  if (!start_info_.has_ns()) {
    return nullptr;
  }

  dart_utils::RunnerTemp::SetupComponent(ns);

  for (auto& ns_entry : *start_info_.mutable_ns()) {
    if (!ns_entry.has_path() || !ns_entry.has_directory()) {
      continue;
    }

    if (ns_entry.path() == kTmpPath) {
      // /tmp is covered by the local memfs.
      continue;
    }

    auto dir = std::move(*ns_entry.mutable_directory());
    auto path = std::move(*ns_entry.mutable_path());

    zx_status_t status =
        fdio_ns_bind(ns, path.c_str(), dir.TakeChannel().release());
    if (status != ZX_OK) {
      FX_LOGF(ERROR, LOG_TAG, "Failed to bind %s to namespace: %s",
              path.c_str(), zx_status_get_string(status));
      return nullptr;
    }
  }

  return ns;
}

int DartComponentControllerV2::GetStdoutFileDescriptor() {
  
  return fileno(stdout);
}

int DartComponentControllerV2::GetStderrFileDescriptor() {
  
  return fileno(stderr);
}

bool DartComponentControllerV2::PrepareBuiltinLibraries() {
  auto dir = std::move(*start_info_.mutable_outgoing_dir());
  InitBuiltinLibrariesForIsolate(url_, namespace_, stdoutfd_, stderrfd_,
                                 nullptr /* environment */, dir.TakeChannel(),
                                 false /* service_isolate */);

  return true;
}

std::vector<std::string> DartComponentControllerV2::GetArguments() {
  return std::vector<std::string>();
}

}  // namespace dart_runner
