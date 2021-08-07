// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/testing/mock_window_win32.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

void MockMessageQueue::InjectMessageList(int count,
                                         const Win32Message* messages) {
  for (int i = 0; i < count; i += 1) {
    _pending_messages.push_back(messages[i]);
  }
  while (!_pending_messages.empty()) {
    Win32Message message = _pending_messages.front();
    _pending_messages.pop_front();
    LRESULT result = Win32SendMessage(message.hWnd, message.message,
                                      message.wParam, message.lParam);
    if (message.expected_result != kWmResultDontCheck) {
      EXPECT_EQ(result, message.expected_result);
    }
  }
}

BOOL MockMessageQueue::Win32PeekMessage(LPMSG lpMsg,
                                        HWND hWnd,
                                        UINT wMsgFilterMin,
                                        UINT wMsgFilterMax,
                                        UINT wRemoveMsg) {
  for (auto iter = _pending_messages.begin(); iter != _pending_messages.end();
       ++iter) {
    if (iter->message >= wMsgFilterMin && iter->message <= wMsgFilterMax) {
      *lpMsg = MSG{
          .message = iter->message,
          .wParam = iter->wParam,
          .lParam = iter->lParam,
      };
      if ((wRemoveMsg & PM_REMOVE) == PM_REMOVE) {
        _pending_messages.erase(iter);
      }
      return TRUE;
    }
  }
  return FALSE;
}

MockWin32Window::MockWin32Window() : WindowWin32(){};

MockWin32Window::~MockWin32Window() = default;

UINT MockWin32Window::GetDpi() {
  return GetCurrentDPI();
}

LRESULT MockWin32Window::Win32DefWindowProc(HWND hWnd,
                                            UINT Msg,
                                            WPARAM wParam,
                                            LPARAM lParam) {
  return kWmResultDefault;
}

LRESULT MockWin32Window::InjectWindowMessage(UINT const message,
                                             WPARAM const wparam,
                                             LPARAM const lparam) {
  return HandleMessage(message, wparam, lparam);
}

LRESULT MockWin32Window::Win32SendMessage(HWND hWnd,
                                          UINT const message,
                                          WPARAM const wparam,
                                          LPARAM const lparam) {
  return HandleMessage(message, wparam, lparam);
}

}  // namespace testing
}  // namespace flutter
