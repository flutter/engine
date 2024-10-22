#include "include/flutter/win32_window.h"
#include "include/flutter/flutter_window_controller.h"

#include "flutter_windows.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <dwmapi.h>

namespace {

auto const* const kWindowClassName{L"FLUTTER_WIN32_WINDOW"};

// The number of Win32Window objects that currently exist.
static int gActiveWindowCount{0};
// A mutex for thread-safe use of the window count.
static std::mutex gActiveWindowMutex;

// Retrieves the calling thread's last-error code message as a string,
// or a fallback message if the error message cannot be formatted.
auto GetLastErrorAsString() -> std::string {
  LPWSTR message_buffer{nullptr};

  if (auto const size{FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
              FORMAT_MESSAGE_IGNORE_INSERTS,
          nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          reinterpret_cast<LPTSTR>(&message_buffer), 0, nullptr)}) {
    std::wstring const wide_message(message_buffer, size);
    LocalFree(message_buffer);
    message_buffer = nullptr;

    if (auto const buffer_size{
            WideCharToMultiByte(CP_UTF8, 0, wide_message.c_str(), -1, nullptr,
                                0, nullptr, nullptr)}) {
      std::string message(buffer_size, 0);
      WideCharToMultiByte(CP_UTF8, 0, wide_message.c_str(), -1, &message[0],
                          buffer_size, nullptr, nullptr);
      return message;
    }
  }

  if (message_buffer) {
    LocalFree(message_buffer);
  }
  std::ostringstream oss;
  oss << "Format message failed with 0x" << std::hex << std::setfill('0')
      << std::setw(8) << GetLastError() << '\n';
  return oss.str();
}

// Calculates the required window size, in physical coordinates, to
// accommodate the given |client_size| (in logical coordinates) for a window
// with the specified |window_style| and |extended_window_style|. The result
// accounts for window borders, non-client areas, and drop-shadow effects.
auto GetWindowSizeForClientSize(flutter::WindowSize const& client_size,
                                DWORD window_style,
                                DWORD extended_window_style,
                                HWND parent_hwnd) -> flutter::WindowSize {
  auto const dpi{FlutterDesktopGetDpiForHWND(parent_hwnd)};
  auto const scale_factor{static_cast<double>(dpi) / USER_DEFAULT_SCREEN_DPI};
  RECT rect{.left = 0,
            .top = 0,
            .right = static_cast<LONG>(client_size.width * scale_factor),
            .bottom = static_cast<LONG>(client_size.height * scale_factor)};

  HMODULE const user32_module{LoadLibraryA("User32.dll")};
  if (user32_module) {
    using AdjustWindowRectExForDpi = BOOL __stdcall(
        LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);

    auto* const adjust_window_rect_ext_for_dpi{
        reinterpret_cast<AdjustWindowRectExForDpi*>(
            GetProcAddress(user32_module, "AdjustWindowRectExForDpi"))};
    if (adjust_window_rect_ext_for_dpi) {
      if (adjust_window_rect_ext_for_dpi(&rect, window_style, FALSE,
                                         extended_window_style, dpi)) {
        FreeLibrary(user32_module);
        return {static_cast<int>(rect.right - rect.left),
                static_cast<int>(rect.bottom - rect.top)};
      } else {
        std::cerr << "Failed to run AdjustWindowRectExForDpi: "
                  << GetLastErrorAsString() << '\n';
      }
    } else {
      std::cerr << "Failed to retrieve AdjustWindowRectExForDpi address from "
                   "User32.dll.\n";
    }
    FreeLibrary(user32_module);
  } else {
    std::cerr << "Failed to load User32.dll.\n";
  }

  if (!AdjustWindowRectEx(&rect, window_style, FALSE, extended_window_style)) {
    std::cerr << "Failed to run AdjustWindowRectEx: " << GetLastErrorAsString()
              << '\n';
  }
  return {static_cast<int>(rect.right - rect.left),
          static_cast<int>(rect.bottom - rect.top)};
}

// Dynamically loads the |EnableNonClientDpiScaling| from the User32 module
// so that the non-client area automatically responds to changes in DPI.
// This API is only needed for PerMonitor V1 awareness mode.
void EnableFullDpiSupportIfAvailable(HWND hwnd) {
  HMODULE user32_module = LoadLibraryA("User32.dll");
  if (!user32_module) {
    return;
  }

  using EnableNonClientDpiScaling = BOOL __stdcall(HWND hwnd);

  auto enable_non_client_dpi_scaling =
      reinterpret_cast<EnableNonClientDpiScaling*>(
          GetProcAddress(user32_module, "EnableNonClientDpiScaling"));
  if (enable_non_client_dpi_scaling != nullptr) {
    enable_non_client_dpi_scaling(hwnd);
  }

  FreeLibrary(user32_module);
}

// Dynamically loads |SetWindowCompositionAttribute| from the User32 module to
// make the window's background transparent.
void EnableTransparentWindowBackground(HWND hwnd) {
  HMODULE const user32_module{LoadLibraryA("User32.dll")};
  if (!user32_module) {
    return;
  }

  enum WINDOWCOMPOSITIONATTRIB { WCA_ACCENT_POLICY = 19 };

  struct WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
  };

  using SetWindowCompositionAttribute =
      BOOL(__stdcall*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

  auto set_window_composition_attribute{
      reinterpret_cast<SetWindowCompositionAttribute>(
          GetProcAddress(user32_module, "SetWindowCompositionAttribute"))};
  if (set_window_composition_attribute != nullptr) {
    enum ACCENT_STATE { ACCENT_DISABLED = 0 };

    struct ACCENT_POLICY {
      ACCENT_STATE AccentState;
      DWORD AccentFlags;
      DWORD GradientColor;
      DWORD AnimationId;
    };

    // Set the accent policy to disable window composition
    ACCENT_POLICY accent{ACCENT_DISABLED, 2, static_cast<DWORD>(0), 0};
    WINDOWCOMPOSITIONATTRIBDATA data{.Attrib = WCA_ACCENT_POLICY,
                                     .pvData = &accent,
                                     .cbData = sizeof(accent)};
    set_window_composition_attribute(hwnd, &data);

    // Extend the frame into the client area and set the window's system
    // backdrop type for visual effects
    MARGINS const margins{-1};
    ::DwmExtendFrameIntoClientArea(hwnd, &margins);
    INT effect_value{1};
    ::DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &effect_value,
                            sizeof(BOOL));
  }

  FreeLibrary(user32_module);
}

/// Window attribute that enables dark mode window decorations.
///
/// Redefined in case the developer's machine has a Windows SDK older than
/// version 10.0.22000.0.
/// See:
/// https://docs.microsoft.com/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

// Update the window frame's theme to match the system theme.
void UpdateTheme(HWND window) {
  // Registry key for app theme preference.
  const wchar_t kGetPreferredBrightnessRegKey[] =
      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
  const wchar_t kGetPreferredBrightnessRegValue[] = L"AppsUseLightTheme";

  // A value of 0 indicates apps should use dark mode. A non-zero or missing
  // value indicates apps should use light mode.
  DWORD light_mode;
  DWORD light_mode_size = sizeof(light_mode);
  LSTATUS const result =
      RegGetValue(HKEY_CURRENT_USER, kGetPreferredBrightnessRegKey,
                  kGetPreferredBrightnessRegValue, RRF_RT_REG_DWORD, nullptr,
                  &light_mode, &light_mode_size);

  if (result == ERROR_SUCCESS) {
    BOOL enable_dark_mode = light_mode == 0;
    DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE,
                          &enable_dark_mode, sizeof(enable_dark_mode));
  }
}

auto IsClassRegistered(LPCWSTR class_name) -> bool {
  WNDCLASSEX window_class{};
  return GetClassInfoEx(GetModuleHandle(nullptr), class_name, &window_class) !=
         0;
}

}  // namespace

namespace flutter {

Win32Window::Win32Window() : win32_{std::make_shared<Win32Wrapper>()} {}

Win32Window::Win32Window(std::shared_ptr<Win32Wrapper> wrapper)
    : win32_{std::move(wrapper)} {}

Win32Window::~Win32Window() {
  std::lock_guard lock(gActiveWindowMutex);
  if (--gActiveWindowCount == 0) {
    UnregisterClass(kWindowClassName, GetModuleHandle(nullptr));
  }
}

auto Win32Window::GetThisFromHandle(HWND hwnd) -> Win32Window* {
  return reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

auto Win32Window::GetHandle() const -> HWND {
  return window_handle_;
}

void Win32Window::SetQuitOnClose(bool quit_on_close) {
  quit_on_close_ = quit_on_close;
}

auto Win32Window::GetQuitOnClose() const -> bool {
  return quit_on_close_;
}

auto Win32Window::GetClientArea() const -> RECT {
  RECT client_rect;
  GetClientRect(window_handle_, &client_rect);
  return client_rect;
}

auto Win32Window::GetArchetype() const -> WindowArchetype {
  return archetype_;
}

auto Win32Window::Create(std::wstring const& title,
                         WindowSize const& client_size,
                         WindowArchetype archetype) -> bool {
  std::lock_guard lock(gActiveWindowMutex);

  archetype_ = archetype;

  DWORD window_style{};
  DWORD extended_window_style{};

  switch (archetype) {
    case WindowArchetype::regular:
      window_style |= WS_OVERLAPPEDWINDOW;
      break;
    case WindowArchetype::floating_regular:
      // TODO
      break;
    case WindowArchetype::dialog:
      // TODO
      break;
    case WindowArchetype::satellite:
      // TODO
      break;
    case WindowArchetype::popup:
      // TODO
      break;
    case WindowArchetype::tip:
      // TODO
      break;
    default:
      std::cerr << "Unhandled window archetype: " << static_cast<int>(archetype)
                << "\n";
      std::abort();
  }

  // Window rectangle in physical coordinates.
  // Default positioning values (CW_USEDEFAULT) are used.
  auto const window_rect{[&]() -> WindowRectangle {
    auto const window_size{GetWindowSizeForClientSize(
        client_size, window_style, extended_window_style, nullptr)};
    return {{CW_USEDEFAULT, CW_USEDEFAULT}, window_size};
  }()};

  if (!IsClassRegistered(kWindowClassName)) {
    auto const idi_app_icon{101};
    WNDCLASSEX window_class{};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32Window::WndProc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = GetModuleHandle(nullptr);
    window_class.hIcon =
        LoadIcon(window_class.hInstance, MAKEINTRESOURCE(idi_app_icon));
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    window_class.hbrBackground = 0;
    window_class.lpszMenuName = nullptr;
    window_class.lpszClassName = kWindowClassName;
    window_class.hIconSm = nullptr;

    RegisterClassEx(&window_class);
  }

  window_handle_ = win32_->CreateWindowEx(
      extended_window_style, kWindowClassName, title.c_str(), window_style,
      window_rect.top_left.x, window_rect.top_left.y, window_rect.size.width,
      window_rect.size.height, nullptr, nullptr, GetModuleHandle(nullptr),
      this);

  if (!window_handle_) {
    auto const error_message{GetLastErrorAsString()};
    std::cerr << "Cannot create window due to a CreateWindowEx error: "
              << error_message.c_str() << '\n';
    return false;
  }

  // Adjust the window position so its origin aligns with the top-left corner
  // of the window frame, not the window rectangle (which includes the
  // drop-shadow). This adjustment must be done post-creation since the frame
  // rectangle is only available after the window has been created.
  RECT frame_rc;
  DwmGetWindowAttribute(window_handle_, DWMWA_EXTENDED_FRAME_BOUNDS, &frame_rc,
                        sizeof(frame_rc));
  RECT window_rc;
  GetWindowRect(window_handle_, &window_rc);
  auto const left_dropshadow_width{frame_rc.left - window_rc.left};
  auto const top_dropshadow_height{window_rc.top - frame_rc.top};
  SetWindowPos(window_handle_, nullptr, window_rc.left - left_dropshadow_width,
               window_rc.top - top_dropshadow_height, 0, 0,
               SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

  UpdateTheme(window_handle_);

  gActiveWindowCount++;

  ShowWindow(window_handle_, SW_SHOW);

  return OnCreate();
}

void Win32Window::Destroy() {
  OnDestroy();
}

void Win32Window::SetChildContent(HWND content) {
  child_content_ = content;
  SetParent(content, window_handle_);
  auto const client_rect{GetClientArea()};

  MoveWindow(content, client_rect.left, client_rect.top,
             client_rect.right - client_rect.left,
             client_rect.bottom - client_rect.top, true);

  SetFocus(child_content_);
}

auto Win32Window::MessageHandler(HWND hwnd,
                                 UINT message,
                                 WPARAM wparam,
                                 LPARAM lparam) -> LRESULT {
  switch (message) {
    case WM_DESTROY:
      Destroy();
      if (quit_on_close_) {
        PostQuitMessage(0);
      }
      return 0;

    case WM_DPICHANGED: {
      auto* const new_scaled_window_rect{reinterpret_cast<RECT*>(lparam)};
      auto const width{new_scaled_window_rect->right -
                       new_scaled_window_rect->left};
      auto const height{new_scaled_window_rect->bottom -
                        new_scaled_window_rect->top};
      SetWindowPos(hwnd, nullptr, new_scaled_window_rect->left,
                   new_scaled_window_rect->top, width, height,
                   SWP_NOZORDER | SWP_NOACTIVATE);
      return 0;
    }
    case WM_SIZE: {
      if (child_content_ != nullptr) {
        // Resize and reposition the child content window
        auto const client_rect{GetClientArea()};
        MoveWindow(child_content_, client_rect.left, client_rect.top,
                   client_rect.right - client_rect.left,
                   client_rect.bottom - client_rect.top, TRUE);
      }
      return 0;
    }

    case WM_ACTIVATE:
      if (child_content_ != nullptr) {
        SetFocus(child_content_);
      }
      return 0;

    case WM_MOUSEACTIVATE:
      if (child_content_ != nullptr) {
        SetFocus(child_content_);
      }
      return MA_ACTIVATE;

    case WM_DWMCOLORIZATIONCOLORCHANGED:
      UpdateTheme(hwnd);
      return 0;

    default:
      break;
  }

  return DefWindowProc(window_handle_, message, wparam, lparam);
}

auto Win32Window::OnCreate() -> bool {
  // No-op; provided for subclasses.
  return true;
}

void Win32Window::OnDestroy() {}

// static
auto CALLBACK Win32Window::WndProc(HWND hwnd,
                                   UINT message,
                                   WPARAM wparam,
                                   LPARAM lparam) -> LRESULT {
  if (message == WM_NCCREATE) {
    auto* const create_struct{reinterpret_cast<CREATESTRUCT*>(lparam)};
    SetWindowLongPtr(hwnd, GWLP_USERDATA,
                     reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
    auto* const window{
        static_cast<Win32Window*>(create_struct->lpCreateParams)};
    window->window_handle_ = hwnd;

    EnableFullDpiSupportIfAvailable(hwnd);
    EnableTransparentWindowBackground(hwnd);
  } else if (auto* const window{GetThisFromHandle(hwnd)}) {
    return FlutterWindowController::GetInstance().MessageHandler(
        hwnd, message, wparam, lparam);
  }

  return DefWindowProc(hwnd, message, wparam, lparam);
}

}  // namespace flutter
