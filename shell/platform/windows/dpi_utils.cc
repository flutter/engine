#include "dpi_utils.h"

#include <ShellScalingApi.h>

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

/// A helper class for abstracting various Windows DPI related functions across
/// Windows OS versions.
class Win32DpiHelper {
 public:
  Win32DpiHelper();

  ~Win32DpiHelper();

  /// Returns the current DPI. Supports all DPI awareness modes, and is backward
  /// compatible down to Windows Vista.
  UINT GetDpi(HWND);
  BOOL EnableNonClientDpiScaling(HWND hwnd);

 private:
  BOOL IsDpiPerWindowSupportedForWindow(HWND hwnd);
  BOOL IsDpiPerMonitorSupported();

  using GetDpiForWindow_ = UINT __stdcall(HWND);
  using GetDpiForMonitor_ = HRESULT __stdcall(HMONITOR hmonitor,
                                              MONITOR_DPI_TYPE dpiType,
                                              UINT* dpiX,
                                              UINT* dpiY);
  using MonitorFromWindow_ = HMONITOR __stdcall(HWND hwnd, DWORD dwFlags);
  using EnableNonClientDpiScaling_ = BOOL __stdcall(HWND hwnd);
  using GetWindowDpiAwarenessContext_ =
      DPI_AWARENESS_CONTEXT __stdcall(HWND hwnd);
  using AreDpiAwarenessContextsEqual_ =
      BOOL __stdcall(DPI_AWARENESS_CONTEXT dpiContextA,
                     DPI_AWARENESS_CONTEXT dpiContextB);

  GetDpiForWindow_* get_dpi_for_window_ = nullptr;
  GetDpiForMonitor_* get_dpi_for_monitor_ = nullptr;
  MonitorFromWindow_* monitor_from_window_ = nullptr;
  EnableNonClientDpiScaling_* enable_non_client_dpi_scaling_ = nullptr;
  GetWindowDpiAwarenessContext_* get_window_dpi_awareness_context_ = nullptr;
  AreDpiAwarenessContextsEqual_* are_dpi_awareness_contexts_equal = nullptr;

  HMODULE user32_module_ = nullptr;
  HMODULE shlib_module_ = nullptr;
  bool dpi_for_window_supported_ = false;
  bool dpi_for_monitor_supported_ = false;
  bool dpi_for_window_api_loaded_ = false;
  bool dpi_for_monitor_api_loaded_ = false;
};

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
  std::cerr << context << std::endl;

  bool is_v2 = are_dpi_awareness_contexts_equal(
      context, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  if (!is_v2) {
    AssignProcAddress(user32_module_, "EnableNonClientDpiScaling",
                      enable_non_client_dpi_scaling_);
  }
  return is_v2;
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

BOOL Win32DpiHelper::EnableNonClientDpiScaling(HWND hwnd) {
  if (enable_non_client_dpi_scaling_ == nullptr) {
    std::cerr << "Not loaded\n";
    return false;
  }
  return enable_non_client_dpi_scaling_(hwnd);
}

UINT Win32DpiHelper::GetDpi(HWND hwnd) {
  // GetDpiForWindow returns the DPI for any awareness mode. If not available,
  // fallback to a per monitor, system, or default DPI.
  if (IsDpiPerWindowSupportedForWindow(hwnd) && hwnd != nullptr) {
    std::cerr << "v2\n";
    return get_dpi_for_window_(hwnd);
  }

  if (IsDpiPerMonitorSupported()) {
    std::cerr << "monitor\n";

    HMONITOR monitor = monitor_from_window_(hwnd, MONITOR_DEFAULTTONEAREST);
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

Win32DpiHelper* GetHelper() {
  static Win32DpiHelper* dpi_helper = new Win32DpiHelper();
  return dpi_helper;
}

UINT GetDpiForView(HWND hwnd) {
  return GetHelper()->GetDpi(hwnd);
}

BOOL EnableNonClientDpiScaling(HWND hwnd) {
  return GetHelper()->EnableNonClientDpiScaling(hwnd);
}

}  // namespace flutter
