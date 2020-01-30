#include "flutter/shell/platform/windows/win32_dpi_helper.h"

#include <iostream>
namespace flutter {

namespace {

constexpr UINT kDefaultDpi = 96;

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
}

Win32DpiHelper::~Win32DpiHelper() {
  if (user32_module_ != nullptr) {
    FreeLibrary(user32_module_);
  }
  if (shlib_module_ != nullptr) {
    FreeLibrary(shlib_module_);
  }
}

BOOL Win32DpiHelper::IsDpiPerWindowSupportedForWindow(HWND hwnd) {
  if (!dpi_for_window_api_loaded_) {
    dpi_for_window_supported_ =
        (AssignProcAddress(user32_module_, "GetDpiForWindow",
                           get_dpi_for_window_) &&
         AssignProcAddress(user32_module_, "GetWindowDpiAwarenessContext",
                           get_window_dpi_awareness_context_) &&
         AssignProcAddress(user32_module_, "AreDpiAwarenessContextsEqual",
                           are_dpi_awareness_contexts_equal));
    dpi_for_window_api_loaded_ = true;
  }
  if (!dpi_for_window_supported_) {
    return false;
  }

  DPI_AWARENESS_CONTEXT context = get_window_dpi_awareness_context_(hwnd);
  return are_dpi_awareness_contexts_equal(
      context, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

BOOL Win32DpiHelper::IsDpiPerMonitorSupported() {
  if (!dpi_for_monitor_api_loaded_) {
    shlib_module_ = LoadLibraryA("Shcore.dll");
    if (shlib_module_ == nullptr) {
      return false;
    }
    dpi_for_monitor_supported_ =
        (AssignProcAddress(shlib_module_, "GetDpiForMonitor",
                           get_dpi_for_monitor_) &&
         AssignProcAddress(user32_module_, "MonitorFromWindow",
                           monitor_from_window_));
    dpi_for_monitor_api_loaded_ = true;
  }
  return dpi_for_monitor_supported_;
}

UINT Win32DpiHelper::GetDpi(HWND hwnd) {
  // GetDpiForWindow returns the DPI for any awareness mode. If not available,
  // fallback to a per monitor, system, or default DPI.
  if (hwnd != nullptr && IsDpiPerWindowSupportedForWindow(hwnd)) {
    std::cerr << "v2\n";
    return get_dpi_for_window_(hwnd);
  }

  if (IsDpiPerMonitorSupported()) {
    std::cerr << "monitor\n";

    DWORD monitor_flag =
        hwnd == nullptr ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST;
    HMONITOR monitor = monitor_from_window_(hwnd, monitor_flag);
    UINT dpi_x = 0, dpi_y = 0;
    HRESULT result =
        get_dpi_for_monitor_(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
    return SUCCEEDED(result) ? dpi_x : kDefaultDpi;
  }
  std::cerr << "system\n";

  HDC hdc = GetDC(hwnd);
  UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
  ReleaseDC(hwnd, hdc);
  return dpi;
}
}  // namespace flutter
