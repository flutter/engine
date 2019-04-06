#include "flutter/shell/platform/windows/flutter_window.h"
#include <chrono>

namespace flutter {

#define WINVER 0x0605

FlutterWindow::FlutterWindow() {
  surface_manager = std::make_unique<AngleSurfaceManager>();
}

FlutterWindow::FlutterWindow(const char* title,
                             const int x,
                             const int y,
                             const int width,
                             const int height) noexcept
    : FlutterWindow() {
  Win32Window::Initialize(title, x, y, width, height);
}

FlutterWindow::~FlutterWindow() {
  DestroyRenderSurface();
}

FlutterDesktopWindowControllerRef FlutterWindow::SetState(FlutterEngine eng) {
  engine_ = eng;

  // TODO: Restructure the internals to follow the structure of the C++ API,
  // so
  // that this isn't a tangle of references.
  auto messenger = std::make_unique<FlutterDesktopMessenger>();
  message_dispatcher_ =
      std::make_unique<flutter::IncomingMessageDispatcher>(messenger.get());
  messenger->engine = engine_;
  messenger->dispatcher = message_dispatcher_.get();

  window_wrapper_ = std::make_unique<FlutterDesktopWindow>();
  window_wrapper_->window = this;

  plugin_registrar_ = std::make_unique<FlutterDesktopPluginRegistrar>();
  plugin_registrar_->messenger = std::move(messenger);
  plugin_registrar_->window =
      window_wrapper_.get();  // state->window_wrapper.get();

  internal_plugin_registrar_ =
      std::make_unique<flutter::PluginRegistrar>(plugin_registrar_.get());

  // Set up the keyboard handlers.
  auto internal_plugin_messenger = internal_plugin_registrar_->messenger();
  keyboard_hook_handlers_.push_back(
      std::make_unique<flutter::KeyEventHandler>(internal_plugin_messenger));
  keyboard_hook_handlers_.push_back(
      std::make_unique<flutter::TextInputPlugin>(internal_plugin_messenger));
  platform_handler_ = std::make_unique<flutter::PlatformHandler>(
      internal_plugin_messenger, this);

  auto state = std::make_unique<FlutterDesktopWindowControllerState>();
  state->engine = engine_;
  state->window = nullptr;  // null here as the caller owns the uniqueptr we'll
                            // ultimately want to return
  state->window_wrapper =
      std::make_unique<FlutterDesktopWindow>();  // a different window wrapper
                                                 // for same window pointer
                                                 // hence make_unique again
  state->window_wrapper->window = this;
  return state.release();
}

FlutterDesktopPluginRegistrarRef FlutterWindow::GetRegistrar() {
  return plugin_registrar_.get();
}

void FlutterWindow::HandleMessage(const FlutterDesktopMessage& message,
                                  std::function<void(void)> input_block_cb,
                                  std::function<void(void)> input_unblock_cb) {
  message_dispatcher_->HandleMessage(message, input_block_cb, input_unblock_cb);
}

// When DesktopWindow notifies that a WM_Size message has come in
// let FlutterEngine know about the new size.
void FlutterWindow::OnResize(unsigned int width, unsigned int height) {
  current_width_ = width;
  current_height_ = height;
  SendWindowMetrics();
}
// When DesktopWindow notifies that a WM_Size message has come in
// let FlutterEngine know about the new size.
void FlutterWindow::OnResize() {
  SendWindowMetrics();
}

// When DesktopWindow notifies that the DPI has changed
// handle the change.
void FlutterWindow::OnDpiScale(unsigned int dpi) {
  current_dpi_ = dpi;

  // No need to SendWindowMetrics here as OnResize will be called right after
  // DPI change
}

void FlutterWindow::OnPointerMove(double x, double y) {
  SendPointerMove(x, y);
}

void FlutterWindow::OnPointerDown(double x, double y) {
  SendPointerDown(x, y);
}

void FlutterWindow::OnPointerUp(double x, double y) {
  SendPointerUp(x, y);
}

void FlutterWindow::OnChar(unsigned int code_point) {
  SendChar(code_point);
}

void FlutterWindow::OnKey(int key, int scancode, int action, int mods) {
  SendKey(key, scancode, action, 0);
}

void FlutterWindow::OnScroll(double delta_x, double delta_y) {
  SendScroll(delta_x, delta_y);
}

void FlutterWindow::FlutterMessageLoop() {
  MSG message;

  while (GetMessage(&message, nullptr, 0, 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
    __FlutterEngineFlushPendingTasksNow();
  }
}

// Send new size information to FlutterEngine.
void FlutterWindow::SendWindowMetrics() {
  if (engine_ == nullptr) {
    return;
  }

  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  event.width = current_width_;
  event.height = current_height_;
  event.pixel_ratio = static_cast<double>(current_dpi_) / base_dpi;
  auto result = FlutterEngineSendWindowMetricsEvent(engine_, &event);
}

// Updates |event_data| with the current location of the mouse cursor.
void FlutterWindow::SetEventLocationFromCursorPosition(
    FlutterPointerEvent* event_data) {
  POINT point;
  GetCursorPos(&point);

  ScreenToClient(window_handle_, &point);

  event_data->x = point.x;
  event_data->y = point.y;
}

// Set's |event_data|'s phase to either kMove or kHover depending on the current
// primary mouse button state.
void FlutterWindow::SetEventPhaseFromCursorButtonState(
    FlutterPointerEvent* event_data) {
  event_data->phase = pointer_is_down_ ? FlutterPointerPhase::kMove
                                       : FlutterPointerPhase::kHover;
}

void FlutterWindow::SendPointerMove(double x, double y) {
  FlutterPointerEvent event = {};
  event.x = x;
  event.y = y;
  SetEventPhaseFromCursorButtonState(&event);
  SendPointerEventWithData(event);
}

void FlutterWindow::SendPointerDown(double x, double y) {
  pointer_is_down_ = true;
  FlutterPointerEvent event = {};
  event.phase = FlutterPointerPhase::kDown;
  event.x = x;
  event.y = y;
  SendPointerEventWithData(event);
}

void FlutterWindow::SendPointerUp(double x, double y) {
  pointer_is_down_ = false;
  FlutterPointerEvent event = {};
  event.phase = FlutterPointerPhase::kUp;
  event.x = x;
  event.y = y;
  SendPointerEventWithData(event);
}

void FlutterWindow::SendChar(unsigned int code_point) {
  for (const auto& handler : keyboard_hook_handlers_) {
    handler->CharHook(this, code_point);
  }
}

void FlutterWindow::SendKey(int key, int scancode, int action, int mods) {
  for (const auto& handler : keyboard_hook_handlers_) {
    handler->KeyboardHook(this, key, scancode, action, mods);
  }
}

void FlutterWindow::SendScroll(double delta_x, double delta_y) {
  FlutterPointerEvent event = {};
  SetEventLocationFromCursorPosition(&event);
  SetEventPhaseFromCursorButtonState(&event);
  event.signal_kind = FlutterPointerSignalKind::kFlutterPointerSignalKindScroll;
  // TODO: See if this can be queried from the OS; this value is chosen
  // arbitrarily to get something that feels reasonable.
  const int kScrollOffsetMultiplier = 20;
  event.scroll_delta_x = delta_x * kScrollOffsetMultiplier;
  event.scroll_delta_y = delta_y * kScrollOffsetMultiplier;
  SendPointerEventWithData(event);
}

void FlutterWindow::SendPointerEventWithData(
    const FlutterPointerEvent& event_data) {
  // If sending anything other than an add, and the pointer isn't already added,
  // synthesize an add to satisfy Flutter's expectations about events.
  if (!pointer_currently_added_ &&
      event_data.phase != FlutterPointerPhase::kAdd) {
    FlutterPointerEvent event = {};
    event.phase = FlutterPointerPhase::kAdd;
    event.x = event_data.x;
    event.y = event_data.y;
    SendPointerEventWithData(event);
  }
  // Don't double-add (e.g., if events are delivered out of order, so an add has
  // already been synthesized).
  if (pointer_currently_added_ &&
      event_data.phase == FlutterPointerPhase::kAdd) {
    return;
  }

  FlutterPointerEvent event = event_data;
  // Set metadata that's always the same regardless of the event.
  event.struct_size = sizeof(event);
  event.timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();

  // Since the app is running in per-monitor V2 DPI mode, the application has a
  // physical view of the system hence no need to scale input
  event.scroll_delta_x;
  event.scroll_delta_y;

  FlutterEngineSendPointerEvent(engine_, &event, 1);

  if (event_data.phase == FlutterPointerPhase::kAdd) {
    pointer_currently_added_ = true;
  } else if (event_data.phase == FlutterPointerPhase::kRemove) {
    pointer_currently_added_ = false;
  }
}

bool FlutterWindow::MakeCurrent() {
  return surface_manager->MakeCurrent(render_surface);
}

bool FlutterWindow::ClearContext() {
  return surface_manager->MakeCurrent(nullptr);
}

bool FlutterWindow::SwapBuffers() {
  return surface_manager->SwapBuffers(render_surface);
}

void FlutterWindow::CreateRenderSurface() {
  if (surface_manager && render_surface == EGL_NO_SURFACE) {
    render_surface = surface_manager->CreateSurface(window_handle_);
  }
}

void FlutterWindow::DestroyRenderSurface() {
  if (surface_manager) {
    surface_manager->DestroySurface(render_surface);
  }
  render_surface = EGL_NO_SURFACE;
}

}  // namespace flutter