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

class Win32DpiHelper {
 public:
  Win32DpiHelper();

  ~Win32DpiHelper();

  bool IsPerMonitorV2Available();

  BOOL EnableNonClientDpiScaling(HWND);

  UINT GetDpiForWindow(HWND);

  BOOL SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT);

 private:
  using EnableNonClientDpiScaling_ = BOOL __stdcall(HWND);
  using GetDpiForWindow_ = UINT __stdcall(HWND);
  using SetProcessDpiAwarenessContext_ = BOOL __stdcall(DPI_AWARENESS_CONTEXT);

  EnableNonClientDpiScaling_* fp_enablenonclientdpiscaling;
  GetDpiForWindow_* fp_getdpiforwindow;
  SetProcessDpiAwarenessContext_* fp_setprocessdpiawarenesscontext;

  HMODULE user32_module_;
  bool permonitorv2_supported_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_DPI_HELPER_H_