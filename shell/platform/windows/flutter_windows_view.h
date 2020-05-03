// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_

#include <windowsx.h>

#include <string>
#include <vector>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/cpp/incoming_message_dispatcher.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/angle_surface_manager.h"
#include "flutter/shell/platform/windows/key_event_handler.h"
#include "flutter/shell/platform/windows/keyboard_hook_handler.h"
#include "flutter/shell/platform/windows/platform_handler.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "flutter/shell/platform/windows/text_input_plugin.h"
//#include "flutter/shell/platform/windows/window_state.h"
#include <dispatcherqueue.h>
#include <versionhelpers.h>
#include <windows.foundation.metadata.h>
#include <windows.ui.composition.h>
#include <windows.ui.composition.interop.h>
#include <winrt/Windows.UI.Composition.h>
#include <wrl.h>

#include "flutter/shell/platform/windows/window_state_comp.h"

namespace flutter {

class RoHelper {
 public:
  RoHelper();
  ~RoHelper();
  bool WinRtAvailable() const;
  bool SupportedWindowsRelease();
  HRESULT GetStringReference(PCWSTR source,
                             HSTRING* act,
                             HSTRING_HEADER* header);
  HRESULT GetActivationFactory(const HSTRING act,
                               const IID& interfaceId,
                               void** fac);
  HRESULT WindowsCompareStringOrdinal(HSTRING one, HSTRING two, int* result);
  HRESULT CreateDispatcherQueueController(
      DispatcherQueueOptions options,
      ABI::Windows::System::IDispatcherQueueController**
          dispatcherQueueController);
  HRESULT WindowsDeleteString(HSTRING one);
  HRESULT RoInitialize(RO_INIT_TYPE type);
  void RoUninitialize();

 private:
  using WindowsCreateStringReference_ = HRESULT __stdcall(PCWSTR,
                                                          UINT32,
                                                          HSTRING_HEADER*,
                                                          HSTRING*);

  using GetActivationFactory_ = HRESULT __stdcall(HSTRING, REFIID, void**);

  using WindowsCompareStringOrginal_ = HRESULT __stdcall(HSTRING,
                                                         HSTRING,
                                                         int*);

  using WindowsDeleteString_ = HRESULT __stdcall(HSTRING);

  using CreateDispatcherQueueController_ =
      HRESULT __stdcall(DispatcherQueueOptions,
                        ABI::Windows::System::IDispatcherQueueController**);

  using RoInitialize_ = HRESULT __stdcall(RO_INIT_TYPE);
  using RoUninitialize_ = void __stdcall();

  WindowsCreateStringReference_* mFpWindowsCreateStringReference;
  GetActivationFactory_* mFpGetActivationFactory;
  WindowsCompareStringOrginal_* mFpWindowsCompareStringOrdinal;
  CreateDispatcherQueueController_* mFpCreateDispatcherQueueController;
  WindowsDeleteString_* mFpWindowsDeleteString;
  RoInitialize_* mFpRoInitialize;
  RoUninitialize_* mFpRoUninitialize;

  bool mWinRtAvailable;

  HMODULE mComBaseModule;
  HMODULE mCoreMessagingModule;
};

// Struct holding the mouse state. The engine doesn't keep track of which mouse
// buttons have been pressed, so it's the embedding's responsibility.
struct MouseState {
  // True if the last event sent to Flutter had at least one mouse button
  // pressed.
  bool flutter_state_is_down = false;

  // True if kAdd has been sent to Flutter. Used to determine whether
  // to send a kAdd event before sending an incoming mouse event, since Flutter
  // expects pointers to be added before events are sent for them.
  bool flutter_state_is_added = false;

  // The currently pressed buttons, as represented in FlutterPointerEvent.
  uint64_t buttons = 0;
};

// A windowing neutral flutter child video used as implementation for flutter
// view that works with win32 hwnds and Windows::UI::Composition visuals
class FlutterWindowsView {
 public:
  // Create flutter Window for use as child window
  FlutterWindowsView(int width, int height);

  ~FlutterWindowsView();

  static FlutterDesktopViewControllerRef CreateFlutterWindowsView(
      int width,
      int height,
      ABI::Windows::UI::Composition::IVisual* visual);

  static FlutterDesktopViewControllerRef CreateFlutterWindowsViewHwnd(
      int width,
      int height,
      void* externalwindow,
      HWND windowrendertarget);

  // Configures the window instance with an instance of a running Flutter engine
  // returning a configured FlutterDesktopWindowControllerRef.
  void SetState(FLUTTER_API_SYMBOL(FlutterEngine) state, void* externalwindow);

  // Returns the currently configured Plugin Registrar.
  FlutterDesktopPluginRegistrarRef GetRegistrar();

  // Callback passed to Flutter engine for notifying window of platform
  // messages.
  void HandlePlatformMessage(const FlutterPlatformMessage*);

  void CreateRenderSurface();

  // Destroy current rendering surface if one has been allocated.
  void DestroyRenderSurface();

  // Callbacks for clearing context, settings context and swapping buffers.
  bool ClearContext();
  bool MakeCurrent();
  bool MakeResourceCurrent();
  bool SwapBuffers();

  // Sends a window metrics update to the Flutter engine using current window
  // dimensions in physical
  void SendWindowMetrics(size_t width, size_t height, double dpiscale);

  // TODO
  void OnPointerMove(double x, double y);

  // TODO
  void OnPointerDown(double x, double y, FlutterPointerMouseButtons button);

  // TODO
  void OnPointerUp(double x, double y, FlutterPointerMouseButtons button);

  // TODO
  void OnPointerLeave();

  // TODO
  void OnText(const std::u16string&);

  // TODO
  void OnKey(int key, int scancode, int action, char32_t character);

  // TODO
  void OnScroll(double x, double y, double delta_x, double delta_y);

  // TODO
  void OnFontChange();

 private:
  // Create a surface for Flutter engine to render into.
  void CreateRenderSurfaceUWP();

  void CreateRenderSurfaceHWND();

  // Reports a mouse movement to Flutter engine.
  void SendPointerMove(double x, double y);

  // Reports mouse press to Flutter engine.
  void SendPointerDown(double x, double y);

  // Reports mouse release to Flutter engine.
  void SendPointerUp(double x, double y);

  // Reports mouse left the window client area.
  //
  // Win32 api doesn't have "mouse enter" event. Therefore, there is no
  // SendPointerEnter method. A mouse enter event is tracked then the "move"
  // event is called.
  void SendPointerLeave();

  // Reports a keyboard character to Flutter engine.
  void SendText(const std::u16string&);

  // Reports a raw keyboard message to Flutter engine.
  void SendKey(int key, int scancode, int action, int mods);

  // Reports scroll wheel events to Flutter engine.
  void SendScroll(double x, double y, double delta_x, double delta_y);

  // Set's |event_data|'s phase to either kMove or kHover depending on the
  // current
  // primary mouse button state.
  void SetEventPhaseFromCursorButtonState(FlutterPointerEvent* event_data);

  // Sends a pointer event to the Flutter engine based on givern data.  Since
  // all input messages are passed in physical pixel values, no translation is
  // needed before passing on to engine.
  void SendPointerEventWithData(const FlutterPointerEvent& event_data);

  // Gets the current mouse state.
  MouseState GetMouseState() { return mouse_state_; }

  // Resets the mouse state to its default values.
  void ResetMouseState() { mouse_state_ = MouseState(); }

  // Updates the mouse state to whether the last event to Flutter had at least
  // one mouse button pressed.
  void SetMouseFlutterStateDown(bool is_down) {
    mouse_state_.flutter_state_is_down = is_down;
  }

  // Updates the mouse state to whether the last event to Flutter was a kAdd
  // event.
  void SetMouseFlutterStateAdded(bool is_added) {
    mouse_state_.flutter_state_is_added = is_added;
  }

  // Updates the currently pressed buttons.
  void SetMouseButtons(uint64_t buttons) { mouse_state_.buttons = buttons; }

  // Updates the size of the compositor host visual
  void SizeHostVisual(size_t width, size_t height);

  std::unique_ptr<AngleSurfaceManager> surface_manager = nullptr;
  EGLSurface render_surface = EGL_NO_SURFACE;

  // state of the mouse button
  bool pointer_is_down_ = false;

  // The handle to the Flutter engine instance.
  FLUTTER_API_SYMBOL(FlutterEngine) engine_ = nullptr;

  // Whether or not to track mouse movements to send kHover events.
  bool hover_tracking_is_enabled_ = false;

  // Whether or not the pointer has been added (or if tracking is enabled, has
  // been added since it was last removed).
  bool pointer_currently_added_ = false;

  // Set to true to be notified when the mouse leaves the window.
  bool tracking_mouse_leave_ = false;

  // Keeps track of mouse state in relation to the window.
  MouseState mouse_state_;

  // The window handle given to API clients.
  std::unique_ptr<FlutterDesktopView> window_wrapper_;

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

  // flag indicating if the message loop should be running
  bool messageloop_running_ = false;

  winrt::Windows::UI::Composition::Compositor compositor_{nullptr};

  winrt::Windows::UI::Composition::Visual flutter_host_{nullptr};

  HWND window_rendertarget_;

  int width_ = 0, height_ = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_H_
