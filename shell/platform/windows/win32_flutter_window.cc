#include "flutter/shell/platform/windows/win32_flutter_window.h"

#include <chrono>

namespace flutter {

// The Windows DPI system is based on this
// constant for machines running at 100% scaling.
constexpr int base_dpi = 96;

constexpr int kScrollOffsetMultiplier = 20;

Win32FlutterWindow::Win32FlutterWindow(int width, int height) : view_(nullptr) {
  Win32Window::InitializeChild("FLUTTERVIEW", width, height);
}

Win32FlutterWindow::~Win32FlutterWindow() {}

void Win32FlutterWindow::SetView(FlutterWindowsView* window) {
  view_ = window;
}

WindowsRenderTarget Win32FlutterWindow::GetRenderTarget() {
  return WindowsRenderTarget(GetWindowHandle());
}

float Win32FlutterWindow::GetDpiScale() {
  return static_cast<float>(GetCurrentDPI()) / static_cast<float>(base_dpi);
}

float Win32FlutterWindow::GetPhysicalWidth() {
  return GetCurrentWidth();
}

float Win32FlutterWindow::GetPhysicalHeight() {
  return GetCurrentHeight();
}

// Translates button codes from Win32 API to FlutterPointerMouseButtons.
static uint64_t ConvertWinButtonToFlutterButton(UINT button) {
  switch (button) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      return kFlutterPointerButtonMousePrimary;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      return kFlutterPointerButtonMouseSecondary;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      return kFlutterPointerButtonMouseMiddle;
    case XBUTTON1:
      return kFlutterPointerButtonMouseBack;
    case XBUTTON2:
      return kFlutterPointerButtonMouseForward;
  }
  std::cerr << "Mouse button not recognized: " << button << std::endl;
  return 0;
}

void Win32FlutterWindow::OnDpiScale(unsigned int dpi){};

// When DesktopWindow notifies that a WM_Size message has come in
// lets FlutterEngine know about the new size.
void Win32FlutterWindow::OnResize(unsigned int width, unsigned int height) {
  if (view_ != nullptr) {
    view_->OnWindowSizeChanged(width, height);
  }
}

void Win32FlutterWindow::OnPointerMove(double x, double y) {
  view_->OnPointerMove(x, y);
}

void Win32FlutterWindow::OnPointerDown(double x, double y, UINT button) {
  uint64_t flutter_button = ConvertWinButtonToFlutterButton(button);
  if (flutter_button != 0) {
    view_->OnPointerDown(
        x, y, static_cast<FlutterPointerMouseButtons>(flutter_button));
  }
}

void Win32FlutterWindow::OnPointerUp(double x, double y, UINT button) {
  uint64_t flutter_button = ConvertWinButtonToFlutterButton(button);
  if (flutter_button != 0) {
    view_->OnPointerUp(x, y,
                       static_cast<FlutterPointerMouseButtons>(flutter_button));
  }
}

void Win32FlutterWindow::OnPointerLeave() {
  view_->OnPointerLeave();
}

void Win32FlutterWindow::OnText(const std::u16string& text) {
  view_->OnText(text);
}

void Win32FlutterWindow::OnKey(int key,
                               int scancode,
                               int action,
                               char32_t character) {
  view_->OnKey(key, scancode, action, character);
}

void Win32FlutterWindow::OnScroll(double delta_x, double delta_y) {
  POINT point;
  GetCursorPos(&point);

  ScreenToClient(GetWindowHandle(), &point);
  view_->OnScroll(point.x, point.y, delta_x, delta_y, kScrollOffsetMultiplier);
}

void Win32FlutterWindow::OnFontChange() {
  view_->OnFontChange();
}

}  // namespace flutter
