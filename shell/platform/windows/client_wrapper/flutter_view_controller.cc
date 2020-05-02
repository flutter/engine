// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "include/flutter/flutter_view_controller.h"
#include <dispatcherqueue.h>
#include <Unknwn.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>


using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;

#include <algorithm>
#include <iostream>

namespace flutter {

auto CreateDispatcherQueueController() {
  namespace abi = ABI::Windows::System;

  DispatcherQueueOptions options{sizeof(DispatcherQueueOptions),
                                 DQTYPE_THREAD_CURRENT, DQTAT_COM_STA};

  Windows::System::DispatcherQueueController controller{nullptr};
  check_hresult(CreateDispatcherQueueController(
      options, reinterpret_cast<abi::IDispatcherQueueController**>(
                   put_abi(controller))));
  return controller;
}

// Compositor overload
FlutterViewController::FlutterViewController(int width,
                                             int height,
                                             const DartProject& project,
                                             void* compositor) {
  if (compositor == nullptr) {
    std::cerr << "Failed to create view controller: compositor can't be null." << std::endl;
    return;
  }
  
  std::vector<const char*> switches;
  std::transform(
      project.engine_switches().begin(), project.engine_switches().end(),
      std::back_inserter(switches),
      [](const std::string& arg) -> const char* { return arg.c_str(); });
  size_t switch_count = switches.size();

  FlutterDesktopEngineProperties properties = {};
  properties.assets_path = project.assets_path().c_str();
  properties.icu_data_path = project.icu_data_path().c_str();
  properties.switches = switch_count > 0 ? switches.data() : nullptr;
  properties.switches_count = switch_count;

  child_window_ =
      std::make_unique<Win32FlutterWindowPub>(width, height, compositor_);

  controller_ = V2FlutterDesktopCreateViewControllerComposition(
      width, height, properties,
      static_cast<void*>(winrt::get_abi(compositor_)),child_window_.get());
  if (!controller_) {
    std::cerr << "Failed to create view controller." << std::endl;
    return;
  }
  view_ = std::make_unique<FlutterView>(FlutterDesktopGetView(controller_));

  child_window_->SetViewComposition(view_.get());
}

// Window overload
FlutterViewController::FlutterViewController(int width,
                                             int height,
                                             const DartProject& project,
                                             HWND parentwindow) {
  std::vector<const char*> switches;
  std::transform(
      project.engine_switches().begin(), project.engine_switches().end(),
      std::back_inserter(switches),
      [](const std::string& arg) -> const char* { return arg.c_str(); });
  size_t switch_count = switches.size();

  FlutterDesktopEngineProperties properties = {};
  properties.assets_path = project.assets_path().c_str();
  properties.icu_data_path = project.icu_data_path().c_str();
  properties.switches = switch_count > 0 ? switches.data() : nullptr;
  properties.switches_count = switch_count;

  child_window_ =
      std::make_unique<Win32FlutterWindowPub>(width, height);

  controller_ = V2CreateViewControllerWindow(
      width, height, properties,
      child_window_.get(),child_window_->GetWindowHandle());
  if (!controller_) {
    std::cerr << "Failed to create view controller." << std::endl;
    return;
  }
  view_ = std::make_unique<FlutterView>(FlutterDesktopGetView(controller_));
  child_window_->SetView(view_.get());
}

FlutterViewController::~FlutterViewController() {
  if (controller_) {
    FlutterDesktopDestroyViewController(controller_);
  }
}

std::chrono::nanoseconds FlutterViewController::ProcessMessages() {
  return std::chrono::nanoseconds(FlutterDesktopProcessMessages(controller_));
}

FlutterDesktopPluginRegistrarRef FlutterViewController::GetRegistrarForPlugin(
    const std::string& plugin_name) {
  if (!controller_) {
    std::cerr << "Cannot get plugin registrar without a window; call "
                 "CreateWindow first."
              << std::endl;
    return nullptr;
  }
  return FlutterDesktopGetPluginRegistrar(controller_, plugin_name.c_str());
}

}  // namespace flutter
