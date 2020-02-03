#include "dpi_utils.h"

#include <ShellScalingApi.h>

namespace flutter {

namespace {

constexpr UINT kDefaultDpi = 96;

template <typename T>

/// Retrieves a function named |name| from a given module in |comBaseModule|
/// into |outProc|. Returns a bool indicating whether the function was found.
bool AssignProcAddress(HMODULE comBaseModule, const char* name, T*& outProc) {
  outProc = reinterpret_cast<T*>(GetProcAddress(comBaseModule, name));
  return *outProc != nullptr;
}

/// A helper class for abstracting various Windows DPI related functions across
/// Windows OS versions.
class Win32DpiHelper {
 public:
  Win32DpiHelper();

  ~Win32DpiHelper();

  /// Returns the current DPI. Supports all DPI awareness modes, and is backward
  /// compatible down to Windows Vista. If |hwnd| is nullptr, returns the DPI
  /// for the nearest monitor is available. Otherwise, returns the system's DPI.
  UINT GetDpiForWindow(HWND);

  /// Returns the DPI of a given monitor. Defaults to 96 if the API is not
  /// available.
  UINT GetDpiForMonitor(HMONITOR);

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

  dpi_for_window_supported_ = (AssignProcAddress(
      user32_module_, "GetDpiForWindow", get_dpi_for_window_));
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

UINT Win32DpiHelper::GetDpiForWindow(HWND hwnd) {
  // GetDpiForWindow returns the DPI for any awareness mode. If not available,
  // or no |hwnd| is provided, fallback to a per monitor, system, or default
  // DPI.
  if (dpi_for_window_supported_ && hwnd != nullptr) {
    return get_dpi_for_window_(hwnd);
  }

  if (dpi_for_monitor_supported_) {
    HMONITOR monitor = monitor_from_window_(hwnd, MONITOR_DEFAULTTOPRIMARY);
    return GetDpiForMonitor(monitor);
  }
  HDC hdc = GetDC(hwnd);
  UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
  ReleaseDC(hwnd, hdc);
  return dpi;
}

UINT Win32DpiHelper::GetDpiForMonitor(HMONITOR monitor) {
  if (dpi_for_monitor_supported_) {
    UINT dpi_x = 0, dpi_y = 0;
    HRESULT result =
        get_dpi_for_monitor_(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
    return SUCCEEDED(result) ? dpi_x : kDefaultDpi;
  }
  return kDefaultDpi;
}

Win32DpiHelper* GetHelper() {
  static Win32DpiHelper* dpi_helper = new Win32DpiHelper();
  return dpi_helper;
}
}  // namespace

UINT GetDpiForHWND(HWND hwnd) {
  return GetHelper()->GetDpiForWindow(hwnd);
}

UINT GetDpiForMonitor(HMONITOR monitor) {
  return GetHelper()->GetDpiForMonitor(monitor);
}
}  // namespace flutter
