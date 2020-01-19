// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_window_manager.h"

#include <fuchsia/ui/gfx/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>
#include <lib/vfs/cpp/service.h>

#include "flutter/fml/logging.h"

namespace flutter_runner {

ScenicWindowManager::ScenicWindowManager(const std::string& debug_label,
                                         WindowCreatedCallback window_created_callback,
                                         WindowDestroyedCallback window_destroyed_callback,
                                         std::shared_ptr<sys::ServiceDirectory> incoming_services)
  : debug_label_(debug_label),
    incoming_services_(incoming_services),
    window_created_callback_(std::move(window_created_callback)),
    window_destroyed_callback_(std::move(window_destroyed_callback)) {}

ScenicWindowManager::~ScenicWindowManager() = default;

void ScenicWindowManager::RegisterServices(vfs::ComposedServiceDir* dir) {
  dir->AddService(
      fuchsia::ui::app::ViewProvider::Name_,
      std::make_unique<vfs::Service>(
          [this](zx::channel channel, async_dispatcher_t* dispatcher) {
            auto view_provider_request = fidl::InterfaceRequest<fuchsia::ui::app::ViewProvider>(std::move(channel));
            view_provider_bindings_.AddBinding(this, std::move(view_provider_request));
          }));
}

void ScenicWindowManager::DestroyWindow(ScenicWindow* window) {
  // Find the window's position in the vector.
  auto window_iter = std::find_if(windows_.begin(), windows_.end(),[window](const auto& unique_window) {
    return unique_window.get() == window;
  });
  if (window_iter == windows_.end()) {
    FML_LOG(INFO) << "Tried to destroy a Scenic window that did not exist.";
    return;
  }

  // "Swap'n'pop" to remove the window.
  if (windows_.size() > 1) {
      std::iter_swap(window_iter, windows_.end() - 1);
  }
  windows_.pop_back();
}

void ScenicWindowManager::CreateView(
    zx::eventpair view_token,
    fidl::InterfaceRequest<fuchsia::sys::ServiceProvider> incoming_services,
    fidl::InterfaceHandle<fuchsia::sys::ServiceProvider> outgoing_services) {
  auto& window_iter = windows_.emplace_back(std::make_unique<ScenicWindow>(debug_label_, [this](ScenicWindow* window) {
    // Destroy the |ScenicWindow|, then inform this object's container about it.
    DestroyWindow(window);
    window_destroyed_callback_(window);
  }, scenic::ToViewToken(std::move(view_token)), incoming_services_));

  // Inform this object's container about the |ScenicWindow|'s existence.
  window_created_callback_(window_iter.get());
}

}  // namespace flutter_runner
