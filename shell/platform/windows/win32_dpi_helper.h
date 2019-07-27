// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_DPI_HELPER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_DPI_HELPER_H_

#include <ShellScalingApi.h>
#include <VersionHelpers.h>

namespace flutter {

template <typename T>
bool AssignProcAddress(HMODULE comBaseModule, const char* name, T*& outProc) {
  outProc = reinterpret_cast<T*>(GetProcAddress(comBaseModule, name));
  return *outProc != nullptr;
}

// A helper class for abstracting various Windows DPI related functions across
// Windows OS versions.
class Win32DpiHelper {
 public:
  Win32DpiHelper();

  ~Win32DpiHelper();

  // Check if Windows Per Monitor V2 DPI scaling functionality is available on
  // current system.
  bool IsPerMonitorV2Available();

  // Wrapper for OS functionality to turn on automatic window non-client scaling
  BOOL EnableNonClientDpiScaling(HWND);

  // Wrapper for OS functionality to return the DPI for |HWND|
  UINT GetDpiForWindow(HWND);

  // Sets the current process to a specified DPI awareness context.
  BOOL SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT);

 private:
  using EnableNonClientDpiScaling_ = BOOL __stdcall(HWND);
  using GetDpiForWindow_ = UINT __stdcall(HWND);
  using SetProcessDpiAwarenessContext_ = BOOL __stdcall(DPI_AWARENESS_CONTEXT);

  EnableNonClientDpiScaling_* fp_enablenonclientdpiscaling{nullptr};
  GetDpiForWindow_* fp_getdpiforwindow{nullptr};
  SetProcessDpiAwarenessContext_* fp_setprocessdpiawarenesscontext{nullptr};

  HMODULE user32_module_{nullptr};
  bool permonitorv2_supported_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_DPI_HELPER_H_