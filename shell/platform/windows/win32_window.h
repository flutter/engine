// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WIN32_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WIN32_WINDOW_H_

#include <Windows.h>
#include <Windowsx.h>
#include <memory>
#include <string>

#include "flutter/shell/platform/windows/win32_dpi_helper.h"

namespace flutter {

class Win32Window {
 public:
  Win32Window();
  ~Win32Window();

  void Initialize(const char* title,
                  const unsigned int x,
                  const unsigned int y,
                  const unsigned int width,
                  const unsigned int height);

  virtual void Destroy();

  std::wstring NarrowToWide(const char* source);

  WNDCLASS ResgisterWindowClass(std::wstring& title);

  static LRESULT CALLBACK WndProc(HWND const window,
                                  UINT const message,
                                  WPARAM const wparam,
                                  LPARAM const lparam) noexcept;

  LRESULT
  MessageHandler(HWND window,
                 UINT const message,
                 WPARAM const wparam,
                 LPARAM const lparam) noexcept;

  // DPI Change handler. on WM_DPICHANGE resize the window
  LRESULT
  HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam);

  virtual void OnDpiScale(UINT dpi) = 0;

  virtual void OnResize(UINT width, UINT height) = 0;

  virtual void OnPointerMove(double x, double y) = 0;
  virtual void OnPointerDown(double x, double y) = 0;
  virtual void OnPointerUp(double x, double y) = 0;
  virtual void OnChar(unsigned int code_point) = 0;
  virtual void OnKey(int key, int scancode, int action, int mods) = 0;
  virtual void OnScroll(double delta_x, double delta_y) = 0;

 protected:
  static Win32Window* GetThisFromHandle(HWND const window) noexcept;
  int current_dpi_ = 0;
  int current_width_ = 0;
  int current_height_ = 0;

  HWND window_handle_ = nullptr;
  std::wstring window_class_name_;

  std::unique_ptr<Win32DpiHelper> dpi_helper_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WIN32_WINDOW_H_