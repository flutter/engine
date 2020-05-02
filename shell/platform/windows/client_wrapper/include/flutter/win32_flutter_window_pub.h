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
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
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

  /*static FlutterDesktopViewControllerRef CreateWin32FlutterWindow(int width,
                                                                  int height);*/

  void SetView(FlutterView* view);

  void SetViewComposition(FlutterView* view);

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

  winrt::Windows::UI::Composition::Compositor const& compositor_;

  winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target_{nullptr};

  //// Configures the window instance with an instance of a running Flutter engine
  //// returning a configured FlutterDesktopWindowControllerRef.
  //void SetState(FLUTTER_API_SYMBOL(FlutterEngine) state);

  //// Returns the currently configured Plugin Registrar.
  //FlutterDesktopPluginRegistrarRef GetRegistrar();

  //// Callback passed to Flutter engine for notifying window of platform
  //// messages.
  //void HandlePlatformMessage(const FlutterPlatformMessage*);

  //// Create a surface for Flutter engine to render into.
  //void CreateRenderSurface();

  //// Callbacks for clearing context, settings context and swapping buffers.
  //bool ClearContext();
  //bool MakeCurrent();
  //bool MakeResourceCurrent();
  //bool SwapBuffers();

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

 // // Reports text input to Flutter engine.
 // void SendText(const std::u16string& text);

 // // Reports a raw keyboard message to Flutter engine.
 // void SendKey(int key, int scancode, int action, char32_t character);

 // // Updates |event_data| with the current location of the mouse cursor.
  void GetEventLocationFromCursorPosition(POINT& location);

 // // Set's |event_data|'s phase to either kMove or kHover depending on the
 // // current
 // // primary mouse button state.
 // void SetEventPhaseFromCursorButtonState(FlutterPointerEvent* event_data);

 // // Sends a pointer event to the Flutter engine based on givern data.  Since
 // // all input messages are passed in physical pixel values, no translation is
 // // needed before passing on to engine.
 // void SendPointerEventWithData(const FlutterPointerEvent& event_data);

 // std::unique_ptr<AngleSurfaceManager> surface_manager = nullptr;
 // EGLSurface render_surface = EGL_NO_SURFACE;

 // // state of the mouse button
 // bool pointer_is_down_ = false;

 // // The handle to the Flutter engine instance.
 // FLUTTER_API_SYMBOL(FlutterEngine) engine_ = nullptr;

 // // Whether or not to track mouse movements to send kHover events.
 // bool hover_tracking_is_enabled_ = false;

 // // Whether or not the pointer has been added (or if tracking is enabled, has
 // // been added since it was last removed).
 // bool pointer_currently_added_ = false;

 // // The window handle given to API clients.
 // std::unique_ptr<FlutterDesktopView> window_wrapper_;

 // // The plugin registrar handle given to API clients.
 // std::unique_ptr<FlutterDesktopPluginRegistrar> plugin_registrar_;

 // // Message dispatch manager for messages from the Flutter engine.
 // std::unique_ptr<flutter::IncomingMessageDispatcher> message_dispatcher_;

 // // The plugin registrar managing internal plugins.
 // std::unique_ptr<flutter::PluginRegistrar> internal_plugin_registrar_;

 // // Handlers for keyboard events from Windows.
 // std::vector<std::unique_ptr<flutter::KeyboardHookHandler>>
 //     keyboard_hook_handlers_;

 // // Handler for the flutter/platform channel.
 // std::unique_ptr<flutter::PlatformHandler> platform_handler_;

 // // should we forword input messages or not
 // bool process_events_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_
