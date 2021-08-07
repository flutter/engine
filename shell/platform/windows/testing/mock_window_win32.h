// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_MOCK_WIN32_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_MOCK_WIN32_WINDOW_H_

#include <windowsx.h>
#include <list>

#include "flutter/shell/platform/windows/window_win32.h"
#include "flutter/shell/platform/windows/testing/test_keyboard.h"
#include "gmock/gmock.h"

namespace flutter {
namespace testing {

constexpr LRESULT kMockDefaultResult = 0xDEADC0DE;
constexpr LRESULT kMockDontCheckResult = 0xFFFF1234;

// A struc to hold simulated events that will be delivered after the framework
// response is handled.
struct Win32Message {
  UINT message;
  WPARAM wparam;
  LPARAM lparam;
  LRESULT expected_result;
  HWND hWnd;
};

class MockMessageQueue {
 public:
  // Simulates a WindowProc message from the OS.
  void InjectMessageList(int count, const Win32Message* messages);

  BOOL Win32PeekMessage(LPMSG lpMsg,
                        HWND hWnd,
                        UINT wMsgFilterMin,
                        UINT wMsgFilterMax,
                        UINT wRemoveMsg);

 protected:
  virtual LRESULT Win32SendMessage(HWND hWnd,
                                   UINT const message,
                                   WPARAM const wparam,
                                   LPARAM const lparam) = 0;

  std::list<Win32Message> _pending_messages;
};

/// Mock for the |WindowWin32| base class.
class MockWin32Window : public WindowWin32,
                        public MockMessageQueue {
 public:
  MockWin32Window();
  virtual ~MockWin32Window();

  // Prevent copying.
  MockWin32Window(MockWin32Window const&) = delete;
  MockWin32Window& operator=(MockWin32Window const&) = delete;

  // Wrapper for GetCurrentDPI() which is a protected method.
  UINT GetDpi();

  // Simulates a WindowProc message from the OS.
  LRESULT InjectWindowMessage(UINT const message,
                              WPARAM const wparam,
                              LPARAM const lparam);

  LRESULT Win32DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

  MOCK_METHOD1(OnDpiScale, void(unsigned int));
  MOCK_METHOD2(OnResize, void(unsigned int, unsigned int));
  MOCK_METHOD2(OnPointerMove, void(double, double));
  MOCK_METHOD3(OnPointerDown, void(double, double, UINT));
  MOCK_METHOD3(OnPointerUp, void(double, double, UINT));
  MOCK_METHOD0(OnPointerLeave, void());
  MOCK_METHOD0(OnSetCursor, void());
  MOCK_METHOD1(OnText, void(const std::u16string&));
  MOCK_METHOD6(OnKey, bool(int, int, int, char32_t, bool, bool));
  MOCK_METHOD2(OnScroll, void(double, double));
  MOCK_METHOD0(OnComposeBegin, void());
  MOCK_METHOD0(OnComposeCommit, void());
  MOCK_METHOD0(OnComposeEnd, void());
  MOCK_METHOD2(OnComposeChange, void(const std::u16string&, int));

 protected:
  LRESULT Win32SendMessage(HWND hWnd,
                           UINT const message,
                           WPARAM const wparam,
                           LPARAM const lparam) override;
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_MOCK_WIN32_WINDOW_H_
