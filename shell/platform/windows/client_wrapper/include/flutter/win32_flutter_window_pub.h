// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_

#include <windowsx.h>

#include <iostream>
#include <string>
#include <vector>

#include <Unknwn.h>
//#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>
#include <windows.ui.composition.h>
#include <windows.ui.composition.desktop.h>

#include <flutter_windows.h>
#include "win32_window_pub.h"

namespace flutter {

    //TODO these should not be copied and pasted.. confirm with stuart per constants in flutter_windows.h
    typedef enum {
  kFlutterPointerButtonMousePrimary = 1 << 0,
  kFlutterPointerButtonMouseSecondary = 1 << 1,
  kFlutterPointerButtonMouseMiddle = 1 << 2,
  kFlutterPointerButtonMouseBack = 1 << 3,
  kFlutterPointerButtonMouseForward = 1 << 4,
  /// If a mouse has more than five buttons, send higher bit shifted values
  /// corresponding to the button number: 1 << 5 for the 6th, etc.
} FlutterPointerMouseButtons;

//Forward declare to avoid circular references
class FlutterView;

// A win32 flutter child window used as implementatin for flutter view.  In the
// future, there will likely be a CoreWindow-based FlutterWindow as well.  At
// the point may make sense to dependency inject the native window rather than
// inherit.
class Win32FlutterWindowPub : public Win32WindowPub {
 public:
  // Create flutter Window for use as child window
  Win32FlutterWindowPub(
      int width,
      int height,
      winrt::Windows::UI::Composition::Compositor const& compositor);

    // Create flutter Window for use as child window
  Win32FlutterWindowPub(
      int width,
      int height);

  virtual ~Win32FlutterWindowPub();

  //TODO
  void SetView(FlutterView* view);

  // |Win32WindowPub|
  void OnDpiScale(unsigned int dpi) override;

  // |Win32WindowPub|
  void OnResize(unsigned int width, unsigned int height) override;

  // |Win32WindowPub|
  void OnPointerMove(double x, double y) override;

  // |Win32WindowPub|
  void OnPointerDown(double x, double y, UINT button) override;

  // |Win32WindowPub|
  void OnPointerUp(double x, double y, UINT button) override;

  // |Win32WindowPub|
  void OnPointerLeave() override;

  // |Win32WindowPub|
  void OnText(const std::u16string& text) override;

  // |Win32WindowPub|
  void OnKey(int key, int scancode, int action, char32_t character) override;

  // |Win32WindowPub|
  void OnScroll(double delta_x, double delta_y) override;

  // |Win32WindowPub|
  void OnFontChange() override;

  winrt::Windows::UI::Composition::Compositor compositor_;

  winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target_{nullptr};

  winrt::Windows::UI::Composition::SpriteVisual host_visual_{nullptr};

  // Sends a window metrics update to the Flutter engine using current window
  // dimensions in physical
  void SendWindowMetrics();

  private:
  FlutterView* flutter_view_;
 // // Destroy current rendering surface if one has been allocated.
 // void DestroyRenderSurface();

 // // Reports a mouse movement to Flutter engine.
  void SendPointerMove(double x, double y);

 // // Reports mouse press to Flutter engine.
  void SendPointerDown(double x, double y, uint64_t btn);

 // // Reports mouse release to Flutter engine.
  void SendPointerUp(double x, double y, uint64_t btn);

 // // Reports mouse left the window client area.
 // //
 // // Win32 api doesn't have "mouse enter" event. Therefore, there is no
 // // SendPointerEnter method. A mouse enter event is tracked then the "move"
 // // event is called.
  void SendPointerLeave();

 // // Updates |event_data| with the current location of the mouse cursor.
  void GetEventLocationFromCursorPosition(POINT& location);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_
