#include "flutter/shell/platform/windows/win32_dpi_helper.h"

namespace flutter {

Win32DpiHelper::Win32DpiHelper()
    : fp_enablenonclientdpiscaling(nullptr),
      permonitorv2_supported_(false),
      fp_getdpiforwindow(nullptr),
      fp_setprocessdpiawarenesscontext(nullptr),
      user32_module_(nullptr) {
  /* if (!IsWindows10OrGreater()) { // requires manifest setting in calling app
     return;
   }*/

  user32_module_ = LoadLibraryA("User32.dll");

  if (user32_module_ == nullptr) {
    return;
  }

  if (!AssignProcAddress(user32_module_, "EnableNonClientDpiScaling",
                         fp_enablenonclientdpiscaling)) {
    return;
  }

  if (!AssignProcAddress(user32_module_, "GetDpiForWindow",
                         fp_getdpiforwindow)) {
    return;
  }

  if (!AssignProcAddress(user32_module_, "SetProcessDpiAwarenessContext",
                         fp_setprocessdpiawarenesscontext)) {
    return;
  }

  permonitorv2_supported_ = true;
}

Win32DpiHelper::~Win32DpiHelper() {
  if (user32_module_ != nullptr) {
    FreeLibrary(user32_module_);
    user32_module_ = nullptr;
  }
}

bool Win32DpiHelper::IsPerMonitorV2Available() {
  return permonitorv2_supported_;
}

BOOL Win32DpiHelper::EnableNonClientDpiScaling(HWND hwnd) {
  if (!permonitorv2_supported_) {
    return false;
  }
  return fp_enablenonclientdpiscaling(hwnd);
}

UINT Win32DpiHelper::GetDpiForWindow(HWND hwnd) {
  if (!permonitorv2_supported_) {
    return false;
  }
  return fp_getdpiforwindow(hwnd);
}

BOOL Win32DpiHelper::SetProcessDpiAwarenessContext(
    DPI_AWARENESS_CONTEXT context) {
  if (!permonitorv2_supported_) {
    return false;
  }
  return fp_setprocessdpiawarenesscontext(context);
}

}  // namespace flutter