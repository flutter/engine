#include "flutter/shell/platform/windows/flutter_windows_view.h"

#include <chrono>

namespace flutter {

// the Windows DPI system is based on this
// constant for machines running at 100% scaling.
constexpr int base_dpi = 96;

FlutterWindowsView::FlutterWindowsView(int width, int height) {
  surface_manager = std::make_unique<AngleSurfaceManager>();

  width_ = width;
  height_ = height;
}

FlutterWindowsView::~FlutterWindowsView() {
  DestroyRenderSurface();
   if (plugin_registrar_ && plugin_registrar_->destruction_handler) {
    plugin_registrar_->destruction_handler(plugin_registrar_.get());
  }
}

FlutterDesktopViewControllerRef FlutterWindowsView::CreateFlutterWindowsViewHwnd(
    const int width,
    const int height,
    void* externalWindow,
    HWND windowrendertarget) {

  auto state = std::make_unique<FlutterDesktopViewControllerState>();
  state->view =
      std::make_unique<flutter::FlutterWindowsView>(width, height);

  state->view->window_rendertarget_ = windowrendertarget;

  // a window wrapper for the state block, distinct from the
  // window_wrapper handed to plugin_registrar.
  state->view_wrapper = std::make_unique<FlutterDesktopView>();
  state->view_wrapper->window = state->view.get();
  state->view_wrapper->externalwindow = externalWindow;
  return state.release();
}

FlutterDesktopViewControllerRef FlutterWindowsView::CreateFlutterWindowsView(
    const int width,
    const int height,
    ABI::Windows::UI::Composition::IVisual* visual) {
  auto state = std::make_unique<FlutterDesktopViewControllerState>();
  state->view = std::make_unique<flutter::FlutterWindowsView>(width, height);

  // a window wrapper for the state block, distinct from the
  // window_wrapper handed to plugin_registrar.
  state->view_wrapper = std::make_unique<FlutterDesktopView>();
  state->view_wrapper->window = state->view.get();

  // retreieve compositor from parent visual, store it and use it to create a child that we store for rendering
  namespace wuc = winrt::Windows::UI::Composition;
  wuc::Visual parentVisual{nullptr};

  winrt::copy_from_abi(parentVisual, *reinterpret_cast<void**>(&visual));
  state->view->compositor_ = state->view->flutter_host_.Compositor();
  state->view->flutter_host_ = state->view->compositor_.CreateSpriteVisual();
  
  // add the host visual we created as a child of the visual passed in to us
  wuc::SpriteVisual sv{nullptr};
  parentVisual.as(sv);
  sv.Children().InsertAtTop(state->view->flutter_host_);

  return state.release();
}

void FlutterWindowsView::SetState(FLUTTER_API_SYMBOL(FlutterEngine) eng, void* externalwindow) {
  engine_ = eng;

  auto messenger = std::make_unique<FlutterDesktopMessenger>();
  message_dispatcher_ =
      std::make_unique<flutter::IncomingMessageDispatcher>(messenger.get());
  messenger->engine = engine_;
  messenger->dispatcher = message_dispatcher_.get();

  window_wrapper_ = std::make_unique<FlutterDesktopView>();
  window_wrapper_->window = this;
  window_wrapper_->externalwindow = externalwindow;

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
}

FlutterDesktopPluginRegistrarRef FlutterWindowsView::GetRegistrar() {
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

// The Flutter Engine calls out to this function when new platform messages
// are available.
void FlutterWindowsView::HandlePlatformMessage(
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

void FlutterWindowsView::OnPointerMove(double x, double y) {
  if (process_events_) {
    SendPointerMove(x, y);
  }
}

void FlutterWindowsView::OnPointerDown(double x,
                                    double y,
                                    FlutterPointerMouseButtons flutter_button) {
  if (process_events_) {
    if  (flutter_button != 0) {
      uint64_t mouse_buttons = GetMouseState().buttons | flutter_button;
      SetMouseButtons(mouse_buttons);
      SendPointerDown(x, y);
    }
  }
      }

void FlutterWindowsView::OnPointerUp(double x,
                                        double y,
          FlutterPointerMouseButtons flutter_button) {
 if (process_events_) {
    if (flutter_button != 0) {
      uint64_t mouse_buttons = GetMouseState().buttons & ~flutter_button;
      SetMouseButtons(mouse_buttons);
      SendPointerUp(x, y);
    }
  }
}

void FlutterWindowsView::OnPointerLeave() {
  if (process_events_) {
    SendPointerLeave();
  }
}

void FlutterWindowsView::OnText(const std::u16string& text) {
  if (process_events_) {
    SendText(text);
  }
}

void FlutterWindowsView::OnKey(int key,
                               int scancode,
                               int action,
                               char32_t character) {
  if (process_events_) {
    SendKey(key, scancode, action, character);
  }
}

void FlutterWindowsView::OnScroll(double x, double y, double delta_x, double delta_y) {
  if (process_events_) {
    SendScroll(x, y, delta_x, delta_y);
  }
}

void FlutterWindowsView::OnFontChange() {
  if (engine_ == nullptr) {
    return;
  }
  FlutterEngineReloadSystemFonts(engine_);
}

// Sends new size  information to FlutterEngine.
void FlutterWindowsView::SendWindowMetrics(size_t width, size_t height, double dpiScale) {
  if (engine_ == nullptr) {
    return;
  }

  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  event.width = width;
  event.height = height;
  event.pixel_ratio = dpiScale;  
  auto result = FlutterEngineSendWindowMetricsEvent(engine_, &event);

  if (flutter_host_ != nullptr) {
    SizeHostVisual(width, height);
  }
}

// Set's |event_data|'s phase to either kMove or kHover depending on the current
// primary mouse button state.
void FlutterWindowsView::SetEventPhaseFromCursorButtonState(
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

void FlutterWindowsView::SendPointerMove(double x, double y) {
  FlutterPointerEvent event = {};
  event.x = x;
  event.y = y;
  SetEventPhaseFromCursorButtonState(&event);
  SendPointerEventWithData(event);
}

void FlutterWindowsView::SendPointerDown(double x, double y) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.x = x;
  event.y = y;
  SendPointerEventWithData(event);
  SetMouseFlutterStateDown(true);
}

void FlutterWindowsView::SendPointerUp(double x, double y) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.x = x;
  event.y = y;
  SendPointerEventWithData(event);
  if (event.phase == FlutterPointerPhase::kUp) {
    SetMouseFlutterStateDown(false); 
  }
}

void FlutterWindowsView::SendPointerLeave() {
  FlutterPointerEvent event = {};
  event.phase = FlutterPointerPhase::kRemove;
  SendPointerEventWithData(event);
}

void FlutterWindowsView::SendText(const std::u16string& code_point) {
  for (const auto& handler : keyboard_hook_handlers_) {
    handler->TextHook(this, code_point);
  }
}

void FlutterWindowsView::SendKey(int key, int scancode, int action, int mods) {
  for (const auto& handler : keyboard_hook_handlers_) {
    handler->KeyboardHook(this, key, scancode, action, mods);
  }
}

void FlutterWindowsView::SendScroll(double x, double y, double delta_x, double delta_y) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.signal_kind = FlutterPointerSignalKind::kFlutterPointerSignalKindScroll;
  // TODO: See if this can be queried from the OS; this value is chosen
  // arbitrarily to get something that feels reasonable.
  const int kScrollOffsetMultiplier = 20;
  event.x = x;
  event.y = y;
  event.scroll_delta_x = delta_x * kScrollOffsetMultiplier;
  event.scroll_delta_y = delta_y * kScrollOffsetMultiplier;
  SendPointerEventWithData(event);
}

void FlutterWindowsView::SendPointerEventWithData(
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

  FlutterEngineSendPointerEvent(engine_, &event, 1);

  if (event_data.phase == FlutterPointerPhase::kAdd) {
    SetMouseFlutterStateAdded(true);
  } else if (event_data.phase == FlutterPointerPhase::kRemove) {
    SetMouseFlutterStateAdded(false);
    ResetMouseState();
  }
}

bool FlutterWindowsView::MakeCurrent() {
  return surface_manager->MakeCurrent(render_surface);
}

bool FlutterWindowsView::MakeResourceCurrent() {
  return surface_manager->MakeResourceCurrent();
}

bool FlutterWindowsView::ClearContext() {
  return surface_manager->MakeCurrent(nullptr);
}

bool FlutterWindowsView::SwapBuffers() {
  return surface_manager->SwapBuffers(render_surface);
}

void FlutterWindowsView::SizeHostVisual(size_t width, size_t height) {
  flutter_host_.Size({static_cast<float>(width), static_cast<float>(height)});
}

void FlutterWindowsView::CreateRenderSurface() {
  if (compositor_ != nullptr) {
    CreateRenderSurfaceUWP();
  } else {
    CreateRenderSurfaceHWND();
  }
}

void FlutterWindowsView::CreateRenderSurfaceHWND() {
  render_surface =
      surface_manager->CreateSurface(static_cast<HWND>(window_rendertarget_));
}

void FlutterWindowsView::CreateRenderSurfaceUWP() {

  if (surface_manager && render_surface == EGL_NO_SURFACE) {
    
    //TODO: replace ROHelper with regen'd c++/WinRT with downlevel
    RoHelper helper;
    HSTRING act;
    HSTRING_HEADER header;

    flutter_host_.Size(
        {static_cast<float>(width_), static_cast<float>(height_)});

    render_surface = surface_manager->CreateSurface(
        static_cast<ABI::Windows::UI::Composition::ISpriteVisual*>(winrt::get_abi(flutter_host_)));
  }
}

void FlutterWindowsView::DestroyRenderSurface() {
  if (surface_manager) {
    surface_manager->DestroySurface(render_surface);
  }
  render_surface = EGL_NO_SURFACE;
}

// RoHelperImpl.  This is temporary and will go away.

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
