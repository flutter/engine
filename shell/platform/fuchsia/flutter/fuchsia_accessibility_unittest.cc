// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "topaz/shell/platform/fuchsia/flutter/fuchsia_accessibility.h"

#include <gtest/gtest.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/gtest/real_loop_fixture.h>
#include <lib/sys/cpp/testing/service_directory_provider.h>

namespace flutter_runner_a11y_test {
using fuchsia::accessibility::semantics::SemanticsManager;
using FuchsiaAccessibilityTests = gtest::RealLoopFixture;

class MockSemanticsManager : public SemanticsManager {
 public:
  MockSemanticsManager() = default;
  ~MockSemanticsManager() = default;

  // |fuchsia::accessibility::semantics::SemanticsManager|:
  void RegisterView(
      fuchsia::ui::views::ViewRef view_ref,
      fidl::InterfaceHandle<
          fuchsia::accessibility::semantics::SemanticActionListener>
          handle,
      fidl::InterfaceRequest<fuchsia::accessibility::semantics::SemanticTree>
          semantic_tree) override {
    has_view_ref_ = true;
  }

  fidl::InterfaceRequestHandler<SemanticsManager> GetHandler(
      async_dispatcher_t* dispatcher) {
    return bindings_.GetHandler(this, dispatcher);
  }

  bool RegisterViewCalled() { return has_view_ref_; }

 private:
  bool has_view_ref_ = false;
  fidl::BindingSet<SemanticsManager> bindings_;
};

TEST_F(FuchsiaAccessibilityTests, RegisterViewRef) {
  MockSemanticsManager semantics_manager;
  sys::testing::ServiceDirectoryProvider services_provider(dispatcher());
  services_provider.AddService(semantics_manager.GetHandler(dispatcher()),
                               SemanticsManager::Name_);
  zx::eventpair a, b;
  zx::eventpair::create(/* flags */ 0u, &a, &b);
  auto view_ref = fuchsia::ui::views::ViewRef({
      .reference = std::move(a),
  });
  auto fuchsia_accessibility = flutter_runner::FuchsiaAccessibility::Create(
      services_provider.service_directory(), std::move(view_ref));

  RunLoopUntilIdle();
  EXPECT_TRUE(semantics_manager.RegisterViewCalled());
}

}  // namespace flutter_runner_a11y_test
