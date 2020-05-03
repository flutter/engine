#include "include/flutter/win32_flutter_window_pub.h"

#include <chrono>

#include "include/flutter/win32_flutter_view.h"

namespace flutter {

winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget
CreateDesktopWindowTarget(
    winrt::Windows::UI::Composition::Compositor const& compositor,
    HWND window) {
  namespace abi = ABI::Windows::UI::Composition::Desktop;

  // attempt workaround for https://bugs.llvm.org/show_bug.cgi?id=38490

  //auto interop = compositor.as<abi::ICompositorDesktopInterop>();
  winrt::com_ptr<abi::ICompositorDesktopInterop> interop;
  //winrt::check_hresult(compositor.as(__uuidof(interop), interop.put_void()));
  winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target{nullptr};
  /*
  winrt::check_hresult(interop->CreateDesktopWindowTarget(
      window, true,
      reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(target))));
  return target;*/
  return nullptr;
}

// The Windows DPI system is based on this
// constant for machines running at 100% scaling.
constexpr int base_dpi = 96;

Win32FlutterWindowPub::Win32FlutterWindowPub(
    int width,
    int height,
    winrt::Windows::UI::Composition::Compositor const& compositor)
    : compositor_(compositor), flutter_view_(nullptr) {
  Win32WindowPub::InitializeChild("Win32FlutterView", width, height);

    // Create Window target, set visual returned from the engine as the root
  target_ =
      CreateDesktopWindowTarget(compositor_, Win32WindowPub::GetWindowHandle());

  host_visual_ = compositor_.CreateSpriteVisual();
  target_.Root(host_visual_);
}

Win32FlutterWindowPub::Win32FlutterWindowPub(int width, int height)
    : compositor_(nullptr), flutter_view_(nullptr) {
  Win32WindowPub::InitializeChild("Win32FlutterView", width, height);
}

void Win32FlutterWindowPub::SetView(Win32FlutterView* view) {
  flutter_view_ = view;
}

Win32FlutterWindowPub::~Win32FlutterWindowPub() {}

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

void Win32FlutterWindowPub::OnDpiScale(unsigned int dpi){
  if (flutter_view_ != nullptr) {
    flutter_view_->SendWindowMetrics(
        GetCurrentWidth(), GetCurrentHeight(),
        static_cast<double>(GetCurrentDPI()) / base_dpi);
  }
};

// When DesktopWindow notifies that a WM_Size message has come in
// lets FlutterEngine know about the new size.
void Win32FlutterWindowPub::OnResize(unsigned int width, unsigned int height) {
  if (flutter_view_ != nullptr) {
    flutter_view_->SendWindowMetrics(
        GetCurrentWidth(), GetCurrentHeight(),
        static_cast<double>(GetCurrentDPI()) / base_dpi);
  }
}

void Win32FlutterWindowPub::OnPointerMove(double x, double y) {
    SendPointerMove(x, y);
}

void Win32FlutterWindowPub::OnPointerDown(double x, double y, UINT button) {
    uint64_t flutter_button = ConvertWinButtonToFlutterButton(button);
    if (flutter_button != 0) {
      SendPointerDown(x, y, flutter_button);
    }
}

void Win32FlutterWindowPub::OnPointerUp(double x, double y, UINT button) {
     uint64_t flutter_button = ConvertWinButtonToFlutterButton(button);
     if (flutter_button != 0) {
       SendPointerUp(x, y, flutter_button);
     }
}

void Win32FlutterWindowPub::OnPointerLeave() {
     SendPointerLeave();
}

void Win32FlutterWindowPub::OnText(const std::u16string& text) {
  flutter_view_->SendText(text.c_str(), text.length());
}

void Win32FlutterWindowPub::OnKey(int key,
                                  int scancode,
                                  int action,
                                  char32_t character) {
  flutter_view_->SendKey(key, scancode, action, character);
}

void Win32FlutterWindowPub::OnScroll(double delta_x, double delta_y) {
  POINT position;
  GetEventLocationFromCursorPosition(position);
  flutter_view_->SendScroll(position.x, position.y, delta_x, delta_y);
}

void Win32FlutterWindowPub::OnFontChange() {
  flutter_view_->SendFontChange();
}

// Sends new size information to FlutterEngine.
void Win32FlutterWindowPub::SendWindowMetrics() {
  flutter_view_->SendWindowMetrics(
      GetCurrentWidth(), GetCurrentHeight(),
      static_cast<double>(GetCurrentDPI()) / base_dpi);
}

// Updates |event_data| with the current location of the mouse cursor.
void Win32FlutterWindowPub::GetEventLocationFromCursorPosition(
    POINT &location
    ) {
    GetCursorPos(&location);

  ScreenToClient(GetWindowHandle(), &location);
}

 void Win32FlutterWindowPub::SendPointerMove(double x, double y) {
   flutter_view_->SendPointerMove(x, y);
 }

 void Win32FlutterWindowPub::SendPointerDown(double x, double y, uint64_t btn) {
  flutter_view_->SendPointerDown(x, y, btn);
 }

 void Win32FlutterWindowPub::SendPointerUp(double x, double y, uint64_t btn) {
   flutter_view_->SendPointerUp(x, y, btn);
 }

 void Win32FlutterWindowPub::SendPointerLeave() {
   flutter_view_->SendPointerLeave();
 }

}  // namespace flutter
