// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "window_top_level_message_handler.h"

#include <WinUser.h>

#include "flutter/shell/platform/windows/flutter_windows_engine.h"

namespace flutter {

static constexpr char kExitTypeCancelable[] = "cancelable";

WindowTopLevelMessageHandler::WindowTopLevelMessageHandler(FlutterWindowsEngine& engine) : engine_(engine) {}

WindowTopLevelMessageHandler::~WindowTopLevelMessageHandler() {}

void WindowTopLevelMessageHandler::Quit(int64_t exit_code) {
  PostQuitMessage(exit_code);
}

bool WindowTopLevelMessageHandler::WindowProc(HWND hwnd, UINT msg, WPARAM wpar, LPARAM lpar, LRESULT* result) {
  switch (msg) {
    case WM_CLOSE:
      engine_.RequestApplicationQuit(kExitTypeCancelable, wpar);
      return true;
  }
  return false;
}

}
