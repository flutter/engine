// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/ui/base_view/view_provider_component.h"

#include <lib/syslog/cpp/macros.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>

using fuchsia::ui::views::ViewToken;

namespace scenic {

ViewProviderComponent::ViewProviderComponent(ViewFactory factory, async::Loop* loop,
                                             sys::ComponentContext* component_context)
    : component_context_(component_context
                             ? std::unique_ptr<sys::ComponentContext>(component_context)
                             : sys::ComponentContext::CreateAndServeOutgoingDirectory()),
      scenic_(component_context_->svc()->Connect<fuchsia::ui::scenic::Scenic>()),
      service_(component_context_.get(), scenic_.get(), factory.share()) {
  // Register the |View| service.
  component_context_->outgoing()->AddPublicService<fuchsia::ui::views::View>(
      [this, component_context = component_context_.get(), factory = std::move(factory)](
          fidl::InterfaceRequest<fuchsia::ui::views::View> request) mutable {
        view_impl_ = std::make_unique<ViewImpl>(factory.share(), std::move(request), scenic_.get(),
                                                component_context);
        view_impl_->SetErrorHandler([this] { view_impl_ = nullptr; });
      });

  if (component_context) {
    // We are only responsible for cleaning up the context if we created it
    // ourselves.  In this case we are "borrowing" an existing context that was
    // provided to us, so we shouldn't retain a unique_ptr to it.
    component_context_.release();
  }

  scenic_.set_error_handler([loop](zx_status_t status) {
    FX_LOGS(INFO) << "Lost connection to Scenic.";
    loop->Quit();
  });
}

ViewProviderComponent::ViewImpl::ViewImpl(ViewFactory factory,
                                          fidl::InterfaceRequest<View> view_request,
                                          fuchsia::ui::scenic::Scenic* scenic,
                                          sys::ComponentContext* component_context)
    : factory_(std::move(factory)),
      scenic_(scenic),
      component_context_(component_context),
      binding_(this, std::move(view_request)) {}

void ViewProviderComponent::ViewImpl::Present(fuchsia::ui::views::ViewToken view_token) {
  if (view_) {
    // This should only be called once.
    FX_LOGS(ERROR) << "Present() can only be called once";
    OnError();
    return;
  }

  ViewContext context = {
      .session_and_listener_request = CreateScenicSessionPtrAndListenerRequest(scenic_),
      .view_token = std::move(view_token),
      .component_context = component_context_,
  };
  view_ = factory_(std::move(context));
}

void ViewProviderComponent::ViewImpl::SetErrorHandler(fit::closure error_handler) {
  error_handler_ = std::move(error_handler);
}

void ViewProviderComponent::ViewImpl::OnError(zx_status_t epitaph_value) {
  binding_.Close(epitaph_value);
  error_handler_();
}

}  // namespace scenic
