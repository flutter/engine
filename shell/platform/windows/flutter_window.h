// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_

#include <windowsx.h>
#include <string>
#include <vector>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/angle_surface_manager.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "flutter/shell/platform/windows/win32_window.h"
#include "flutter/shell/platform/common/cpp/incoming_message_dispatcher.h"
#include "flutter/shell/platform/windows/key_event_handler.h"
#include "flutter/shell/platform/windows/keyboard_hook_handler.h"
#include "flutter/shell/platform/windows/platform_handler.h"
#include "flutter/shell/platform/windows/text_input_plugin.h"
#include "flutter/shell/platform/windows/window_state.h"

namespace flutter {

constexpr int base_dpi = 96;  // the Windows DPI system is based on this
                              // constant for machines running at 100% scaling.

// A win32 flutter window.  In the future, there will likely be a
// CoreWindow-based FlutterWindow as well.  At the point may make sense to
// dependency inject the native window rather than inherit.
class FlutterWindow : public Win32Window {
 public:
  FlutterWindow();
  FlutterWindow(const char* title,
                const int x,
                const int y,
                const int width,
                const int height) noexcept;
  ~FlutterWindow();

  void FlutterMessageLoop();

  void OnResize(unsigned int width, unsigned int height) override;
  void OnResize();
  void OnDpiScale(unsigned int dpi) override;
  void OnPointerMove(double x, double y) override;
  void OnPointerDown(double x, double y) override;
  void OnPointerUp(double x, double y) override;
  void OnChar(unsigned int code_point) override;
  void OnKey(int key, int scancode, int action, int mods) override;
  void OnScroll(double delta_x, double delta_y) override;

  FlutterDesktopWindowControllerRef SetState(FlutterEngine state);
  FlutterDesktopPluginRegistrarRef GetRegistrar();

  void HandlePlatformMessage(const FlutterPlatformMessage*);

  void CreateRenderSurface();
  void DestroyRenderSurface();

  bool ClearContext();
  bool MakeCurrent();
  bool SwapBuffers();

 private:
  void SendWindowMetrics();
  void SendPointerMove(double x, double y);
  void SendPointerDown(double x, double y);
  void SendPointerUp(double x, double y);
  void SendChar(unsigned int code_point);
  void SendKey(int key, int scancode, int action, int mods);
  void SendScroll(double delta_x, double delta_y);
  void SetEventLocationFromCursorPosition(FlutterPointerEvent* event_data);
  void SetEventPhaseFromCursorButtonState(FlutterPointerEvent* event_data);
  void SendPointerEventWithData(const FlutterPointerEvent& event_data);

  std::unique_ptr<AngleSurfaceManager> surface_manager{nullptr};
  EGLSurface render_surface{EGL_NO_SURFACE};

  // state of the mouse button
  bool pointer_is_down_ = false;

  // The handle to the Flutter engine instance.
  FlutterEngine engine_{nullptr};

  // Whether or not to track mouse movements to send kHover events.
  bool hover_tracking_is_enabled_ = false;

  // Whether or not the pointer has been added (or if tracking is enabled, has
  // been added since it was last removed).
  bool pointer_currently_added_ = false;

  // The window handle given to API clients.
  std::unique_ptr<FlutterDesktopWindow> window_wrapper_;

  // The plugin registrar handle given to API clients.
  std::unique_ptr<FlutterDesktopPluginRegistrar> plugin_registrar_;

  // Message dispatch manager for messages from the Flutter engine.
  std::unique_ptr<flutter::IncomingMessageDispatcher> message_dispatcher_;

  // The plugin registrar managing internal plugins.
  std::unique_ptr<flutter::PluginRegistrar> internal_plugin_registrar_;

  // Handlers for keyboard events from Windows.
  std::vector<std::unique_ptr<flutter::KeyboardHookHandler>>
      keyboard_hook_handlers_;

  // Handler for the flutter/platform channel.
  std::unique_ptr<flutter::PlatformHandler> platform_handler_;

  // should we forword input messages or not
  bool process_events_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_