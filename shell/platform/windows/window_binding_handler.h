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

// Represents the type of a WindowsRenderTarget
enum WindowsRenderTargetType { HWND /*, Visual*/};

struct WindowsRenderTarget {

// Creates a new HWND backed render target from window
  static WindowsRenderTarget CreateForHWND(::HWND window) {
    WindowsRenderTarget wrt;
    wrt.type_ = WindowsRenderTargetType::HWND;
    wrt.render_target_ = window;
    return wrt;
  }

  // Returns the type of instance
  WindowsRenderTargetType GetType() { return type_; }

// Returns the physical window for HWND backed instances
  ::HWND GetWindowHandle() { return std::get<::HWND>(render_target_); }

 private:
// prevent external construction
  WindowsRenderTarget() : render_target_(static_cast<::HWND>(0)) {}

  // type of instance
  WindowsRenderTargetType type_;

  // win32 WindowsRenderTarget instances will by HWND backed, in the future
  // UWP instances will be backed by a SpriteVisual
  std::variant</*winrt::Windows::UI::Composition::SpriteVisual, */ ::HWND>
      render_target_;
};

// Abstract class for binding Windows OS Windows to Flutter views.
class FlutterWindowBindingHandler {
 public:
  virtual ~FlutterWindowBindingHandler() = default;

  // Caller provides a view for implementor to use to communicate state changes.
  virtual void SetView(FlutterWindowsView* view) = 0;

  // Implementor returns a valid WindowsRenderTarget representing the backing window.
  virtual WindowsRenderTarget GetRenderTarget() = 0;

  // Implementor returns the scale factor for the backing window.
  virtual float GetDpiScale() = 0;

  // Implementor returns the width of the backing window in physical pixels.
  virtual float GetPhysicalWidth() = 0;

  // Implementor returns the height of the backing window in physical pixels.
  virtual float GetPhysicalHeight() = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_BINDING_HANDLER_H_
