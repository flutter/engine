// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_ISOLATE_CONFIGURATOR_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_ISOLATE_CONFIGURATOR_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>

#include "flutter/shell/platform/fuchsia/flutter/unique_fdio_ns.h"

namespace flutter_runner {

// This embedder component contains all the information necessary to configure
// a new root isolate. This is a single use item; a new one must be created for
// each root isolate.
//
// The lifetime of this object must extend past that of the root isolate.
class IsolateConfigurator final {
 public:
  IsolateConfigurator(
      UniqueFDIONS fdio_ns,
      fidl::InterfaceHandle<fuchsia::sys::Environment> environment,
      zx::channel directory_request,
      zx::eventpair view_ref);
  ~IsolateConfigurator();
  IsolateConfigurator(const IsolateConfigurator&) = delete;
  IsolateConfigurator& operator=(const IsolateConfigurator&) = delete;
  IsolateConfigurator(IsolateConfigurator&& other) {
    *this = std::move(other);
  }
  IsolateConfigurator& operator=(IsolateConfigurator&& other) {
    if (this != &other) {
      fdio_ns_ = std::move(other.fdio_ns_);
      environment_ = std::move(other.environment_);
      directory_request_ = std::move(other.directory_request_);
      view_ref_ = std::move(other.view_ref_);
      used_ = std::move(other.used_);
    }
    return *this;
  }

  // Can be used only once and only on the UI thread with the newly created
  // isolate already current.
  bool ConfigureCurrentIsolate();

 private:
  void BindFuchsia();
  void BindZircon();
  void BindDartIO();

  UniqueFDIONS fdio_ns_;
  fidl::InterfaceHandle<fuchsia::sys::Environment> environment_;
  zx::channel directory_request_;
  zx::eventpair view_ref_;
  bool used_ = false;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_ISOLATE_CONFIGURATOR_H_
