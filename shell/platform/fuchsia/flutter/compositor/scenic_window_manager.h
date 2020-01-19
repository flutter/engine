// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSDstyle license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPOSITOR_SCENIC_WINDOW_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPOSITOR_SCENIC_WINDOW_MANAGER_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <fuchsia/ui/app/cpp/fidl.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/vfs/cpp/composed_service_dir.h>
#include <lib/zx/eventpair.h>

#include <memory>
#include <vector>

#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_window.h"

namespace flutter_runner {

// This embedder component is responsible for the lifecycle and management of
// connections with the Scenic compositor on Fuchsia.  Each connection is
// represented as a |ScenicWindow| object.
//
// The |ScenicWindowManager| creates and destroys these |ScenicWindow|s in
// response to |View| requests from other applications.  It also destroys
// a given |ScenicWindow| when an error occurs on the window's |ScenicSession|.
//
// It is the embedder's responsibility to bind each individual |Window| to a
// running Flutter engine instance in response to the |WindowCreatedCallback|.
// The |WindowDestroyedCallback| allows the embedder to perform cleanup when a
// |Window| is destroyed due to a system lifecycle event.
class ScenicWindowManager final : public fuchsia::ui::app::ViewProvider {
 public:
  using WindowCreatedCallback = std::function<void(ScenicWindow*)>;
  using WindowDestroyedCallback = std::function<void(ScenicWindow*)>;

  ScenicWindowManager(const std::string& debug_label,
                      WindowCreatedCallback window_created_callback,
                      WindowDestroyedCallback window_destroyed_callback,
                      std::shared_ptr<sys::ServiceDirectory> incoming_services);
  ~ScenicWindowManager();
  ScenicWindowManager(const ScenicWindowManager&) = delete;
  ScenicWindowManager& operator=(const ScenicWindowManager&) = delete;

  void RegisterServices(vfs::ComposedServiceDir* dir);

  void DestroyWindow(ScenicWindow* handle);

 private:
  // |fuchsia::ui::app::ViewProvider|
  void CreateView(
      zx::eventpair view_token,
      fidl::InterfaceRequest<fuchsia::sys::ServiceProvider> incoming_services,
      fidl::InterfaceHandle<fuchsia::sys::ServiceProvider> outgoing_services)
      override;

  const std::string& debug_label_;

  fidl::BindingSet<fuchsia::ui::app::ViewProvider> view_provider_bindings_;

  std::vector<std::unique_ptr<ScenicWindow>> windows_;

  std::shared_ptr<sys::ServiceDirectory> incoming_services_;

  WindowCreatedCallback window_created_callback_;
  WindowDestroyedCallback window_destroyed_callback_;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPOSITOR_SCENIC_WINDOW_MANAGER_H_
