// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/client_wrapper/testing/stub_flutter_windows_api.h"

static flutter::testing::StubFlutterWindowsApi* s_stub_implementation;

namespace flutter {
namespace testing {

// static
void StubFlutterWindowsApi::SetTestStub(StubFlutterWindowsApi* stub) {
  s_stub_implementation = stub;
}

// static
StubFlutterWindowsApi* StubFlutterWindowsApi::GetTestStub() {
  return s_stub_implementation;
}

ScopedStubFlutterWindowsApi::ScopedStubFlutterWindowsApi(
    std::unique_ptr<StubFlutterWindowsApi> stub)
    : stub_(std::move(stub)) {
  previous_stub_ = StubFlutterWindowsApi::GetTestStub();
  StubFlutterWindowsApi::SetTestStub(stub_.get());
}

ScopedStubFlutterWindowsApi::~ScopedStubFlutterWindowsApi() {
  StubFlutterWindowsApi::SetTestStub(previous_stub_);
}

}  // namespace testing
}  // namespace flutter

// Forwarding dummy implementations of the C API.

FlutterDesktopViewControllerRef V2CreateViewControllerWindow(
    int width,
    int height,
    const FlutterDesktopEngineProperties& engine_properties,
    void* hostwindow,
    HWND windowrendertarget) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2CreateViewControllerWindow(width, height,
                                                       engine_properties, hostwindow, windowrendertarget);
  }
  return nullptr;
}

FlutterDesktopViewControllerRef V2FlutterDesktopCreateViewControllerComposition(
    int width,
    int height,
    const FlutterDesktopEngineProperties& engine_properties,
    void* compositor,
    void* hostwindow) {
  if (s_stub_implementation) {
    return s_stub_implementation
        ->V2FlutterDesktopCreateViewControllerComposition(width, height,
                                                       engine_properties, compositor, hostwindow);
  }
  return nullptr;
}

void* V2FlutterDesktopViewGetVisual(FlutterDesktopViewRef view) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopViewGetVisual(view);
  }
  return nullptr;
}

void V2FlutterDesktopSendWindowMetrics(
    FlutterDesktopViewRef view,
    size_t width,
    size_t height,
    double dpiScale) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendWindowMetrics(view, width, height, dpiScale);
  }
}

void V2FlutterDesktopSendPointerMove(FlutterDesktopViewRef view,
                                                    double x,
                                     double y) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendPointerMove(
            view, x, y);
  }
}

void V2FlutterDesktopSendPointerDown(
    FlutterDesktopViewRef view,
    double x,
    double y,
    uint64_t btn) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendPointerDown(view, x, y, btn);
  }
}

void V2FlutterDesktopSendPointerUp(
    FlutterDesktopViewRef view,
    double x,
    double y,
    uint64_t btn) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendPointerUp(view, x, y,
                                                                  btn);
  }
}

void V2FlutterDesktopSendPointerLeave(FlutterDesktopViewRef view) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendPointerLeave(view);
  }
}

void V2FlutterDesktopSendScroll(FlutterDesktopViewRef view,
                                               double x,
                                               double y,
                                               double delta_x,
                                double delta_y) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendScroll(view, x, y,
                                                                  delta_x, delta_y);
  }
}

void V2FlutterDesktopSendFontChange(FlutterDesktopViewRef view) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendFontChange(view);
  }
}

void V2FlutterDesktopSendText(FlutterDesktopViewRef view,
                                             const char16_t* code_point,
                              size_t size) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendText(view, code_point,
                                                                  size);
  }
}

void V2FlutterDesktopSendKey(FlutterDesktopViewRef view,
                                            int key,
                                            int scancode,
                                            int action,
                             char32_t character) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopSendKey(view, key, scancode, action, character
                                                           );
  }
}



void FlutterDesktopDestroyViewController(
    FlutterDesktopViewControllerRef controller) {
  if (s_stub_implementation) {
    s_stub_implementation->DestroyViewController();
  }
}

FlutterDesktopViewRef FlutterDesktopGetView(
    FlutterDesktopViewControllerRef controller) {
  // The stub ignores this, so just return an arbitrary non-zero value.
  return reinterpret_cast<FlutterDesktopViewRef>(1);
}

uint64_t FlutterDesktopProcessMessages(
    FlutterDesktopViewControllerRef controller) {
  if (s_stub_implementation) {
    return s_stub_implementation->ProcessMessages();
  }
  return 0;
}

HWND FlutterDesktopViewGetHWND(FlutterDesktopViewRef controller) {
  if (s_stub_implementation) {
    return s_stub_implementation->ViewGetHWND();
  }
  return reinterpret_cast<HWND>(-1);
}

void* V2FlutterDesktopGetExternalWindow(FlutterDesktopViewRef view) {
  if (s_stub_implementation) {
    return s_stub_implementation->V2FlutterDesktopGetExternalWindow(view);
  }
  return static_cast<HWND>(nullptr);
}

FlutterDesktopEngineRef FlutterDesktopRunEngine(
    const FlutterDesktopEngineProperties& engine_properties) {
  if (s_stub_implementation) {
    return s_stub_implementation->RunEngine(engine_properties);
  }
  return nullptr;
}

bool FlutterDesktopShutDownEngine(FlutterDesktopEngineRef engine_ref) {
  if (s_stub_implementation) {
    return s_stub_implementation->ShutDownEngine();
  }
  return true;
}

FlutterDesktopPluginRegistrarRef FlutterDesktopGetPluginRegistrar(
    FlutterDesktopViewControllerRef controller,
    const char* plugin_name) {
  // The stub ignores this, so just return an arbitrary non-zero value.
  return reinterpret_cast<FlutterDesktopPluginRegistrarRef>(1);
}

FlutterDesktopViewRef FlutterDesktopRegistrarGetView(
    FlutterDesktopPluginRegistrarRef controller) {
  // The stub ignores this, so just return an arbitrary non-zero value.
  return reinterpret_cast<FlutterDesktopViewRef>(1);
}

