// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fuchsia_accessibility.h"

#include <src/lib/fxl/logging.h>
#include <zircon/status.h>

#include "fuchsia/accessibility/semantics/cpp/fidl.h"

namespace flutter_runner {
namespace {
class FuchsiaAccessibilityImpl
    : public FuchsiaAccessibility,
      public fuchsia::accessibility::semantics::SemanticActionListener {
 public:
  FuchsiaAccessibilityImpl(std::shared_ptr<sys::ServiceDirectory> services,
                           fuchsia::ui::views::ViewRef view_ref)
      : binding_(this), view_ref_(std::move(view_ref)) {
    services->Connect(
        fuchsia::accessibility::semantics::SemanticsManager::Name_,
        manager_.NewRequest().TakeChannel());
    manager_.set_error_handler([this](zx_status_t status) {
      FXL_LOG(ERROR)
          << "Flutter cannot connect to SemanticsManager with status: "
          << zx_status_get_string(status) << ".";
    });
    fidl::InterfaceHandle<
        fuchsia::accessibility::semantics::SemanticActionListener>
        listener_handle;
    binding_.Bind(listener_handle.NewRequest());
    manager_->RegisterView(std::move(view_ref_), std::move(listener_handle),
                           tree_ptr_.NewRequest());
  }

  ~FuchsiaAccessibilityImpl() override = default;

  void UpdateSemanticNodes(
      std::vector<fuchsia::accessibility::semantics::Node> nodes) override {}
  void DeleteSemanticNodes(std::vector<uint32_t> node_ids) override {}
  void Commit() override {}

 private:
  // |fuchsia::accessibility::semantics::SemanticActionListener|
  void OnAccessibilityActionRequested(
      uint32_t node_id, fuchsia::accessibility::semantics::Action action,
      fuchsia::accessibility::semantics::SemanticActionListener::
          OnAccessibilityActionRequestedCallback callback) override {}

  fuchsia::accessibility::semantics::SemanticsManagerPtr manager_;
  fuchsia::accessibility::semantics::SemanticTreePtr tree_ptr_;
  fidl::Binding<fuchsia::accessibility::semantics::SemanticActionListener>
      binding_;
  fuchsia::ui::views::ViewRef view_ref_;

  FXL_DISALLOW_COPY_AND_ASSIGN(FuchsiaAccessibilityImpl);
};

}  // namespace

// static
std::unique_ptr<FuchsiaAccessibility> FuchsiaAccessibility::Create(
    std::shared_ptr<sys::ServiceDirectory> services,
    fuchsia::ui::views::ViewRef view_ref) {
  return std::make_unique<FuchsiaAccessibilityImpl>(std::move(services),
                                                    std::move(view_ref));
}

}  // namespace flutter_runner
