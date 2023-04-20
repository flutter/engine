// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/direct_manipulation.h"

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/windows/testing/mock_window_binding_handler_delegate.h"
#include "gtest/gtest.h"

using testing::_;

namespace flutter {
namespace testing {

class MockIDirectManipulationViewport : public IDirectManipulationViewport {
 public:
  MockIDirectManipulationViewport() {}

  MOCK_METHOD(ULONG, AddRef, (), (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(ULONG, Release, (), (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              QueryInterface,
              (REFIID, void**),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT, Abandon, (), (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              ActivateConfiguration,
              (DIRECTMANIPULATION_CONFIGURATION),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              AddConfiguration,
              (DIRECTMANIPULATION_CONFIGURATION),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              AddContent,
              (IDirectManipulationContent*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              AddEventHandler,
              (HWND, IDirectManipulationViewportEventHandler*, DWORD*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT, Disable, (), (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT, Enable, (), (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetPrimaryContent,
              (REFIID, void**),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetStatus,
              (DIRECTMANIPULATION_STATUS*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetTag,
              (REFIID, void**, UINT32*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetViewportRect,
              (RECT*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              ReleaseAllContacts,
              (),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              ReleaseContact,
              (UINT32),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              RemoveConfiguration,
              (DIRECTMANIPULATION_CONFIGURATION),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              RemoveContent,
              (IDirectManipulationContent*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              RemoveEventHandler,
              (DWORD),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetChaining,
              (DIRECTMANIPULATION_MOTION_TYPES),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetContact,
              (UINT32),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetInputMode,
              (DIRECTMANIPULATION_INPUT_MODE),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetManualGesture,
              (DIRECTMANIPULATION_GESTURE_CONFIGURATION),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetTag,
              (IUnknown*, UINT32),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetUpdateMode,
              (DIRECTMANIPULATION_INPUT_MODE),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetViewportOptions,
              (DIRECTMANIPULATION_VIEWPORT_OPTIONS),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetViewportRect,
              (const RECT*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetViewportTransform,
              (const float*, DWORD),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT, Stop, (), (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SyncDisplayTransform,
              (const float*, DWORD),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              ZoomToRect,
              (const float, const float, const float, const float, BOOL),
              (override, Calltype(STDMETHODCALLTYPE)));

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(MockIDirectManipulationViewport);
};

class MockIDirectManipulationContent : public IDirectManipulationContent {
 public:
  MockIDirectManipulationContent() {}

  MOCK_METHOD(ULONG, AddRef, (), (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(ULONG, Release, (), (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              QueryInterface,
              (REFIID, void**),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetContentRect,
              (RECT*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetContentTransform,
              (float*, DWORD),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetOutputTransform,
              (float*, DWORD),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetTag,
              (REFIID, void**, UINT32*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              GetViewport,
              (REFIID, void**),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetContentRect,
              (const RECT*),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SetTag,
              (IUnknown*, UINT32),
              (override, Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(HRESULT,
              SyncContentTransform,
              (const float*, DWORD),
              (override, Calltype(STDMETHODCALLTYPE)));

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(MockIDirectManipulationContent);
};

TEST(DirectManipulationTest, TestGesture) {
  MockIDirectManipulationContent content;
  MockWindowBindingHandlerDelegate delegate;
  MockIDirectManipulationViewport viewport;
  const float scale = 1.5;
  const float pan_x = 32.0;
  const float pan_y = 16.0;
  const int DISPLAY_WIDTH = 800;
  const int DISPLAY_HEIGHT = 600;
  auto owner = std::make_unique<DirectManipulationOwner>(nullptr);
  owner->SetBindingHandlerDelegate(&delegate);
  auto handler =
      fml::MakeRefCounted<DirectManipulationEventHandler>(owner.get());
  int32_t device_id = (int32_t) reinterpret_cast<int64_t>(handler.get());
  EXPECT_CALL(viewport, GetPrimaryContent(_, _))
      .WillOnce(::testing::Invoke([&content](REFIID in, void** out) {
        *out = &content;
        return S_OK;
      }))
      .RetiresOnSaturation();
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([scale](float* transform, DWORD size) {
        transform[0] = 1.0f;
        transform[4] = 0.0;
        transform[5] = 0.0;
        return S_OK;
      }))
      .RetiresOnSaturation();
  EXPECT_CALL(delegate, OnPointerPanZoomStart(device_id));
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_RUNNING,
                                   DIRECTMANIPULATION_READY);
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke(
          [scale, pan_x, pan_y](float* transform, DWORD size) {
            transform[0] = scale;
            transform[4] = pan_x;
            transform[5] = pan_y;
            return S_OK;
          }));
  EXPECT_CALL(delegate,
              OnPointerPanZoomUpdate(device_id, pan_x, pan_y, scale, 0));
  handler->OnContentUpdated((IDirectManipulationViewport*)&viewport,
                            (IDirectManipulationContent*)&content);
  EXPECT_CALL(delegate, OnPointerPanZoomEnd(device_id));
  EXPECT_CALL(viewport, GetViewportRect(_))
      .WillOnce(::testing::Invoke([DISPLAY_WIDTH, DISPLAY_HEIGHT](RECT* rect) {
        rect->left = 0;
        rect->top = 0;
        rect->right = DISPLAY_WIDTH;
        rect->bottom = DISPLAY_HEIGHT;
        return S_OK;
      }));
  EXPECT_CALL(viewport, ZoomToRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false))
      .WillOnce(::testing::Return(S_OK));
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_INERTIA,
                                   DIRECTMANIPULATION_RUNNING);
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_READY,
                                   DIRECTMANIPULATION_INERTIA);
}

// Verify that scale mantissa rounding works as expected
TEST(DirectManipulationTest, TestRounding) {
  MockIDirectManipulationContent content;
  MockWindowBindingHandlerDelegate delegate;
  MockIDirectManipulationViewport viewport;
  const float scale = 1.5;
  const int DISPLAY_WIDTH = 800;
  const int DISPLAY_HEIGHT = 600;
  auto owner = std::make_unique<DirectManipulationOwner>(nullptr);
  owner->SetBindingHandlerDelegate(&delegate);
  auto handler =
      fml::MakeRefCounted<DirectManipulationEventHandler>(owner.get());
  int32_t device_id = (int32_t) reinterpret_cast<int64_t>(handler.get());
  EXPECT_CALL(viewport, GetPrimaryContent(_, _))
      .WillOnce(::testing::Invoke([&content](REFIID in, void** out) {
        *out = &content;
        return S_OK;
      }))
      .RetiresOnSaturation();
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([scale](float* transform, DWORD size) {
        transform[0] = 1.0f;
        transform[4] = 0.0;
        transform[5] = 0.0;
        return S_OK;
      }))
      .RetiresOnSaturation();
  EXPECT_CALL(delegate, OnPointerPanZoomStart(device_id));
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_RUNNING,
                                   DIRECTMANIPULATION_READY);
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([scale](float* transform, DWORD size) {
        transform[0] = 1.5000001f;
        transform[4] = 4.0;
        transform[5] = 0.0;
        return S_OK;
      }))
      .RetiresOnSaturation();
  EXPECT_CALL(delegate,
              OnPointerPanZoomUpdate(device_id, 4.0, 0, 1.5000001f, 0))
      .Times(0);
  EXPECT_CALL(delegate, OnPointerPanZoomUpdate(device_id, 4.0, 0, 1.5f, 0))
      .Times(1)
      .RetiresOnSaturation();
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([scale](float* transform, DWORD size) {
        transform[0] = 1.50000065f;
        transform[4] = 2.0;
        transform[5] = 0.0;
        return S_OK;
      }))
      .RetiresOnSaturation();
  EXPECT_CALL(delegate,
              OnPointerPanZoomUpdate(device_id, 2.0, 0, 1.50000065f, 0))
      .Times(0);
  EXPECT_CALL(delegate,
              OnPointerPanZoomUpdate(device_id, 2.0, 0, 1.50000047f, 0))
      .Times(1)
      .RetiresOnSaturation();
  EXPECT_CALL(delegate, OnPointerPanZoomEnd(device_id));
  EXPECT_CALL(viewport, GetViewportRect(_))
      .WillOnce(::testing::Invoke([DISPLAY_WIDTH, DISPLAY_HEIGHT](RECT* rect) {
        rect->left = 0;
        rect->top = 0;
        rect->right = DISPLAY_WIDTH;
        rect->bottom = DISPLAY_HEIGHT;
        return S_OK;
      }));
  EXPECT_CALL(viewport, ZoomToRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false))
      .WillOnce(::testing::Return(S_OK));
  handler->OnContentUpdated((IDirectManipulationViewport*)&viewport,
                            (IDirectManipulationContent*)&content);
  handler->OnContentUpdated((IDirectManipulationViewport*)&viewport,
                            (IDirectManipulationContent*)&content);
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_INERTIA,
                                   DIRECTMANIPULATION_RUNNING);
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_READY,
                                   DIRECTMANIPULATION_INERTIA);
}

TEST(DirectManipulationTest, TestInertiaCancelSentForUserCancel) {
  MockIDirectManipulationContent content;
  MockWindowBindingHandlerDelegate delegate;
  MockIDirectManipulationViewport viewport;
  const int DISPLAY_WIDTH = 800;
  const int DISPLAY_HEIGHT = 600;
  auto owner = std::make_unique<DirectManipulationOwner>(nullptr);
  owner->SetBindingHandlerDelegate(&delegate);
  auto handler =
      fml::MakeRefCounted<DirectManipulationEventHandler>(owner.get());
  int32_t device_id = (int32_t) reinterpret_cast<int64_t>(handler.get());
  // No need to mock the actual gesture, just start at the end.
  EXPECT_CALL(viewport, GetViewportRect(_))
      .WillOnce(::testing::Invoke([DISPLAY_WIDTH, DISPLAY_HEIGHT](RECT* rect) {
        rect->left = 0;
        rect->top = 0;
        rect->right = DISPLAY_WIDTH;
        rect->bottom = DISPLAY_HEIGHT;
        return S_OK;
      }));
  EXPECT_CALL(viewport, ZoomToRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false))
      .WillOnce(::testing::Return(S_OK));
  EXPECT_CALL(delegate, OnPointerPanZoomEnd(device_id));
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_INERTIA,
                                   DIRECTMANIPULATION_RUNNING);
  // Have pan_y change by 10 between inertia updates.
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([](float* transform, DWORD size) {
        transform[0] = 1;
        transform[4] = 0;
        transform[5] = 100;
        return S_OK;
      }));
  handler->OnContentUpdated((IDirectManipulationViewport*)&viewport,
                            (IDirectManipulationContent*)&content);
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([](float* transform, DWORD size) {
        transform[0] = 1;
        transform[4] = 0;
        transform[5] = 110;
        return S_OK;
      }));
  handler->OnContentUpdated((IDirectManipulationViewport*)&viewport,
                            (IDirectManipulationContent*)&content);
  // This looks like an interruption in the middle of synthetic inertia because
  // of user input.
  EXPECT_CALL(delegate, OnScrollInertiaCancel(device_id));
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_READY,
                                   DIRECTMANIPULATION_INERTIA);
}

TEST(DirectManipulationTest, TestInertiaCamcelNotSentAtInertiaEnd) {
  MockIDirectManipulationContent content;
  MockWindowBindingHandlerDelegate delegate;
  MockIDirectManipulationViewport viewport;
  const int DISPLAY_WIDTH = 800;
  const int DISPLAY_HEIGHT = 600;
  auto owner = std::make_unique<DirectManipulationOwner>(nullptr);
  owner->SetBindingHandlerDelegate(&delegate);
  auto handler =
      fml::MakeRefCounted<DirectManipulationEventHandler>(owner.get());
  int32_t device_id = (int32_t) reinterpret_cast<int64_t>(handler.get());
  // No need to mock the actual gesture, just start at the end.
  EXPECT_CALL(viewport, GetViewportRect(_))
      .WillOnce(::testing::Invoke([DISPLAY_WIDTH, DISPLAY_HEIGHT](RECT* rect) {
        rect->left = 0;
        rect->top = 0;
        rect->right = DISPLAY_WIDTH;
        rect->bottom = DISPLAY_HEIGHT;
        return S_OK;
      }));
  EXPECT_CALL(viewport, ZoomToRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false))
      .WillOnce(::testing::Return(S_OK));
  EXPECT_CALL(delegate, OnPointerPanZoomEnd(device_id));
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_INERTIA,
                                   DIRECTMANIPULATION_RUNNING);
  // Have no change in pan between events.
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([](float* transform, DWORD size) {
        transform[0] = 1;
        transform[4] = 0;
        transform[5] = 140;
        return S_OK;
      }));
  handler->OnContentUpdated((IDirectManipulationViewport*)&viewport,
                            (IDirectManipulationContent*)&content);
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([](float* transform, DWORD size) {
        transform[0] = 1;
        transform[4] = 0;
        transform[5] = 140;
        return S_OK;
      }));
  handler->OnContentUpdated((IDirectManipulationViewport*)&viewport,
                            (IDirectManipulationContent*)&content);
  // OnScrollInertiaCancel should not be called.
  EXPECT_CALL(delegate, OnScrollInertiaCancel(device_id)).Times(0);
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_READY,
                                   DIRECTMANIPULATION_INERTIA);
}

// Have some initial values in the matrix, only the differences should be
// reported.
TEST(DirectManipulationTest, TestGestureWithInitialData) {
  MockIDirectManipulationContent content;
  MockWindowBindingHandlerDelegate delegate;
  MockIDirectManipulationViewport viewport;
  const float scale = 1.5;
  const float pan_x = 32.0;
  const float pan_y = 16.0;
  const int DISPLAY_WIDTH = 800;
  const int DISPLAY_HEIGHT = 600;
  auto owner = std::make_unique<DirectManipulationOwner>(nullptr);
  owner->SetBindingHandlerDelegate(&delegate);
  auto handler =
      fml::MakeRefCounted<DirectManipulationEventHandler>(owner.get());
  int32_t device_id = (int32_t) reinterpret_cast<int64_t>(handler.get());
  EXPECT_CALL(viewport, GetPrimaryContent(_, _))
      .WillOnce(::testing::Invoke([&content](REFIID in, void** out) {
        *out = &content;
        return S_OK;
      }))
      .RetiresOnSaturation();
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke([scale](float* transform, DWORD size) {
        transform[0] = 2.0f;
        transform[4] = 234.0;
        transform[5] = 345.0;
        return S_OK;
      }))
      .RetiresOnSaturation();
  EXPECT_CALL(delegate, OnPointerPanZoomStart(device_id));
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_RUNNING,
                                   DIRECTMANIPULATION_READY);
  EXPECT_CALL(content, GetContentTransform(_, 6))
      .WillOnce(::testing::Invoke(
          [scale, pan_x, pan_y](float* transform, DWORD size) {
            transform[0] = 2.0f * scale;
            transform[4] = 234.0 + pan_x;
            transform[5] = 345.0 + pan_y;
            return S_OK;
          }));
  EXPECT_CALL(delegate,
              OnPointerPanZoomUpdate(device_id, pan_x, pan_y, scale, 0));
  handler->OnContentUpdated((IDirectManipulationViewport*)&viewport,
                            (IDirectManipulationContent*)&content);
  EXPECT_CALL(delegate, OnPointerPanZoomEnd(device_id));
  EXPECT_CALL(viewport, GetViewportRect(_))
      .WillOnce(::testing::Invoke([DISPLAY_WIDTH, DISPLAY_HEIGHT](RECT* rect) {
        rect->left = 0;
        rect->top = 0;
        rect->right = DISPLAY_WIDTH;
        rect->bottom = DISPLAY_HEIGHT;
        return S_OK;
      }));
  EXPECT_CALL(viewport, ZoomToRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false))
      .WillOnce(::testing::Return(S_OK));
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_INERTIA,
                                   DIRECTMANIPULATION_RUNNING);
  handler->OnViewportStatusChanged((IDirectManipulationViewport*)&viewport,
                                   DIRECTMANIPULATION_READY,
                                   DIRECTMANIPULATION_INERTIA);
}

}  // namespace testing
}  // namespace flutter
