// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/windowing_handler.h"

#include "flutter/fml/logging.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"

namespace {

// Name of the windowing channel.
static constexpr char kChannelName[] = "flutter/windowing";

// Methods for creating different types of windows.
static constexpr char kCreateWindowMethod[] = "createWindow";
static constexpr char kCreateDialogMethod[] = "createDialog";
static constexpr char kCreateSatelliteMethod[] = "createSatellite";
static constexpr char kCreatePopupMethod[] = "createPopup";

// The method to destroy a window.
static constexpr char kDestroyWindowMethod[] = "destroyWindow";

// Error codes used for responses.
static constexpr char kErrorCodeInvalidValue[] = "INVALID_VALUE";
static constexpr char kErrorCodeUnavailable[] = "UNAVAILABLE";

// Retrieves the value associated with |key| from |map|, ensuring it matches
// the expected type |T|. Returns the value if found and correctly typed,
// otherwise logs an error in |result| and returns std::nullopt.
template <typename T>
std::optional<T> GetSingleValueForKeyOrSendError(
    std::string const& key,
    flutter::EncodableMap const* map,
    flutter::MethodResult<>& result) {
  if (auto const it = map->find(flutter::EncodableValue(key));
      it != map->end()) {
    if (auto const* const value = std::get_if<T>(&it->second)) {
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
std::optional<std::vector<T>> GetListValuesForKeyOrSendError(
    std::string const& key,
    flutter::EncodableMap const* map,
    flutter::MethodResult<>& result) {
  if (auto const it = map->find(flutter::EncodableValue(key));
      it != map->end()) {
    if (auto const* const array =
            std::get_if<std::vector<flutter::EncodableValue>>(&it->second)) {
      if (array->size() != Size) {
        result.Error(kErrorCodeInvalidValue,
                     "Array for '" + key + "' key must have " +
                         std::to_string(Size) + " values.");
        return std::nullopt;
      }
      std::vector<T> decoded_values;
      for (flutter::EncodableValue const& value : *array) {
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
std::wstring ArchetypeToWideString(flutter::WindowArchetype archetype) {
  switch (archetype) {
    case flutter::WindowArchetype::regular:
      return L"regular";
    case flutter::WindowArchetype::dialog:
      return L"dialog";
    case flutter::WindowArchetype::satellite:
      return L"satellite";
    case flutter::WindowArchetype::popup:
      return L"popup";
  }
  FML_UNREACHABLE();
}

}  // namespace

namespace flutter {

WindowingHandler::WindowingHandler(BinaryMessenger* messenger,
                                   FlutterHostWindowController* controller)
    : channel_(std::make_shared<MethodChannel<EncodableValue>>(
          messenger,
          kChannelName,
          &StandardMethodCodec::GetInstance())),
      controller_(controller) {
  channel_->SetMethodCallHandler(
      [this](const MethodCall<EncodableValue>& call,
             std::unique_ptr<MethodResult<EncodableValue>> result) {
        HandleMethodCall(call, std::move(result));
      });
  controller_->SetMethodChannel(channel_);
}

void WindowingHandler::HandleMethodCall(
    const MethodCall<EncodableValue>& method_call,
    std::unique_ptr<MethodResult<EncodableValue>> result) {
  const std::string& method = method_call.method_name();

  if (method == kCreateWindowMethod) {
    HandleCreateWindow(WindowArchetype::regular, method_call, *result);
  } else if (method == kCreateDialogMethod) {
    HandleCreateWindow(WindowArchetype::dialog, method_call, *result);
  } else if (method == kCreateSatelliteMethod) {
    HandleCreateWindow(WindowArchetype::satellite, method_call, *result);
  } else if (method == kCreatePopupMethod) {
    HandleCreateWindow(WindowArchetype::popup, method_call, *result);
  } else if (method == kDestroyWindowMethod) {
    HandleDestroyWindow(method_call, *result);
  } else {
    result->NotImplemented();
  }
}

void WindowingHandler::HandleCreateWindow(WindowArchetype archetype,
                                          MethodCall<> const& call,
                                          MethodResult<>& result) {
  auto const* const arguments = call.arguments();
  auto const* const map = std::get_if<EncodableMap>(arguments);
  if (!map) {
    result.Error(kErrorCodeInvalidValue, "Method call argument is not a map.");
    return;
  }

  std::wstring const title = ArchetypeToWideString(archetype);

  auto const size_list =
      GetListValuesForKeyOrSendError<int, 2>("size", map, result);
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

  std::optional<WindowPositioner> positioner;
  std::optional<WindowRectangle> anchor_rect;

  if (archetype == WindowArchetype::satellite ||
      archetype == WindowArchetype::popup) {
    if (auto const anchor_rect_it = map->find(EncodableValue("anchorRect"));
        anchor_rect_it != map->end()) {
      if (!anchor_rect_it->second.IsNull()) {
        auto const anchor_rect_list =
            GetListValuesForKeyOrSendError<int, 4>("anchorRect", map, result);
        if (!anchor_rect_list) {
          return;
        }
        anchor_rect =
            WindowRectangle{{anchor_rect_list->at(0), anchor_rect_list->at(1)},
                            {anchor_rect_list->at(2), anchor_rect_list->at(3)}};
      }
    } else {
      result.Error(kErrorCodeInvalidValue,
                   "Map does not contain required 'anchorRect' key.");
      return;
    }

    auto const positioner_parent_anchor = GetSingleValueForKeyOrSendError<int>(
        "positionerParentAnchor", map, result);
    if (!positioner_parent_anchor) {
      return;
    }
    auto const positioner_child_anchor = GetSingleValueForKeyOrSendError<int>(
        "positionerChildAnchor", map, result);
    if (!positioner_child_anchor) {
      return;
    }
    auto const child_anchor =
        static_cast<WindowPositioner::Anchor>(positioner_child_anchor.value());

    auto const positioner_offset_list =
        GetListValuesForKeyOrSendError<int, 2>("positionerOffset", map, result);
    if (!positioner_offset_list) {
      return;
    }
    auto const positioner_constraint_adjustment =
        GetSingleValueForKeyOrSendError<int>("positionerConstraintAdjustment",
                                             map, result);
    if (!positioner_constraint_adjustment) {
      return;
    }
    positioner = WindowPositioner{
        .anchor_rect = anchor_rect,
        .parent_anchor = static_cast<WindowPositioner::Anchor>(
            positioner_parent_anchor.value()),
        .child_anchor = child_anchor,
        .offset = {positioner_offset_list->at(0),
                   positioner_offset_list->at(1)},
        .constraint_adjustment =
            static_cast<WindowPositioner::ConstraintAdjustment>(
                positioner_constraint_adjustment.value())};
  }

  std::optional<FlutterViewId> parent_view_id;
  if (archetype == WindowArchetype::dialog ||
      archetype == WindowArchetype::satellite ||
      archetype == WindowArchetype::popup) {
    if (auto const parent_it = map->find(EncodableValue("parent"));
        parent_it != map->end()) {
      if (parent_it->second.IsNull()) {
        if (archetype != WindowArchetype::dialog) {
          result.Error(kErrorCodeInvalidValue,
                       "Value for 'parent' key must not be null.");
          return;
        }
      } else {
        if (auto const* const parent = std::get_if<int>(&parent_it->second)) {
          parent_view_id = *parent >= 0 ? std::optional<FlutterViewId>(*parent)
                                        : std::nullopt;
          if (!parent_view_id.has_value() &&
              (archetype == WindowArchetype::satellite ||
               archetype == WindowArchetype::popup)) {
            result.Error(kErrorCodeInvalidValue,
                         "Value for 'parent' key (" +
                             std::to_string(parent_view_id.value()) +
                             ") must be nonnegative.");
            return;
          }
        } else {
          result.Error(kErrorCodeInvalidValue,
                       "Value for 'parent' key must be of type int.");
          return;
        }
      }
    } else {
      result.Error(kErrorCodeInvalidValue,
                   "Map does not contain required 'parent' key.");
      return;
    }
  }

  if (std::optional<WindowMetadata> const data_opt =
          controller_->CreateHostWindow(
              title, {.width = size_list->at(0), .height = size_list->at(1)},
              archetype, positioner, parent_view_id)) {
    WindowMetadata const& data = data_opt.value();
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

void WindowingHandler::HandleDestroyWindow(MethodCall<> const& call,
                                           MethodResult<>& result) {
  auto const* const arguments = call.arguments();
  auto const* const map = std::get_if<EncodableMap>(arguments);
  if (!map) {
    result.Error(kErrorCodeInvalidValue, "Method call argument is not a map.");
    return;
  }

  auto const view_id =
      GetSingleValueForKeyOrSendError<int>("viewId", map, result);
  if (!view_id) {
    return;
  }
  if (view_id.value() < 0) {
    result.Error(kErrorCodeInvalidValue, "Value for 'viewId' (" +
                                             std::to_string(view_id.value()) +
                                             ") cannot be negative.");
    return;
  }

  if (!controller_->DestroyHostWindow(view_id.value())) {
    result.Error(kErrorCodeInvalidValue, "Can't find window with 'viewId' (" +
                                             std::to_string(view_id.value()) +
                                             ").");
    return;
  }

  result.Success();
}

}  // namespace flutter
