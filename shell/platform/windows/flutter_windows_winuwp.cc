// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/public/flutter_windows.h"

#include <io.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include <winrt/Windows.ApplicationModel.Activation.h>
#include "winrt/Windows.ApplicationModel.Core.h"

#include "flutter/shell/platform/common/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/incoming_message_dispatcher.h"
#include "flutter/shell/platform/windows/flutter_window_winuwp.h"  // nogncheck

// Returns the engine corresponding to the given opaque API handle.
static flutter::FlutterWindowsEngine* EngineFromHandle(
    FlutterDesktopEngineRef ref) {
  return reinterpret_cast<flutter::FlutterWindowsEngine*>(ref);
}

std::vector<std::string> split(std::string const s) {
  size_t pos_start = 0, pos_end = 1;
  std::string token;
  std::vector<std::string> result;

  while ((pos_end = s.find(",", pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + 1;
    result.push_back(token);
  }

  result.push_back(s.substr(pos_start));
  return (std::move(result));
}

FlutterDesktopViewControllerRef
FlutterDesktopViewControllerCreateFromCoreWindow(
    ABI::Windows::UI::Core::CoreWindow* window,
    ABI::Windows::ApplicationModel::Activation::IActivatedEventArgs* args,
    FlutterDesktopEngineRef engine) {
  std::unique_ptr<flutter::WindowBindingHandler> window_wrapper =
      std::make_unique<flutter::FlutterWindowWinUWP>(window);

  auto state = std::make_unique<FlutterDesktopViewControllerState>();
  state->view =
      std::make_unique<flutter::FlutterWindowsView>(std::move(window_wrapper));
  // Take ownership of the engine, starting it if necessary.
  state->view->SetEngine(
      std::unique_ptr<flutter::FlutterWindowsEngine>(EngineFromHandle(engine)));
  state->view->CreateRenderSurface();

  winrt::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs
      arg_interface{nullptr};
  winrt::copy_from_abi(arg_interface, args);

  std::vector<std::string> engine_switches;

  if (arg_interface != nullptr) {
    std::string launchargs = winrt::to_string(arg_interface.Arguments());
    if (!launchargs.empty()) {
      engine_switches = split(launchargs);
    }
  }

  state->view->GetEngine()->SetSwitches(engine_switches);

  if (!state->view->GetEngine()->running()) {
    if (!state->view->GetEngine()->RunWithEntrypoint(nullptr)) {
      return nullptr;
    }
  }

  // Must happen after engine is running.
  state->view->SendInitialBounds();
  return state.release();
}
