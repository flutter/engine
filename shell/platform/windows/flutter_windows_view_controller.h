// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_VIEW_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_VIEW_CONTROLLER_H_

#include <memory>

#include "flutter_windows_engine.h"
#include "flutter_windows_view.h"

namespace flutter {

/// Controls a view that displays Flutter content.
class FlutterWindowsViewController {
 public:
  /// Create a view controller that owns a view.
  explicit FlutterWindowsViewController(
      std::unique_ptr<FlutterWindowsView> view)
      : view_(std::move(view)) {}

  /// Create a view controller that owns both a view and an engine.
  ///
  /// This is used for backwards compatibility scenarios where
  /// destroying the view controller also destroyed the engine.
  FlutterWindowsViewController(std::unique_ptr<FlutterWindowsEngine> engine,
                               std::unique_ptr<FlutterWindowsView> view)
      : engine_(std::move(engine)), view_(std::move(view)) {}

  /// The engine that manages this view controller.
  FlutterWindowsEngine* engine() const { return view_->GetEngine(); }

  /// The view that is managed by this view controller.
  FlutterWindowsView* view() const { return view_.get(); }

 private:
  // The engine owned by this view controller, if any.
  // This is only used if the view controller was created
  // using |FlutterDesktopViewControllerCreate| as that takes
  // ownership of the engine. View controllers created using
  // |FlutterDesktopEngineCreateViewController| do not take
  // ownership of the engine and do not set this.
  std::unique_ptr<FlutterWindowsEngine> engine_;

  std::unique_ptr<FlutterWindowsView> view_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_VIEW_CONTROLLER_H_
