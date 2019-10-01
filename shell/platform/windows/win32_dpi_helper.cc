#include "flutter/shell/platform/windows/win32_dpi_helper.h"

#include <iostream>

namespace flutter {

namespace {

template <typename T>
bool AssignProcAddress(HMODULE comBaseModule, const char* name, T*& outProc) {
  outProc = reinterpret_cast<T*>(GetProcAddress(comBaseModule, name));
  return *outProc != nullptr;
}

}  // namespace

Win32DpiHelper::Win32DpiHelper() {
  // TODO ensure that this helper works correctly on downlevel builds.
  user32_module_ = LoadLibraryA("User32.dll");
  if (user32_module_ == nullptr) {
    return;
  }
  if (!AssignProcAddress(user32_module_, "EnableNonClientDpiScaling",
                         enable_non_client_dpi_scaling_)) {
    return;
  }

  if (!AssignProcAddress(user32_module_, "GetDpiForWindow",
                         get_dpi_for_window_)) {
    return;
  }

  if (!AssignProcAddress(user32_module_, "SetProcessDpiAwarenessContext",
                         set_process_dpi_awareness_context_)) {
    return;
  }
  permonitorv2_supported_ = true;
}

Win32DpiHelper::~Win32DpiHelper() {
  if (user32_module_ != nullptr) {
    FreeLibrary(user32_module_);
  }
}

// SetAwareness
// Check the available api. Start with v2, then fall back.
void Win32DpiHelper::SetDpiAwerenessAllVersions() {
  if (permonitorv2_supported_) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  } else {
    std::cerr << "Per Monitor V2 DPI awareness is not supported on this "
                 "version of Windows. Setting System DPI awareness\n";
    AssignProcAddress(user32_module_, "GetDpiForSystem", get_dpi_for_system_);
    AssignProcAddress(user32_module_, "SetProcessDpiAware",
                      set_process_dpi_aware_);

    SetProcessDPIAware();
  }
}

bool Win32DpiHelper::IsPerMonitorV2Available() {
  return permonitorv2_supported_;
}

BOOL Win32DpiHelper::EnableNonClientDpiScaling(HWND hwnd) {
  if (!permonitorv2_supported_) {
    return false;
  }
  return enable_non_client_dpi_scaling_(hwnd);
}

UINT Win32DpiHelper::GetDpiForWindow(HWND hwnd) {
  if (!permonitorv2_supported_) {
    return false;
  }
  return get_dpi_for_window_(hwnd);
}

UINT Win32DpiHelper::GetDpiForSystem() {
  return get_dpi_for_system_();
}

BOOL Win32DpiHelper::SetProcessDpiAwarenessContext(
    DPI_AWARENESS_CONTEXT context) {
  if (!permonitorv2_supported_) {
    return false;
  }
  return set_process_dpi_awareness_context_(context);
}

BOOL Win32DpiHelper::SetProcessDpiAware() {
  return set_process_dpi_aware_();
}

}  // namespace flutter
