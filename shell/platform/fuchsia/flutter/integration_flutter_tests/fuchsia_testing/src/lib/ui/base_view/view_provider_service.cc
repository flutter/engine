// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/ui/base_view/view_provider_service.h"

#include <lib/syslog/cpp/macros.h>
#include <lib/ui/scenic/cpp/view_ref_pair.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>

#include <algorithm>

namespace scenic {

ViewProviderService::ViewProviderService(sys::ComponentContext* component_context,
                                         fuchsia::ui::scenic::Scenic* scenic,
                                         ViewFactory view_factory)
    : component_context_(component_context),
      scenic_(scenic),
      view_factory_(std::move(view_factory)) {
  FX_DCHECK(scenic_);
  FX_DCHECK(view_factory_);
  FX_DCHECK(component_context_);

  component_context_->outgoing()->AddPublicService<fuchsia::ui::app::ViewProvider>(
      bindings_.GetHandler(this));
}

ViewProviderService::~ViewProviderService() {
  component_context_->outgoing()->RemovePublicService<fuchsia::ui::app::ViewProvider>();
}

void ViewProviderService::CreateView(
    zx::eventpair view_token,
    fidl::InterfaceRequest<fuchsia::sys::ServiceProvider> incoming_services,
    fidl::InterfaceHandle<fuchsia::sys::ServiceProvider> outgoing_services) {
  auto [view_ref_control, view_ref] = scenic::ViewRefPair::New();
  CreateViewWithViewRef(std::move(view_token), std::move(view_ref_control), std::move(view_ref));
}

void ViewProviderService::CreateViewWithViewRef(zx::eventpair view_token,
                                                fuchsia::ui::views::ViewRefControl view_ref_control,
                                                fuchsia::ui::views::ViewRef view_ref) {
  scenic::ViewRefPair view_ref_pair{
      .control_ref = std::move(view_ref_control),
      .view_ref = std::move(view_ref),
  };
  ViewContext context = {
      .session_and_listener_request = CreateScenicSessionPtrAndListenerRequest(scenic_),
      .view_token = scenic::ToViewToken(std::move(view_token)),
      .view_ref_pair = std::move(view_ref_pair),
      .component_context = component_context_,
  };

  if (auto base_view = view_factory_(std::move(context))) {
    base_view->SetReleaseHandler([this, base_view = base_view.get()](zx_status_t status) {
      auto it = std::find_if(
          views_.begin(), views_.end(),
          [base_view](const std::unique_ptr<BaseView>& other) { return other.get() == base_view; });
      FX_DCHECK(it != views_.end());
      views_.erase(it);
    });
    views_.push_back(std::move(base_view));
  }
}

}  // namespace scenic
