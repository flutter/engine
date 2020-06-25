// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_BINDING_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_BINDING_HANDLER_H_

#include <string>
#include <variant>

#include <windows.h>

#include "flutter/shell/platform/windows/public/flutter_windows.h"

namespace flutter {

class FlutterWindowsView;

using WindowsRenderTarget = std::variant<
    /*winrt::Windows::UI::Composition::SpriteVisual, */ ::HWND>;

// Abstract class for binding Windows platform windows to Flutter views.
class FlutterWindowBindingHandler {
 public:
  virtual ~FlutterWindowBindingHandler() = default;

  // Sets the view used to communicate state changes from Window to view such
  // as key presses, mouse position updates etc.
  virtual void SetView(FlutterWindowsView* view) = 0;

  // Returns a valid WindowsRenderTarget representing the backing
  // window.

  virtual WindowsRenderTarget GetRenderTarget() = 0;

  // Returns the scale factor for the backing window.
  virtual float GetDpiScale() = 0;

  // Returns the width of the backing window in physical pixels.
  virtual float GetPhysicalWidth() = 0;

  // Returns the height of the backing window in physical pixels.
  virtual float GetPhysicalHeight() = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_BINDING_HANDLER_H_
