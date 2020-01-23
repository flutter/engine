#include "flutter/shell/platform/windows/win32_dpi_helper.h"

namespace flutter {

namespace {

template <typename T>
bool AssignProcAddress(HMODULE comBaseModule, const char* name, T*& outProc) {
  outProc = reinterpret_cast<T*>(GetProcAddress(comBaseModule, name));
  return *outProc != nullptr;
}

}  // namespace

Win32DpiHelper::Win32DpiHelper() {
  user32_module_ = LoadLibraryA("User32.dll");
  if (user32_module_ == nullptr) {
    return;
  }
  shlib_module_ = LoadLibraryA("Shcore.dll");
  if (shlib_module_ == nullptr) {
    return;
  }

  if (AssignProcAddress(user32_module_, "GetDpiForWindow",
                        get_dpi_for_window_)) {
    dpi_for_window_supported_ = true;
  }

  if (AssignProcAddress(shlib_module_, "GetDpiForMonitor",
                        get_dpi_for_monitor_) &&
      AssignProcAddress(user32_module_, "MonitorFromWindow",
                        monitor_from_window_)) {
    dpi_for_monitor_supported_ = true;
  }
}

Win32DpiHelper::~Win32DpiHelper() {
  if (user32_module_ != nullptr) {
    FreeLibrary(user32_module_);
  }
  if (shlib_module_ != nullptr) {
    FreeLibrary(shlib_module_);
  }
}

UINT Win32DpiHelper::GetDpi(HWND hwnd) {
  // GetDpiForWindow returns the DPI for any awareness mode. If not available, fallback to a
  // per monitor, system, or default DPI.
  if (dpi_for_window_supported_) {
    return get_dpi_for_window_(hwnd);
  } else if (dpi_for_monitor_supported_) {
    HMONITOR monitor = monitor_from_window_(hwnd, MONITOR_DEFAULTTONEAREST);
    UINT dpi_x = 0, dpi_y = 0;
    HRESULT result =
        get_dpi_for_monitor_(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
    if (result == S_OK) {
      return dpi_x;
    }
  } else {
    HDC hdc = GetDC(hwnd);
    UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(hwnd, hdc);
    return dpi;
  }
  return default_dpi_;
}
}  // namespace flutter
