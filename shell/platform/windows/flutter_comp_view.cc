#include "flutter/shell/platform/windows/flutter_comp_view.h"

#include <chrono>

namespace flutter {

// the Windows DPI system is based on this
// constant for machines running at 100% scaling.
constexpr int base_dpi = 96;

FlutterCompView::FlutterCompView(int width, int height, void* compositor) {
  surface_manager = std::make_unique<AngleSurfaceManager>();

  compositor_.Attach(static_cast<ABI::Windows::UI::Composition::ICompositor*>(compositor));

  //TODO: use C++/Winrt copy_from_abi once C++.Winrt is supportrf
  //winrt::copy_from_abi(compositor_, compositor);

  width_ = width;
  height_ = height;
}

FlutterCompView::~FlutterCompView() {
  DestroyRenderSurface();
}

FlutterDesktopViewControllerRef FlutterCompView::CreateFlutterCompView(
    const int width,
    const int height,
    void* compositor) {
  auto state = std::make_unique<FlutterDesktopViewControllerState>();
  state->view = std::make_unique<flutter::FlutterCompView>(width, height, compositor);

  // a window wrapper for the state block, distinct from the
  // window_wrapper handed to plugin_registrar.
  state->view_wrapper = std::make_unique<FlutterDesktopView>();
  state->view_wrapper->window = state->view.get();
  return state.release();
}

void FlutterCompView::SetState(FLUTTER_API_SYMBOL(FlutterEngine) eng) {
  engine_ = eng;

  auto messenger = std::make_unique<FlutterDesktopMessenger>();
  message_dispatcher_ =
      std::make_unique<flutter::IncomingMessageDispatcher>(messenger.get());
  messenger->engine = engine_;
  messenger->dispatcher = message_dispatcher_.get();

  window_wrapper_ = std::make_unique<FlutterDesktopView>();
  window_wrapper_->window = this;

  plugin_registrar_ = std::make_unique<FlutterDesktopPluginRegistrar>();
  plugin_registrar_->messenger = std::move(messenger);
  plugin_registrar_->window = window_wrapper_.get();

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

  process_events_ = true;

  //HACK for UWP
  if (compositor_ != nullptr) {
      SendWindowMetrics(width_, height_);
  }
}

FlutterDesktopPluginRegistrarRef FlutterCompView::GetRegistrar() {
  return plugin_registrar_.get();
}

// Converts a FlutterPlatformMessage to an equivalent FlutterDesktopMessage.
static FlutterDesktopMessage ConvertToDesktopMessage(
    const FlutterPlatformMessage& engine_message) {
  FlutterDesktopMessage message = {};
  message.struct_size = sizeof(message);
  message.channel = engine_message.channel;
  message.message = engine_message.message;
  message.message_size = engine_message.message_size;
  message.response_handle = engine_message.response_handle;
  return message;
}

//// Translates button codes from Win32 API to FlutterPointerMouseButtons.
//static uint64_t ConvertWinButtonToFlutterButton(UINT button) {
//  switch (button) {
//    case WM_LBUTTONDOWN:
//    case WM_LBUTTONUP:
//      return kFlutterPointerButtonMousePrimary;
//    case WM_RBUTTONDOWN:
//    case WM_RBUTTONUP:
//      return kFlutterPointerButtonMouseSecondary;
//    case WM_MBUTTONDOWN:
//    case WM_MBUTTONUP:
//      return kFlutterPointerButtonMouseMiddle;
//    case XBUTTON1:
//      return kFlutterPointerButtonMouseBack;
//    case XBUTTON2:
//      return kFlutterPointerButtonMouseForward;
//  }
//  std::cerr << "Mouse button not recognized: " << button << std::endl;
//  return 0;
//}

// The Flutter Engine calls out to this function when new platform messages
// are available.
void FlutterCompView::HandlePlatformMessage(
    const FlutterPlatformMessage* engine_message) {
  if (engine_message->struct_size != sizeof(FlutterPlatformMessage)) {
    std::cerr << "Invalid message size received. Expected: "
              << sizeof(FlutterPlatformMessage) << " but received "
              << engine_message->struct_size << std::endl;
    return;
  }

  auto message = ConvertToDesktopMessage(*engine_message);

  message_dispatcher_->HandleMessage(message,
                                     [this] { this->process_events_ = false; },
                                     [this] { this->process_events_ = true; });
}

void FlutterCompView::OnPointerMove(double x, double y) {
  if (process_events_) {
    SendPointerMove(x, y);
  }
}

void FlutterCompView::OnPointerDown(double x,
                                    double y,
                                    FlutterPointerMouseButtons flutter_button) {
  if (process_events_) {
   /* uint64_t flutter_button = ConvertWinButtonToFlutterButton(button);*/
    if  (flutter_button != 0) {
      uint64_t mouse_buttons = GetMouseState().buttons | flutter_button;
      SetMouseButtons(mouse_buttons);
      SendPointerDown(x, y);
    }
  }
      }

void FlutterCompView::OnPointerUp(double x,
                                        double y,
          FlutterPointerMouseButtons flutter_button) {
 if (process_events_) {
   /* uint64_t flutter_button = ConvertWinButtonToFlutterButton(button);*/
    if (flutter_button != 0) {
      uint64_t mouse_buttons = GetMouseState().buttons & ~flutter_button;
      SetMouseButtons(mouse_buttons);
      SendPointerUp(x, y);
    }
  }
}

void FlutterCompView::OnPointerLeave() {
  if (process_events_) {
    SendPointerLeave();
  }
}

void FlutterCompView::OnChar(char32_t code_point) {
  if (process_events_) {
    SendChar(code_point);
  }
}

void FlutterCompView::OnKey(int key,
                               int scancode,
                               int action,
                               char32_t character) {
  if (process_events_) {
    SendKey(key, scancode, action, character);
  }
}

void FlutterCompView::OnScroll(double delta_x, double delta_y) {
  if (process_events_) {
    SendScroll(delta_x, delta_y);
  }
}

void FlutterCompView::OnFontChange() {
  if (engine_ == nullptr) {
    return;
  }
  FlutterEngineReloadSystemFonts(engine_);
}

// Sends new size  information to FlutterEngine.
void FlutterCompView::SendWindowMetrics(size_t width, size_t height) {
  if (engine_ == nullptr) {
    return;
  }

  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  //event.width = 320;  // GetCurrentWidth(); TODO
  //event.height = 240; //GetCurrentHeight(); TODO
  event.width = width;   // GetCurrentWidth(); TODO
  event.height = height;  // GetCurrentHeight(); TODO
  event.pixel_ratio = 1.2;  // static_cast<double>(GetCurrentDPI()) / base_dpi; TODO
  auto result = FlutterEngineSendWindowMetricsEvent(engine_, &event);


  SizeHostVisual(width, height);
}

// Updates |event_data| with the current location of the mouse cursor.
void FlutterCompView::SetEventLocationFromCursorPosition(
    FlutterPointerEvent* event_data) {
  //POINT point;
  //GetCursorPos(&point);

  //ScreenToClient(GetWindowHandle(), &point); TODO

  //event_data->x = point.x;
  //event_data->y = point.y;
}

// Set's |event_data|'s phase to either kMove or kHover depending on the current
// primary mouse button state.
void FlutterCompView::SetEventPhaseFromCursorButtonState(
    FlutterPointerEvent* event_data) {
  MouseState state = GetMouseState();
  // For details about this logic, see FlutterPointerPhase in the embedder.h
  // file.
  event_data->phase = state.buttons == 0 ? state.flutter_state_is_down
                                               ? FlutterPointerPhase::kUp
                                               : FlutterPointerPhase::kHover
                                         : state.flutter_state_is_down
                                               ? FlutterPointerPhase::kMove
                                               : FlutterPointerPhase::kDown;
}

void FlutterCompView::SendPointerMove(double x, double y) {
  FlutterPointerEvent event = {};
  event.x = x;
  event.y = y;
  SetEventPhaseFromCursorButtonState(&event);
  SendPointerEventWithData(event);
}

void FlutterCompView::SendPointerDown(double x, double y) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.x = x;
  event.y = y;
  SendPointerEventWithData(event);
  SetMouseFlutterStateDown(true);
}

void FlutterCompView::SendPointerUp(double x, double y) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.x = x;
  event.y = y;
  SendPointerEventWithData(event);
  if (event.phase == FlutterPointerPhase::kUp) {
    SetMouseFlutterStateDown(false); 
  }
}

void FlutterCompView::SendPointerLeave() {
  FlutterPointerEvent event = {};
  event.phase = FlutterPointerPhase::kRemove;
  SendPointerEventWithData(event);
}

void FlutterCompView::SendChar(char32_t code_point) {
  for (const auto& handler : keyboard_hook_handlers_) {
    handler->CharHook(this, code_point);
  }
}

void FlutterCompView::SendKey(int key, int scancode, int action, int mods) {
  for (const auto& handler : keyboard_hook_handlers_) {
    handler->KeyboardHook(this, key, scancode, action, mods);
  }
}

void FlutterCompView::SendScroll(double delta_x, double delta_y) {
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

void FlutterCompView::SendPointerEventWithData(
    const FlutterPointerEvent& event_data) {
    //TODO
  MouseState mouse_state = GetMouseState();
  // If sending anything other than an add, and the pointer isn't already added,
  // synthesize an add to satisfy Flutter's expectations about events.
  if (!mouse_state.flutter_state_is_added &&
      event_data.phase != FlutterPointerPhase::kAdd) {
    FlutterPointerEvent event = {};
    event.phase = FlutterPointerPhase::kAdd;
    event.x = event_data.x;
    event.y = event_data.y;
    event.buttons = 0;
    SendPointerEventWithData(event);
  }
  // Don't double-add (e.g., if events are delivered out of order, so an add has
  // already been synthesized).
  if (mouse_state.flutter_state_is_added &&
      event_data.phase == FlutterPointerPhase::kAdd) {
    return;
  }

  FlutterPointerEvent event = event_data;
  event.device_kind = kFlutterPointerDeviceKindMouse;
  event.buttons = mouse_state.buttons;

  // Set metadata that's always the same regardless of the event.
  event.struct_size = sizeof(event);
  event.timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();

  //// Windows passes all input in either physical pixels (Per-monitor, System
  //// DPI) or pre-scaled to match bitmap scaling of output where process is
  //// running in DPI unaware more.  In either case, no need to manually scale
  //// input here.  For more information see DPIHelper.
  //event.scroll_delta_x;
  //event.scroll_delta_y;

  FlutterEngineSendPointerEvent(engine_, &event, 1);

  if (event_data.phase == FlutterPointerPhase::kAdd) {
    SetMouseFlutterStateAdded(true);
  } else if (event_data.phase == FlutterPointerPhase::kRemove) {
    SetMouseFlutterStateAdded(false);
    ResetMouseState();
  }
}

bool FlutterCompView::MakeCurrent() {
  return surface_manager->MakeCurrent(render_surface);
}

bool FlutterCompView::MakeResourceCurrent() {
  return surface_manager->MakeResourceCurrent();
}

bool FlutterCompView::ClearContext() {
  return surface_manager->MakeCurrent(nullptr);
}

bool FlutterCompView::SwapBuffers() {
  return surface_manager->SwapBuffers(render_surface);
}

Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ISpriteVisual>
FlutterCompView::GetFlutterHost() {
  return flutter_host_;
}

void FlutterCompView::SizeHostVisual(size_t width, size_t height) {
  Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::IVisual> visualPtr;

  auto hr = flutter_host_.As(&visualPtr);
  (SUCCEEDED(hr));
  hr = visualPtr->put_Size({static_cast<float>(width), static_cast<float>(height)});
  (SUCCEEDED(hr));
}

// TODO: move this into visual_flutter_host class when that exists
// TODO: can we use c++/winrt in here (requires newer windows SDK and c++17
// support in compiler toolchain)
// TODO: do we still need RoHelper with the above?
void FlutterCompView::CreateRenderSurfaceUWP() {

  if (surface_manager && render_surface == EGL_NO_SURFACE) {
    // render_surface = surface_manager->CreateSurface(GetWindowHandle());

    RoHelper helper;
    HSTRING act;
    HSTRING_HEADER header;

    auto hr = compositor_->CreateSpriteVisual(flutter_host_.GetAddressOf());
    (SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::IVisual> visualPtr;

    hr = flutter_host_.As(&visualPtr);
    (SUCCEEDED(hr));
    hr = visualPtr->put_Size({static_cast<float>(width_), static_cast<float>(height_)});
    (SUCCEEDED(hr));

    hr = visualPtr->put_Offset({40, 20});
    (SUCCEEDED(hr));

    render_surface = surface_manager->CreateSurface(flutter_host_.Get());
  }
}

void FlutterCompView::DestroyRenderSurface() {
  if (surface_manager) {
    surface_manager->DestroySurface(render_surface);
  }
  render_surface = EGL_NO_SURFACE;
}

// RoHelperImpl

template <typename T>
bool AssignProcAddress(HMODULE comBaseModule, const char* name, T*& outProc) {
  outProc = reinterpret_cast<T*>(GetProcAddress(comBaseModule, name));
  return *outProc != nullptr;
}

RoHelper::RoHelper()
    : mFpWindowsCreateStringReference(nullptr),
      mFpGetActivationFactory(nullptr),
      mFpWindowsCompareStringOrdinal(nullptr),
      mFpCreateDispatcherQueueController(nullptr),
      mFpWindowsDeleteString(nullptr),
      mFpRoInitialize(nullptr),
      mFpRoUninitialize(nullptr),
      mWinRtAvailable(false),
      mComBaseModule(nullptr),
      mCoreMessagingModule(nullptr) {
  mComBaseModule = LoadLibraryA("ComBase.dll");

  if (mComBaseModule == nullptr) {
    return;
  }

  if (!AssignProcAddress(mComBaseModule, "WindowsCreateStringReference",
                         mFpWindowsCreateStringReference)) {
    return;
  }

  if (!AssignProcAddress(mComBaseModule, "RoGetActivationFactory",
                         mFpGetActivationFactory)) {
    return;
  }

  if (!AssignProcAddress(mComBaseModule, "WindowsCompareStringOrdinal",
                         mFpWindowsCompareStringOrdinal)) {
    return;
  }

  if (!AssignProcAddress(mComBaseModule, "WindowsDeleteString",
                         mFpWindowsDeleteString)) {
    return;
  }

  if (!AssignProcAddress(mComBaseModule, "RoInitialize", mFpRoInitialize)) {
    return;
  }

  if (!AssignProcAddress(mComBaseModule, "RoUninitialize", mFpRoUninitialize)) {
    return;
  }

  mCoreMessagingModule = LoadLibraryA("coremessaging.dll");

  if (mCoreMessagingModule == nullptr) {
    return;
  }

  if (!AssignProcAddress(mCoreMessagingModule,
                         "CreateDispatcherQueueController",
                         mFpCreateDispatcherQueueController)) {
    return;
  }

  auto result = RoInitialize(RO_INIT_SINGLETHREADED);

  if (SUCCEEDED(result)) {
    // TODO
  }

  mWinRtAvailable = true;
}

RoHelper::~RoHelper() {
  if (mWinRtAvailable) {
    // TODO
    // RoUninitialize();
  }

  if (mCoreMessagingModule != nullptr) {
    FreeLibrary(mCoreMessagingModule);
    mCoreMessagingModule = nullptr;
  }

  if (mComBaseModule != nullptr) {
    FreeLibrary(mComBaseModule);
    mComBaseModule = nullptr;
  }
}

bool RoHelper::WinRtAvailable() const {
  return mWinRtAvailable;
}

bool RoHelper::SupportedWindowsRelease() {
  // if (!IsWindows10OrGreater() || !mWinRtAvailable)
  if (!mWinRtAvailable) {
    return false;
  }

  HSTRING className, contractName;
  HSTRING_HEADER classNameHeader, contractNameHeader;
  boolean isSupported = false;

  HRESULT hr = GetStringReference(
      RuntimeClass_Windows_Foundation_Metadata_ApiInformation, &className,
      &classNameHeader);

  if (FAILED(hr)) {
    return !!isSupported;
  }

  Microsoft::WRL::ComPtr<
      ABI::Windows::Foundation::Metadata::IApiInformationStatics>
      api;

  hr = GetActivationFactory(
      className,
      __uuidof(ABI::Windows::Foundation::Metadata::IApiInformationStatics),
      &api);

  if (FAILED(hr)) {
    return !!isSupported;
  }

  hr = GetStringReference(L"Windows.Foundation.UniversalApiContract",
                          &contractName, &contractNameHeader);
  if (FAILED(hr)) {
    return !!isSupported;
  }

  api->IsApiContractPresentByMajor(contractName, 6, &isSupported);

  return !!isSupported;
}

HRESULT RoHelper::GetStringReference(PCWSTR source,
                                     HSTRING* act,
                                     HSTRING_HEADER* header) {
  if (!mWinRtAvailable) {
    return E_FAIL;
  }

  const wchar_t* str = static_cast<const wchar_t*>(source);

  unsigned int length;
  HRESULT hr = SizeTToUInt32(::wcslen(str), &length);
  if (FAILED(hr)) {
    return hr;
  }

  return mFpWindowsCreateStringReference(source, length, header, act);
}

HRESULT RoHelper::GetActivationFactory(const HSTRING act,
                                       const IID& interfaceId,
                                       void** fac) {
  if (!mWinRtAvailable) {
    return E_FAIL;
  }
  auto hr = mFpGetActivationFactory(act, interfaceId, fac);
  return hr;
}

HRESULT RoHelper::WindowsCompareStringOrdinal(HSTRING one,
                                              HSTRING two,
                                              int* result) {
  if (!mWinRtAvailable) {
    return E_FAIL;
  }
  return mFpWindowsCompareStringOrdinal(one, two, result);
}

HRESULT RoHelper::CreateDispatcherQueueController(
    DispatcherQueueOptions options,
    ABI::Windows::System::IDispatcherQueueController**
        dispatcherQueueController) {
  if (!mWinRtAvailable) {
    return E_FAIL;
  }
  return mFpCreateDispatcherQueueController(options, dispatcherQueueController);
}

HRESULT RoHelper::WindowsDeleteString(HSTRING one) {
  if (!mWinRtAvailable) {
    return E_FAIL;
  }
  return mFpWindowsDeleteString(one);
}

HRESULT RoHelper::RoInitialize(RO_INIT_TYPE type) {
  return mFpRoInitialize(type);
}

void RoHelper::RoUninitialize() {
  mFpRoUninitialize();
}

}  // namespace flutter
