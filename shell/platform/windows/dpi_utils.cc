#include "dpi_utils.h"

#include <ShellScalingApi.h>

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

  /// Enables scaling of non-client UI (scrolling bars, title bars, etc). Only
  /// supported on Per-Monitor V1 DPI awareness mode.
  BOOL EnableNonClientDpiScaling(HWND hwnd);

 private:
  using GetDpiForWindow_ = UINT __stdcall(HWND);
  using GetDpiForMonitor_ = HRESULT __stdcall(HMONITOR hmonitor,
                                              MONITOR_DPI_TYPE dpiType,
                                              UINT* dpiX,
                                              UINT* dpiY);
  using MonitorFromWindow_ = HMONITOR __stdcall(HWND hwnd, DWORD dwFlags);
  using EnableNonClientDpiScaling_ = BOOL __stdcall(HWND hwnd);

  GetDpiForWindow_* get_dpi_for_window_ = nullptr;
  GetDpiForMonitor_* get_dpi_for_monitor_ = nullptr;
  MonitorFromWindow_* monitor_from_window_ = nullptr;
  EnableNonClientDpiScaling_* enable_non_client_dpi_scaling_ = nullptr;

  HMODULE user32_module_ = nullptr;
  HMODULE shlib_module_ = nullptr;
  bool dpi_for_window_supported_ = false;
  bool dpi_for_monitor_supported_ = false;
};

Win32DpiHelper::Win32DpiHelper() {
  user32_module_ = LoadLibraryA("User32.dll");
  shlib_module_ = LoadLibraryA("Shcore.dll");
  if (shlib_module_ == nullptr && user32_module_ == nullptr) {
    return;
  }

  dpi_for_window_supported_ =
      (AssignProcAddress(user32_module_, "GetDpiForWindow",
                         get_dpi_for_window_) &&
       AssignProcAddress(user32_module_, "EnableNonClientDpiScaling",
                         enable_non_client_dpi_scaling_));
  dpi_for_monitor_supported_ =
      (AssignProcAddress(shlib_module_, "GetDpiForMonitor",
                         get_dpi_for_monitor_) &&
       AssignProcAddress(user32_module_, "MonitorFromWindow",
                         monitor_from_window_));
}

Win32DpiHelper::~Win32DpiHelper() {
  if (user32_module_ != nullptr) {
    FreeLibrary(user32_module_);
  }
  if (shlib_module_ != nullptr) {
    FreeLibrary(shlib_module_);
  }
}

BOOL Win32DpiHelper::EnableNonClientDpiScaling(HWND hwnd) {
  if (enable_non_client_dpi_scaling_ == nullptr) {
    return false;
  }
  return enable_non_client_dpi_scaling_(hwnd);
}

UINT Win32DpiHelper::GetDpi(HWND hwnd) {
  // GetDpiForWindow returns the DPI for any awareness mode. If not available,
  // or no |hwnd| is provided. fallback to a per monitor, system, or default
  // DPI.
  if (dpi_for_window_supported_ && hwnd != nullptr) {
    return get_dpi_for_window_(hwnd);
  }

  if (dpi_for_monitor_supported_) {
    HMONITOR monitor = monitor_from_window_(hwnd, MONITOR_DEFAULTTONEAREST);
    UINT dpi_x = 0, dpi_y = 0;
    HRESULT result =
        get_dpi_for_monitor_(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
    return SUCCEEDED(result) ? dpi_x : kDefaultDpi;
  }
  HDC hdc = GetDC(hwnd);
  UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
  ReleaseDC(hwnd, hdc);
  return dpi;
}

Win32DpiHelper* GetHelper() {
  static Win32DpiHelper* dpi_helper = new Win32DpiHelper();
  return dpi_helper;
}

UINT GetDpiForHWND(HWND hwnd) {
  return GetHelper()->GetDpi(hwnd);
}

BOOL EnableNonClientDpiScaling(HWND hwnd) {
  return GetHelper()->EnableNonClientDpiScaling(hwnd);
}
}  // namespace flutter
