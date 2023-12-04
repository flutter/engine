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
  FlutterWindowsViewController(std::unique_ptr<FlutterWindowsEngine> engine,
                               std::unique_ptr<FlutterWindowsView> view)
      : engine_(std::move(engine)), view_(std::move(view)) {}

  FlutterWindowsEngine* engine() { return engine_.get(); }
  FlutterWindowsView* view() { return view_.get(); }

 private:
  std::unique_ptr<FlutterWindowsEngine> engine_;
  std::unique_ptr<FlutterWindowsView> view_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_VIEW_CONTROLLER_H_
