// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_UI_BASE_VIEW_VIEW_PROVIDER_SERVICE_H_
#define SRC_LIB_UI_BASE_VIEW_VIEW_PROVIDER_SERVICE_H_

#include <fuchsia/ui/app/cpp/fidl.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/fit/function.h>
#include <lib/sys/cpp/component_context.h>

#include <vector>

#include "src/lib/ui/base_view/base_view.h"

namespace scenic {

// A callback to create a view when given a context.
using ViewFactory = fit::function<std::unique_ptr<BaseView>(ViewContext context)>;

// Publishes a view provider as an outgoing service of the application.
// The views created by the view provider are owned by it and will be destroyed
// when the view provider itself is destroyed.
//
// This is only intended to be used for simple example programs.
class ViewProviderService : private fuchsia::ui::app::ViewProvider {
 public:
  ViewProviderService(sys::ComponentContext* component_context, fuchsia::ui::scenic::Scenic* scenic,
                      ViewFactory factory);
  ~ViewProviderService();

  // |fuchsia::ui::app::ViewProvider|
  void CreateView(zx::eventpair view_token,
                  fidl::InterfaceRequest<fuchsia::sys::ServiceProvider> incoming_services,
                  fidl::InterfaceHandle<fuchsia::sys::ServiceProvider> outgoing_services) override;

  // |fuchsia::ui::app::ViewProvider|
  void CreateViewWithViewRef(zx::eventpair view_token,
                             fuchsia::ui::views::ViewRefControl view_ref_control,
                             fuchsia::ui::views::ViewRef view_ref) override;

 private:
  ViewProviderService(const ViewProviderService&) = delete;
  ViewProviderService& operator=(const ViewProviderService&) = delete;

  sys::ComponentContext* const component_context_;
  fuchsia::ui::scenic::Scenic* const scenic_;
  const ViewFactory view_factory_;

  std::vector<std::unique_ptr<BaseView>> views_;
  fidl::BindingSet<fuchsia::ui::app::ViewProvider> bindings_;
};

}  // namespace scenic

#endif  // SRC_LIB_UI_BASE_VIEW_VIEW_PROVIDER_SERVICE_H_
