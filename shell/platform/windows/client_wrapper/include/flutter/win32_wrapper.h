#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_WIN32_WRAPPER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_WIN32_WRAPPER_H_

#include <windows.h>

namespace flutter {

// Wraps Win32 API calls to enable mock-based testing.
class Win32Wrapper {
 public:
  virtual ~Win32Wrapper() = default;

  virtual HWND CreateWindowEx(DWORD dwExStyle,
                              LPCWSTR lpClassName,
                              LPCWSTR lpWindowName,
                              DWORD dwStyle,
                              int X,
                              int Y,
                              int nWidth,
                              int nHeight,
                              HWND hWndParent,
                              HMENU hMenu,
                              HINSTANCE hInstance,
                              LPVOID lpParam) {
    return ::CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y,
                            nWidth, nHeight, hWndParent, hMenu, hInstance,
                            lpParam);
  }
  virtual BOOL DestroyWindow(HWND hWnd) { return ::DestroyWindow(hWnd); }
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_WIN32_WRAPPER_H_
