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

// A class abstraction for a high DPI aware Win32 Window.  Intended to be
// inherited from by classes that wish to specialize with custom
// rendering and input handling
class Win32Window {
 public:
  Win32Window();
  ~Win32Window();

  // Initialize and show window with |title| and position and size using |x|,
  // |y|, |width| and |height|
  void Initialize(const char* title,
                  const unsigned int x,
                  const unsigned int y,
                  const unsigned int width,
                  const unsigned int height);

  // Release OS resources asociated with window.
  virtual void Destroy();

 protected:
  // Helper function to convert a c string to a wide unicode string.
  std::wstring NarrowToWide(const char* source);

  // Helper to register a window class with default style attributes, cursor and
  // icon.
  WNDCLASS ResgisterWindowClass(std::wstring& title);

  // OS callback called by message pump.  Handles the WM_NCCREATE message which
  // is passed when the non-client area is being created and enables automatic
  // non-client DPI scaling so that the non-client area automatically
  // responsponds to changes in DPI.  All other messages are handled by
  // MessageHandler.
  static LRESULT CALLBACK WndProc(HWND const window,
                                  UINT const message,
                                  WPARAM const wparam,
                                  LPARAM const lparam) noexcept;

  // Function to process and route salient window messages for mouse handling,
  // size change and DPI.  Delegates handling of these to member overloads that
  // inheriting classes can handle.
  LRESULT
  MessageHandler(HWND window,
                 UINT const message,
                 WPARAM const wparam,
                 LPARAM const lparam) noexcept;

  // DPI Change handler. on WM_DPICHANGE resize the window to the new suggested
  // size and notify inheriting class.
  LRESULT
  HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam);

  // Virtual overloads that inheriting classes and override to be notified of
  // windowing and input events they care about.
  virtual void OnDpiScale(UINT dpi) = 0;
  virtual void OnResize(UINT width, UINT height) = 0;
  virtual void OnPointerMove(double x, double y) = 0;
  virtual void OnPointerDown(double x, double y) = 0;
  virtual void OnPointerUp(double x, double y) = 0;
  virtual void OnChar(unsigned int code_point) = 0;
  virtual void OnKey(int key, int scancode, int action, int mods) = 0;
  virtual void OnScroll(double delta_x, double delta_y) = 0;
  virtual void OnClose() = 0;

  // Static helper to retrieve a class instance pointer for |window|
  static Win32Window* GetThisFromHandle(HWND const window) noexcept;
  int current_dpi_ = 0;
  int current_width_ = 0;
  int current_height_ = 0;

  // Member variable to hold window handle.
  HWND window_handle_ = nullptr;

  // Member variable to hold the window title.
  std::wstring window_class_name_;

  // Member variable referencing an instance of dpi_helper used to abstract some
  // aspects of win32 High DPI handling across different OS versions.
  std::unique_ptr<Win32DpiHelper> dpi_helper_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WIN32_WINDOW_H_