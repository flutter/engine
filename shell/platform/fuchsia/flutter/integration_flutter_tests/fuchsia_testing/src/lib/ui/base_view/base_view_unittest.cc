// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/ui/base_view/base_view.h"

#include <fuchsia/ui/gfx/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl_test_base.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/fostr/fidl/fuchsia/ui/gfx/formatting.h>
#include <lib/gtest/test_loop_fixture.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/sys/cpp/testing/component_context_provider.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>

#include <gmock/gmock.h>

using ::testing::_;

namespace scenic {

class MockSession : public fuchsia::ui::scenic::testing::Session_TestBase {
 public:
  MockSession() : binding_(this) {}

  void NotImplemented_(const std::string& name) final {}

  void Bind(fidl::InterfaceRequest<::fuchsia::ui::scenic::Session> request,
            ::fuchsia::ui::scenic::SessionListenerPtr listener) {
    binding_.Bind(std::move(request));
    listener_ = std::move(listener);
  }

  MOCK_METHOD4(Present, void(uint64_t presentation_time, std::vector<zx::event> acquire_fences,
                             std::vector<zx::event> release_fences, PresentCallback callback));

 private:
  fidl::Binding<fuchsia::ui::scenic::Session> binding_;
  fuchsia::ui::scenic::SessionListenerPtr listener_;
};

class FakeScenic : public fuchsia::ui::scenic::testing::Scenic_TestBase {
 public:
  void NotImplemented_(const std::string& name) final {}

  fidl::InterfaceRequestHandler<fuchsia::ui::scenic::Scenic> GetRequestHandler() {
    return bindings_.GetHandler(this);
  }

  void CreateSession(
      fidl::InterfaceRequest<fuchsia::ui::scenic::Session> session,
      fidl::InterfaceHandle<fuchsia::ui::scenic::SessionListener> listener) override {
    mock_session_.Bind(std::move(session), listener.Bind());
  }

  MockSession* mock_session() { return &mock_session_; }

 private:
  fidl::BindingSet<fuchsia::ui::scenic::Scenic> bindings_;
  MockSession mock_session_;
};

class BaseViewImpl : public BaseView {
 public:
  BaseViewImpl(ViewContext context, const std::string& debug_name)
      : BaseView(std::move(context), debug_name) {}

  void DoPresentScene() { PresentScene(); }

 private:
  // |fuchsia::ui::scenic::SessionListener|
  void OnScenicError(std::string error) override {}
};

class BaseViewTest : public gtest::TestLoopFixture {
 protected:
  void SetUp() override {
    provider.service_directory_provider()->AddService(fake_scenic_.GetRequestHandler());

    auto [view_token, view_holder_token] = scenic::ViewTokenPair::New();
    auto component_context = provider.TakeContext();

    scenic::ViewContext view_context = {
        .session_and_listener_request =
            scenic::CreateScenicSessionPtrAndListenerRequest(&fake_scenic_),
        .view_token = std::move(view_token),
        .component_context = component_context.get(),
    };
    view_holder_token_ = std::move(view_holder_token);
    component_context_ = std::move(component_context);
    base_view_ = std::make_unique<BaseViewImpl>(std::move(view_context), std::string());
  }

  std::unique_ptr<BaseViewImpl> base_view_;
  FakeScenic fake_scenic_;

 private:
  sys::testing::ComponentContextProvider provider;
  fuchsia::ui::views::ViewHolderToken view_holder_token_;
  std::unique_ptr<sys::ComponentContext> component_context_;
};

TEST_F(BaseViewTest, HandlesMultiplePresentCalls) {
  // Expect Present() calls in initialization.
  EXPECT_CALL(*fake_scenic_.mock_session(), Present(_, _, _, _))
      .WillRepeatedly([](uint64_t presentation_time, std::vector<zx::event> acquire_fences,
                         std::vector<zx::event> release_fences, Session::PresentCallback callback) {
        callback(fuchsia::images::PresentationInfo());
      });
  RunLoopUntilIdle();
  ASSERT_TRUE(testing::Mock::VerifyAndClear(fake_scenic_.mock_session()));

  // Queue 3 calls but expect only 1 to be sent to Session.
  EXPECT_CALL(*fake_scenic_.mock_session(), Present(_, _, _, _)).Times(1);
  base_view_->DoPresentScene();
  base_view_->DoPresentScene();
  base_view_->DoPresentScene();
  RunLoopUntilIdle();
}

TEST_F(BaseViewTest, InvalidateCallbackInvoked) {
  // Expect Present() calls in initialization.
  EXPECT_CALL(*fake_scenic_.mock_session(), Present(_, _, _, _))
      .WillRepeatedly([](uint64_t presentation_time, std::vector<zx::event> acquire_fences,
                         std::vector<zx::event> release_fences, Session::PresentCallback callback) {
        callback(fuchsia::images::PresentationInfo());
      });
  RunLoopUntilIdle();
  ASSERT_TRUE(testing::Mock::VerifyAndClear(fake_scenic_.mock_session()));

  size_t num_present_callbacks = 0;
  EXPECT_CALL(*fake_scenic_.mock_session(), Present(_, _, _, _))
      .Times(2)
      .WillRepeatedly([&num_present_callbacks](uint64_t presentation_time,
                                               std::vector<zx::event> acquire_fences,
                                               std::vector<zx::event> release_fences,
                                               Session::PresentCallback callback) {
        num_present_callbacks++;
        callback(fuchsia::images::PresentationInfo());
      });
  base_view_->InvalidateScene();
  base_view_->DoPresentScene();

  // Now that there's a PresentScene pending, expect that InvalidateScene() will trigger
  // a second Present, and won't receive it's callback until the second present.
  bool callback_invoked;
  base_view_->InvalidateScene([&num_present_callbacks, &callback_invoked](auto) {
    EXPECT_EQ(num_present_callbacks, 2u);
    callback_invoked = true;
  });
  RunLoopUntilIdle();
  EXPECT_TRUE(callback_invoked);
}

}  // namespace scenic
