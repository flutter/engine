#include "include/flutter/flutter_window_controller.h"

#include "include/flutter/encodable_value.h"
#include "include/flutter/flutter_win32_window.h"
#include "include/flutter/standard_method_codec.h"

#include <iomanip>
#include <sstream>

#include <dwmapi.h>

namespace {

auto const* const kChannel{"flutter/windowing"};
auto const* const kErrorCodeInvalidValue{"INVALID_VALUE"};
auto const* const kErrorCodeUnavailable{"UNAVAILABLE"};

// Retrieves the value associated with |key| from |map|, ensuring it matches
// the expected type |T|. Returns the value if found and correctly typed,
// otherwise logs an error in |result| and returns std::nullopt.
template <typename T>
auto GetSingleValueForKeyOrSendError(std::string const& key,
                                     flutter::EncodableMap const* map,
                                     flutter::MethodResult<>& result)
    -> std::optional<T> {
  if (auto const it{map->find(flutter::EncodableValue(key))};
      it != map->end()) {
    if (auto const* const value{std::get_if<T>(&it->second)}) {
      return *value;
    } else {
      result.Error(kErrorCodeInvalidValue, "Value for '" + key +
                                               "' key must be of type '" +
                                               typeid(T).name() + "'.");
    }
  } else {
    result.Error(kErrorCodeInvalidValue,
                 "Map does not contain required '" + key + "' key.");
  }
  return std::nullopt;
}

// Retrieves a list of values associated with |key| from |map|, ensuring the
// list has |Size| elements, all of type |T|. Returns the list if found and
// valid, otherwise logs an error in |result| and returns std::nullopt.
template <typename T, size_t Size>
auto GetListValuesForKeyOrSendError(std::string const& key,
                                    flutter::EncodableMap const* map,
                                    flutter::MethodResult<>& result)
    -> std::optional<std::vector<T>> {
  if (auto const it{map->find(flutter::EncodableValue(key))};
      it != map->end()) {
    if (auto const* const array{
            std::get_if<std::vector<flutter::EncodableValue>>(&it->second)}) {
      if (array->size() != Size) {
        result.Error(kErrorCodeInvalidValue,
                     "Array for '" + key + "' key must have " +
                         std::to_string(Size) + " values.");
        return std::nullopt;
      }
      std::vector<T> decoded_values;
      for (auto const& value : *array) {
        if (std::holds_alternative<T>(value)) {
          decoded_values.push_back(std::get<T>(value));
        } else {
          result.Error(kErrorCodeInvalidValue,
                       "Array for '" + key +
                           "' key must only have values of type '" +
                           typeid(T).name() + "'.");
          return std::nullopt;
        }
      }
      return decoded_values;
    } else {
      result.Error(kErrorCodeInvalidValue,
                   "Value for '" + key + "' key must be an array.");
    }
  } else {
    result.Error(kErrorCodeInvalidValue,
                 "Map does not contain required '" + key + "' key.");
  }
  return std::nullopt;
}

// Converts a |flutter::WindowArchetype| to its corresponding wide string
// representation.
auto ArchetypeToWideString(flutter::WindowArchetype archetype) -> std::wstring {
  switch (archetype) {
    case flutter::WindowArchetype::regular:
      return L"regular";
    case flutter::WindowArchetype::floating_regular:
      return L"floating_regular";
    case flutter::WindowArchetype::dialog:
      return L"dialog";
    case flutter::WindowArchetype::satellite:
      return L"satellite";
    case flutter::WindowArchetype::popup:
      return L"popup";
    case flutter::WindowArchetype::tip:
      return L"tip";
  }
  std::cerr
      << "Unhandled window archetype encountered in archetypeToWideString: "
      << static_cast<int>(archetype) << "\n";
  std::abort();
}

}  // namespace

namespace flutter {

FlutterWindowController::~FlutterWindowController() {
  {
    std::lock_guard lock(mutex_);
    if (channel_) {
      channel_->SetMethodCallHandler(nullptr);
    }
  }
  DestroyWindows();
}

void FlutterWindowController::DestroyWindows() {
  std::unique_lock lock(mutex_);
  std::vector<FlutterViewId> view_ids;
  view_ids.reserve(windows_.size());
  for (auto const& [view_id, _] : windows_) {
    view_ids.push_back(view_id);
  }
  lock.unlock();
  for (auto const& view_id : view_ids) {
    DestroyFlutterWindow(view_id);
  }
}

void FlutterWindowController::SetEngine(std::shared_ptr<FlutterEngine> engine) {
  DestroyWindows();
  std::lock_guard const lock(mutex_);
  engine_ = std::move(engine);
  channel_ = std::make_unique<MethodChannel<>>(
      engine_->messenger(), kChannel, &StandardMethodCodec::GetInstance());
  channel_->SetMethodCallHandler(
      [this](MethodCall<> const& call, std::unique_ptr<MethodResult<>> result) {
        MethodCallHandler(call, *result);
      });
}

auto FlutterWindowController::CreateFlutterWindow(std::wstring const& title,
                                                  WindowSize const& size,
                                                  WindowArchetype archetype)
    -> std::optional<WindowMetadata> {
  std::unique_lock lock(mutex_);
  if (!engine_) {
    std::cerr << "Cannot create window without an engine.\n";
    return std::nullopt;
  }

  auto window{std::make_unique<FlutterWin32Window>(engine_, win32_)};

  lock.unlock();

  if (!window->Create(title, size, archetype)) {
    return std::nullopt;
  }

  lock.lock();

  // Assume first window is the main window
  if (windows_.empty()) {
    window->SetQuitOnClose(true);
  }

  auto const view_id{window->GetFlutterViewId()};
  windows_[view_id] = std::move(window);

  SendOnWindowCreated(view_id, std::nullopt);

  WindowMetadata result{.view_id = view_id,
                        .archetype = archetype,
                        .size = GetWindowSize(view_id),
                        .parent_id = std::nullopt};

  return result;
}

auto FlutterWindowController::DestroyFlutterWindow(FlutterViewId view_id)
    -> bool {
  std::unique_lock lock(mutex_);
  auto it{windows_.find(view_id)};
  if (it != windows_.end()) {
    auto* const window{it->second.get()};

    lock.unlock();

    // |window| will be removed from |windows_| when WM_NCDESTROY is handled
    win32_->DestroyWindow(window->GetHandle());

    return true;
  }
  return false;
}

FlutterWindowController::FlutterWindowController()
    : win32_{std::make_shared<Win32Wrapper>()} {}

FlutterWindowController::FlutterWindowController(
    std::shared_ptr<Win32Wrapper> wrapper)
    : win32_{std::move(wrapper)} {}

void FlutterWindowController::MethodCallHandler(MethodCall<> const& call,
                                                MethodResult<>& result) {
  if (call.method_name() == "createWindow") {
    HandleCreateWindow(WindowArchetype::regular, call, result);
  } else if (call.method_name() == "destroyWindow") {
    HandleDestroyWindow(call, result);
  } else {
    result.NotImplemented();
  }
}

auto FlutterWindowController::MessageHandler(HWND hwnd,
                                             UINT message,
                                             WPARAM wparam,
                                             LPARAM lparam) -> LRESULT {
  switch (message) {
    case WM_NCDESTROY: {
      std::unique_lock lock{mutex_};
      auto const it{std::find_if(windows_.begin(), windows_.end(),
                                 [hwnd](auto const& window) {
                                   return window.second->GetHandle() == hwnd;
                                 })};
      if (it != windows_.end()) {
        auto const view_id{it->first};
        auto const quit_on_close{it->second.get()->GetQuitOnClose()};

        windows_.erase(it);

        if (quit_on_close) {
          auto it2{windows_.begin()};
          while (it2 != windows_.end()) {
            auto const& that{it2->second};
            lock.unlock();
            DestroyWindow(that->GetHandle());
            lock.lock();
            it2 = windows_.begin();
          }
        }

        SendOnWindowDestroyed(view_id);
      }
    }
      return 0;
    case WM_SIZE: {
      std::lock_guard lock{mutex_};
      auto const it{std::find_if(windows_.begin(), windows_.end(),
                                 [hwnd](auto const& window) {
                                   return window.second->GetHandle() == hwnd;
                                 })};
      if (it != windows_.end()) {
        auto const view_id{it->first};
        SendOnWindowChanged(view_id);
      }
    } break;
    default:
      break;
  }

  if (auto* const window{Win32Window::GetThisFromHandle(hwnd)}) {
    return window->MessageHandler(hwnd, message, wparam, lparam);
  }
  return DefWindowProc(hwnd, message, wparam, lparam);
}

void FlutterWindowController::SendOnWindowCreated(
    FlutterViewId view_id,
    std::optional<FlutterViewId> parent_view_id) const {
  if (channel_) {
    channel_->InvokeMethod(
        "onWindowCreated",
        std::make_unique<EncodableValue>(EncodableMap{
            {EncodableValue("viewId"), EncodableValue(view_id)},
            {EncodableValue("parentViewId"),
             parent_view_id ? EncodableValue(parent_view_id.value())
                            : EncodableValue()}}));
  }
}

void FlutterWindowController::SendOnWindowDestroyed(
    FlutterViewId view_id) const {
  if (channel_) {
    channel_->InvokeMethod(
        "onWindowDestroyed",
        std::make_unique<EncodableValue>(EncodableMap{
            {EncodableValue("viewId"), EncodableValue(view_id)},
        }));
  }
}

void FlutterWindowController::SendOnWindowChanged(FlutterViewId view_id) const {
  if (channel_) {
    auto const size{GetWindowSize(view_id)};
    channel_->InvokeMethod(
        "onWindowChanged",
        std::make_unique<EncodableValue>(EncodableMap{
            {EncodableValue("viewId"), EncodableValue(view_id)},
            {EncodableValue("size"),
             EncodableValue(EncodableList{EncodableValue(size.width),
                                          EncodableValue(size.height)})},
            {EncodableValue("relativePosition"), EncodableValue()},  // TODO
            {EncodableValue("isMoving"), EncodableValue()}}));       // TODO
  }
}

void FlutterWindowController::HandleCreateWindow(WindowArchetype archetype,
                                                 MethodCall<> const& call,
                                                 MethodResult<>& result) {
  auto const* const arguments{call.arguments()};
  auto const* const map{std::get_if<EncodableMap>(arguments)};
  if (!map) {
    result.Error(kErrorCodeInvalidValue, "Method call argument is not a map.");
    return;
  }

  std::wstring const title{ArchetypeToWideString(archetype)};

  auto const size_list{
      GetListValuesForKeyOrSendError<int, 2>("size", map, result)};
  if (!size_list) {
    return;
  }
  if (size_list->at(0) < 0 || size_list->at(1) < 0) {
    result.Error(kErrorCodeInvalidValue,
                 "Values for 'size' key (" + std::to_string(size_list->at(0)) +
                     ", " + std::to_string(size_list->at(1)) +
                     ") must be nonnegative.");
    return;
  }

  if (auto const data_opt{CreateFlutterWindow(
          title, {.width = size_list->at(0), .height = size_list->at(1)},
          archetype)}) {
    auto const& data{data_opt.value()};
    result.Success(EncodableValue(EncodableMap{
        {EncodableValue("viewId"), EncodableValue(data.view_id)},
        {EncodableValue("archetype"),
         EncodableValue(static_cast<int>(data.archetype))},
        {EncodableValue("size"),
         EncodableValue(EncodableList{EncodableValue(data.size.width),
                                      EncodableValue(data.size.height)})},
        {EncodableValue("parentViewId"),
         data.parent_id ? EncodableValue(data.parent_id.value())
                        : EncodableValue()}}));
  } else {
    result.Error(kErrorCodeUnavailable, "Can't create window.");
  }
}

void FlutterWindowController::HandleDestroyWindow(MethodCall<> const& call,
                                                  MethodResult<>& result) {
  auto const* const arguments{call.arguments()};
  auto const* const map{std::get_if<EncodableMap>(arguments)};
  if (!map) {
    result.Error(kErrorCodeInvalidValue, "Method call argument is not a map.");
    return;
  }

  auto const view_id{
      GetSingleValueForKeyOrSendError<int>("viewId", map, result)};
  if (!view_id) {
    return;
  }
  if (view_id.value() < 0) {
    result.Error(kErrorCodeInvalidValue, "Value for 'viewId' (" +
                                             std::to_string(view_id.value()) +
                                             ") cannot be negative.");
    return;
  }

  if (!DestroyFlutterWindow(view_id.value())) {
    result.Error(kErrorCodeInvalidValue, "Can't find window with 'viewId' (" +
                                             std::to_string(view_id.value()) +
                                             ").");
    return;
  }

  result.Success();
}

WindowSize FlutterWindowController::GetWindowSize(
    flutter::FlutterViewId view_id) const {
  auto* const hwnd{windows_.at(view_id)->GetHandle()};
  RECT frame_rect;
  DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &frame_rect,
                        sizeof(frame_rect));

  // Convert to logical coordinates
  auto const dpr{FlutterDesktopGetDpiForHWND(hwnd) /
                 static_cast<double>(USER_DEFAULT_SCREEN_DPI)};
  frame_rect.left = static_cast<LONG>(frame_rect.left / dpr);
  frame_rect.top = static_cast<LONG>(frame_rect.top / dpr);
  frame_rect.right = static_cast<LONG>(frame_rect.right / dpr);
  frame_rect.bottom = static_cast<LONG>(frame_rect.bottom / dpr);

  auto const width{frame_rect.right - frame_rect.left};
  auto const height{frame_rect.bottom - frame_rect.top};
  return {static_cast<int>(width), static_cast<int>(height)};
}

}  // namespace flutter
