// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_BINDING_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_BINDING_HANDLER_H_

#include <windows.h>

#include <string>
#include <variant>
#include <vector>

#include "flutter/shell/platform/windows/public/flutter_windows.h"

namespace flutter {

class WindowBindingHandlerDelegate;
class FlutterWindowsView;

// Structure containing physical bounds of a Window or Screen
struct PhysicalBounds {
  /// Physical horizontal location of the left side of the window.
  int64_t left;
  /// Physical vertical location of the top of the window.
  int64_t top;
  /// Physical width of the window.
  size_t width;
  /// Physical height of the window.
  size_t height;
};

using WindowsRenderTarget = std::variant<
    /*winrt::Windows::UI::Composition::SpriteVisual, */ HWND>;

// Abstract class for binding Windows platform windows to Flutter views.
class WindowBindingHandler {
 public:
  virtual ~WindowBindingHandler() = default;

  // Sets the delegate used to communicate state changes from window to view
  // such as key presses, mouse position updates etc.
  virtual void SetView(WindowBindingHandlerDelegate* view) = 0;

  // Returns a valid WindowsRenderTarget representing the backing
  // window.
  virtual WindowsRenderTarget GetRenderTarget() = 0;

  // Gets the current DPI value for the window.
  virtual float GetDpiScale() = 0;

  // Returns the metrics of the backing window in physical pixels.
  virtual PhysicalBounds GetWindowBounds() = 0;

  // Returns the metrics of all the screens connected to the device, in physical
  // pixels.
  virtual std::vector<PhysicalBounds> GetScreenBounds() = 0;

  // Sets the cursor that should be used when the mouse is over the Flutter
  // content. See mouse_cursor.dart for the values and meanings of cursor_name.
  virtual void UpdateFlutterCursor(const std::string& cursor_name) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_BINDING_HANDLER_H_
