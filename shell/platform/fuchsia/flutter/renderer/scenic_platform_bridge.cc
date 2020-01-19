// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define RAPIDJSON_HAS_STDSTRING 1

#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_platform_bridge.h"

#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/trace/event.h>
#include <zircon/status.h>
#include <zircon/types.h>

#include <cmath>

#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace flutter_runner {
namespace {

static constexpr char kFlutterPlatformChannel[] = "flutter/platform";
static constexpr char kTextInputChannel[] = "flutter/textinput";
static constexpr char kKeyEventChannel[] = "flutter/keyevent";
static constexpr char kAccessibilityChannel[] = "flutter/accessibility";
static constexpr char kFlutterPlatformViewsChannel[] = "flutter/platform_views";

FlutterPointerPhase GetFlutterPhaseFromFuchsiaPhase(
    fuchsia::ui::input::PointerEventPhase phase) {
  switch (phase) {
    case fuchsia::ui::input::PointerEventPhase::ADD:
      return FlutterPointerPhase::kAdd;
    case fuchsia::ui::input::PointerEventPhase::HOVER:
      return FlutterPointerPhase::kHover;
    case fuchsia::ui::input::PointerEventPhase::DOWN:
      return FlutterPointerPhase::kDown;
    case fuchsia::ui::input::PointerEventPhase::MOVE:
      return FlutterPointerPhase::kMove;
    case fuchsia::ui::input::PointerEventPhase::UP:
      return FlutterPointerPhase::kUp;
    case fuchsia::ui::input::PointerEventPhase::REMOVE:
      return FlutterPointerPhase::kRemove;
    case fuchsia::ui::input::PointerEventPhase::CANCEL:
      return FlutterPointerPhase::kCancel;
    default:
      return FlutterPointerPhase::kCancel;
  }
}

FlutterPointerDeviceKind GetFlutterDeviceKindFromFuchsiaPointerType(
    fuchsia::ui::input::PointerEventType type) {
  switch (type) {
    case fuchsia::ui::input::PointerEventType::TOUCH:
      return FlutterPointerDeviceKind::kFlutterPointerDeviceKindTouch;
    case fuchsia::ui::input::PointerEventType::MOUSE:
      return FlutterPointerDeviceKind::kFlutterPointerDeviceKindMouse;
    default:
      return FlutterPointerDeviceKind::kFlutterPointerDeviceKindTouch;
  }
}

// TODO(SCN-1278): Remove this.
// Turns two floats (high bits, low bits) into a 64-bit uint.
trace_flow_id_t PointerTraceHACK(float fa, float fb) {
  uint32_t ia, ib;
  memcpy(&ia, &fa, sizeof(uint32_t));
  memcpy(&ib, &fb, sizeof(uint32_t));
  return (((uint64_t)ia) << 32) | ib;
}

inline fuchsia::ui::gfx::vec3 Add(const fuchsia::ui::gfx::vec3& a,
                                  const fuchsia::ui::gfx::vec3& b) {
  return {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

inline fuchsia::ui::gfx::vec3 Subtract(const fuchsia::ui::gfx::vec3& a,
                                       const fuchsia::ui::gfx::vec3& b) {
  return {.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

inline fuchsia::ui::gfx::BoundingBox InsetBy(
    const fuchsia::ui::gfx::BoundingBox& box,
    const fuchsia::ui::gfx::vec3& inset_from_min,
    const fuchsia::ui::gfx::vec3& inset_from_max) {
  return {.min = Add(box.min, inset_from_min),
          .max = Subtract(box.max, inset_from_max)};
}

inline fuchsia::ui::gfx::BoundingBox ViewPropertiesLayoutBox(
    const fuchsia::ui::gfx::ViewProperties& view_properties) {
  return InsetBy(view_properties.bounding_box, view_properties.inset_from_min,
                 view_properties.inset_from_max);
}

inline fuchsia::ui::gfx::vec3 Max(const fuchsia::ui::gfx::vec3& v,
                                  float min_val) {
  return {.x = std::max(v.x, min_val),
          .y = std::max(v.y, min_val),
          .z = std::max(v.z, min_val)};
}

}  // end namespace

ScenicPlatformBridge::ScenicPlatformBridge(
    Renderer::DispatchTable dispatch_table,
    RenderDispatchTable render_dispatch_table,
    fuchsia::ui::views::ViewRef view_ref,
    std::shared_ptr<sys::ServiceDirectory> runner_services)
    : dispatch_table_(std::move(dispatch_table)),
      render_dispatch_table_(std::move(render_dispatch_table)),
      accessibility_bridge_(dispatch_table_,
                            std::move(view_ref),
                            runner_services),
      ime_client_(this),
      ime_(nullptr),
      text_sync_service_(
          runner_services->Connect<fuchsia::ui::input::ImeService>()) {
  ime_client_.set_error_handler([this](zx_status_t status) {
    FX_LOG(ERROR) << "Interface error (binding) for "
                     "fuchsia::ui::input::InputMethodEditorClient: "
                  << zx_status_get_string(status);
    dispatch_table_.error_callback();
  });
  ime_.set_error_handler([this](zx_status_t status) {
    FX_LOG(ERROR)
        << "Interface error for fuchsia::ui::input::InputMethodEditor: "
        << zx_status_get_string(status);
    dispatch_table_.error_callback();
  });
  text_sync_service_.set_error_handler([this](zx_status_t status) {
    FX_LOG(ERROR) << "Interface error for fuchsia::ui::input::ImeService: "
                  << zx_status_get_string(status);
    dispatch_table_.error_callback();
  });

  RegisterPlatformMessageHandlers();
}

void ScenicPlatformBridge::OnScenicEvent(
    std::vector<fuchsia::ui::scenic::Event> events) {
  TRACE_DURATION("flutter", "ScenicPlatformBridge::OnScenicEvent");

  bool metrics_updated = false;
  for (const auto& event : events) {
    switch (event.Which()) {
      case fuchsia::ui::scenic::Event::Tag::kGfx:
        switch (event.gfx().Which()) {
          case fuchsia::ui::gfx::Event::Tag::kMetrics: {
            FX_LOG(ERROR) << "Processing gfx::Metrics Event";
            auto& metrics = event.gfx().metrics().metrics;
            FX_DCHECK(metrics.scale_x == metrics.scale_y);

            logical_metrics_.z = metrics.scale_x;
            metrics_updated = true;
            break;
          }
          case fuchsia::ui::gfx::Event::Tag::kViewPropertiesChanged: {
            FX_LOG(ERROR) << "Processing gfx::ViewPropertiesChanged Event";
            auto& view_properties =
                event.gfx().view_properties_changed().properties;
            fuchsia::ui::gfx::BoundingBox layout_box =
                ViewPropertiesLayoutBox(view_properties);
            fuchsia::ui::gfx::vec3 logical_size =
                Max(Subtract(layout_box.max, layout_box.min), 0.f);

            logical_metrics_.x = logical_size.x;
            logical_metrics_.y = logical_size.y;
            metrics_updated = true;
            break;
          }
          case fuchsia::ui::gfx::Event::Tag::kViewConnected:
            FX_LOG(ERROR) << "Processing gfx::ViewConnected Event";
            render_dispatch_table_.child_view_connected_callback(
                event.gfx().view_connected().view_holder_id);
            break;
          case fuchsia::ui::gfx::Event::Tag::kViewDisconnected:
            FX_LOG(ERROR) << "Processing gfx::ViewDisconnected Event";
            render_dispatch_table_.child_view_disconnected_callback(
                event.gfx().view_disconnected().view_holder_id);
            break;
          case fuchsia::ui::gfx::Event::Tag::kViewStateChanged:
            FX_LOG(ERROR) << "Processing gfx::ViewStateChanged Event";
            render_dispatch_table_.child_view_state_changed_callback(
                event.gfx().view_state_changed().view_holder_id,
                event.gfx().view_state_changed().state.is_rendering);
            break;
          case fuchsia::ui::gfx::Event::Tag::Invalid:
            FX_LOG(ERROR) << "Flutter ScenicPlatformBridge::OnScenicEvent: Got "
                             "an invalid GFX event.";
            FX_DCHECK(false);
            break;
          default:
            FX_LOG(ERROR) << "Flutter ScenicPlatformBridge::OnScenicEvent: Got "
                             "an unknown GFX event: "
                          << event.gfx().Which();
            // We don't care about some event types, so not handling them is OK.
            break;
        }
        break;
      case fuchsia::ui::scenic::Event::Tag::kInput:
        switch (event.input().Which()) {
          case fuchsia::ui::input::InputEvent::Tag::kFocus: {
            FX_LOG(ERROR) << "Processing input::Focus Event";
            OnHandleFocusEvent(event.input().focus());
            break;
          }
          case fuchsia::ui::input::InputEvent::Tag::kPointer: {
            FX_LOG(ERROR) << "Processing input::Pointer Event";
            OnHandlePointerEvent(event.input().pointer());
            break;
          }
          case fuchsia::ui::input::InputEvent::Tag::kKeyboard: {
            FX_LOG(ERROR) << "Processing input::Keyboard Event";
            OnHandleKeyboardEvent(event.input().keyboard());
            break;
          }
          case fuchsia::ui::input::InputEvent::Tag::Invalid: {
            FX_LOG(ERROR) << "Flutter ScenicPlatformBridge::OnScenicEvent: Got "
                             "an invalid INPUT event.";
            FX_DCHECK(false);
          }
          default:
            FX_LOG(ERROR) << "Flutter ScenicPlatformBridge::OnScenicEvent: Got "
                             "an unknown INPUT event: "
                          << event.input().Which();
            // We don't care about some event types, so not handling them is OK.
            break;
        }
        break;
      default: {
        break;
      }
    }
  }

  if (metrics_updated) {
    long width = std::lround(logical_metrics_.x * logical_metrics_.z);
    long height = std::lround(logical_metrics_.y * logical_metrics_.z);
    FX_DCHECK(width >= 0);
    FX_DCHECK(height >= 0);
    FX_LOG(ERROR) << "METRICS = " << logical_metrics_.x << ","
                  << logical_metrics_.y << " " << logical_metrics_.z
                  << std::endl
                  << "ROUNDS TO = " << width << "," << height;

    FlutterWindowMetricsEvent current_metrics{
        .struct_size = sizeof(FlutterWindowMetricsEvent),
        .width = static_cast<size_t>(width),
        .height = static_cast<size_t>(height),
        .pixel_ratio = logical_metrics_.z,
    };

    render_dispatch_table_.metrics_changed_callback(&current_metrics);
    dispatch_table_.window_metrics_callback(&current_metrics);
  }
}

void ScenicPlatformBridge::PlatformMessageResponse(
    const FlutterPlatformMessage* message) {
  FX_DCHECK(message);

  bool result = false;
  auto found = platform_message_handlers_.find(message->channel);
  if (found != platform_message_handlers_.end()) {
    result = found->second(message);
  } else {
    FX_LOG(ERROR) << "Got PlatformMessage on channel '" << message->channel
                  << "' with no registered handler; please implement the "
                     "native message handler.";
  }

  // Send a default empty response if the handler did not send one already.
  if (!result) {
    dispatch_table_.platform_message_response_callback(
        message->response_handle);
  }
}

void ScenicPlatformBridge::UpdateSemanticsNode(
    const FlutterSemanticsNode* node) {
  accessibility_bridge_.AddSemanticsNodeUpdate(node);
}

void ScenicPlatformBridge::UpdateSemanticsCustomAction(
    const FlutterSemanticsCustomAction* action) {
  accessibility_bridge_.AddSemanticsCustomActionUpdate(action);
}

void ScenicPlatformBridge::DidUpdateState(
    fuchsia::ui::input::TextInputState state,
    std::unique_ptr<fuchsia::ui::input::InputEvent> input_event) {
  rapidjson::Document document;
  auto& allocator = document.GetAllocator();
  rapidjson::Value encoded_state(rapidjson::kObjectType);
  encoded_state.AddMember("text", state.text, allocator);
  encoded_state.AddMember("selectionBase", state.selection.base, allocator);
  encoded_state.AddMember("selectionExtent", state.selection.extent, allocator);
  switch (state.selection.affinity) {
    case fuchsia::ui::input::TextAffinity::UPSTREAM:
      encoded_state.AddMember("selectionAffinity",
                              rapidjson::Value("TextAffinity.upstream"),
                              allocator);
      break;
    case fuchsia::ui::input::TextAffinity::DOWNSTREAM:
      encoded_state.AddMember("selectionAffinity",
                              rapidjson::Value("TextAffinity.downstream"),
                              allocator);
      break;
  }
  encoded_state.AddMember("selectionIsDirectional", true, allocator);
  encoded_state.AddMember("composingBase", state.composing.start, allocator);
  encoded_state.AddMember("composingExtent", state.composing.end, allocator);

  rapidjson::Value args(rapidjson::kArrayType);
  args.PushBack(current_text_input_client_, allocator);
  args.PushBack(encoded_state, allocator);

  document.SetObject();
  document.AddMember("method",
                     rapidjson::Value("TextInputClient.updateEditingState"),
                     allocator);
  document.AddMember("args", args, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);

  const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer.GetString());
  FlutterPlatformMessage message;
  message.struct_size = sizeof(FlutterPlatformMessage);
  message.channel = kTextInputChannel;
  message.message = data;
  message.message_size = buffer.GetSize();
  message.response_handle = nullptr;
  dispatch_table_.platform_message_callback(&message);

  last_text_state_.emplace(std::move(state));

  // Handle keyboard input events for HID keys only.
  // TODO(SCN-1189): Are we done here?
  if (input_event && input_event->keyboard().hid_usage != 0) {
    OnHandleKeyboardEvent(input_event->keyboard());
  }
}

void ScenicPlatformBridge::OnAction(
    fuchsia::ui::input::InputMethodAction action) {
  rapidjson::Document document;
  auto& allocator = document.GetAllocator();

  rapidjson::Value args(rapidjson::kArrayType);
  args.PushBack(current_text_input_client_, allocator);

  // Done is currently the only text input action defined by Flutter.
  args.PushBack("TextInputAction.done", allocator);

  document.SetObject();
  document.AddMember(
      "method", rapidjson::Value("TextInputClient.performAction"), allocator);
  document.AddMember("args", args, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);

  const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer.GetString());
  FlutterPlatformMessage message;
  message.struct_size = sizeof(FlutterPlatformMessage);
  message.channel = kTextInputChannel;
  message.message = data;
  message.message_size = buffer.GetSize();
  message.response_handle = nullptr;
  dispatch_table_.platform_message_callback(&message);
}

bool ScenicPlatformBridge::OnHandlePointerEvent(
    const fuchsia::ui::input::PointerEvent& pointer) {
  TRACE_DURATION("flutter", "ScenicPlatformBridge::OnHandlePointerEvent");

  // TODO(SCN-1278): Use proper trace_id for tracing flow.
  trace_flow_id_t trace_id =
      PointerTraceHACK(pointer.radius_major, pointer.radius_minor);
  TRACE_FLOW_END("input", "dispatch_event_to_client", trace_id);

  FlutterPointerEvent pointer_event;
  pointer_event.struct_size = sizeof(FlutterPointerEvent);
  pointer_event.phase = GetFlutterPhaseFromFuchsiaPhase(pointer.phase);
  pointer_event.timestamp = pointer.event_time / 1000;
  pointer_event.x = pointer.x * logical_metrics_.z;  // z == pixel_ratio
  pointer_event.y = pointer.y * logical_metrics_.z;  // z == pixel_ratio
  pointer_event.device = pointer.pointer_id;
  pointer_event.signal_kind = kFlutterPointerSignalKindNone;
  pointer_event.scroll_delta_x = 0;
  pointer_event.scroll_delta_y = 0;
  pointer_event.device_kind =
      GetFlutterDeviceKindFromFuchsiaPointerType(pointer.type);
  // Buttons are single bit values starting with kMousePrimaryButton = 1.
  pointer_event.buttons = static_cast<int64_t>(pointer.buttons);

  switch (pointer_event.phase) {
    case FlutterPointerPhase::kDown:
      down_pointers_.insert(pointer_event.device);
      break;
    case FlutterPointerPhase::kCancel:
    case FlutterPointerPhase::kUp:
      down_pointers_.erase(pointer_event.device);
      break;
    case FlutterPointerPhase::kMove:
      if (down_pointers_.count(pointer_event.device) == 0) {
        pointer_event.phase = FlutterPointerPhase::kHover;
      }
      break;
    case FlutterPointerPhase::kAdd:
      if (down_pointers_.count(pointer_event.device) != 0) {
        FX_LOG(ERROR) << "Received add event for down pointer.";
      }
      break;
    case FlutterPointerPhase::kRemove:
      if (down_pointers_.count(pointer_event.device) != 0) {
        FX_LOG(ERROR) << "Received remove event for down pointer.";
      }
      break;
    case FlutterPointerPhase::kHover:
      if (down_pointers_.count(pointer_event.device) != 0) {
        FX_LOG(ERROR) << "Received hover event for down pointer.";
      }
      break;
  }
  dispatch_table_.pointer_event_callback(&pointer_event, 1);

  return true;
}

bool ScenicPlatformBridge::OnHandleKeyboardEvent(
    const fuchsia::ui::input::KeyboardEvent& keyboard) {
  const char* type = nullptr;
  if (keyboard.phase == fuchsia::ui::input::KeyboardEventPhase::PRESSED) {
    type = "keydown";
  } else if (keyboard.phase == fuchsia::ui::input::KeyboardEventPhase::REPEAT) {
    type = "keydown";  // TODO(bug??) change this to keyrepeat
  } else if (keyboard.phase ==
             fuchsia::ui::input::KeyboardEventPhase::RELEASED) {
    type = "keyup";
  }

  if (type == nullptr) {
    FX_LOG(ERROR) << "Unknown key event phase.";
    return false;
  }

  rapidjson::Document document;
  auto& allocator = document.GetAllocator();
  document.SetObject();
  document.AddMember("type", rapidjson::Value(type, strlen(type)), allocator);
  document.AddMember("keymap", rapidjson::Value("fuchsia"), allocator);
  document.AddMember("hidUsage", keyboard.hid_usage, allocator);
  document.AddMember("codePoint", keyboard.code_point, allocator);
  document.AddMember("modifiers", keyboard.modifiers, allocator);
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);

  const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer.GetString());
  FlutterPlatformMessage message;
  message.struct_size = sizeof(FlutterPlatformMessage);
  message.channel = kKeyEventChannel;
  message.message = data;
  message.message_size = buffer.GetSize();
  message.response_handle = nullptr;
  dispatch_table_.platform_message_callback(&message);

  return true;
}

bool ScenicPlatformBridge::OnHandleFocusEvent(
    const fuchsia::ui::input::FocusEvent& focus) {
  // Ensure last_text_state_ is set to make sure Flutter actually wants an IME.
  if (focus.focused && last_text_state_) {
    ActivateIme();
    return true;
  } else if (!focus.focused) {
    DeactivateIme();
    return true;
  }
  return false;
}

void ScenicPlatformBridge::RegisterPlatformMessageHandlers() {
  platform_message_handlers_[kFlutterPlatformChannel] = std::bind(
      &ScenicPlatformBridge::HandleFlutterPlatformChannelPlatformMessage, this,
      std::placeholders::_1);
  platform_message_handlers_[kTextInputChannel] = std::bind(
      &ScenicPlatformBridge::HandleFlutterTextInputChannelPlatformMessage, this,
      std::placeholders::_1);
  platform_message_handlers_[kAccessibilityChannel] = std::bind(
      &ScenicPlatformBridge::HandleAccessibilityChannelPlatformMessage, this,
      std::placeholders::_1);
  platform_message_handlers_[kFlutterPlatformViewsChannel] = std::bind(
      &ScenicPlatformBridge::HandleFlutterPlatformViewsChannelPlatformMessage,
      this, std::placeholders::_1);
}

bool ScenicPlatformBridge::HandleAccessibilityChannelPlatformMessage(
    const FlutterPlatformMessage* message) {
  FX_DCHECK(message->channel == kAccessibilityChannel);
  return false;
}

bool ScenicPlatformBridge::HandleFlutterPlatformChannelPlatformMessage(
    const FlutterPlatformMessage* message) {
  FX_DCHECK(message->channel == kFlutterPlatformChannel);
  return false;
}

bool ScenicPlatformBridge::HandleFlutterTextInputChannelPlatformMessage(
    const FlutterPlatformMessage* message) {
  FX_DCHECK(message->channel == kTextInputChannel);
  rapidjson::Document document;
  document.Parse(reinterpret_cast<const char*>(message->message),
                 message->message_size);
  if (document.HasParseError() || !document.IsObject()) {
    return false;
  }
  auto root = document.GetObject();
  auto method = root.FindMember("method");
  if (method == root.MemberEnd() || !method->value.IsString()) {
    return false;
  }

  if (method->value == "TextInput.show") {
    if (ime_) {
      text_sync_service_->ShowKeyboard();
    }
  } else if (method->value == "TextInput.hide") {
    if (ime_) {
      text_sync_service_->HideKeyboard();
    }
  } else if (method->value == "TextInput.setClient") {
    current_text_input_client_ = 0;
    DeactivateIme();
    auto args = root.FindMember("args");
    if (args == root.MemberEnd() || !args->value.IsArray() ||
        args->value.Size() != 2)
      return false;
    const auto& configuration = args->value[1];
    if (!configuration.IsObject()) {
      return false;
    }
    // TODO(bug??): Read the keyboard type from the configuration.
    current_text_input_client_ = args->value[0].GetInt();

    auto initial_text_input_state = fuchsia::ui::input::TextInputState{};
    initial_text_input_state.text = "";
    last_text_state_.emplace(initial_text_input_state);
    ActivateIme();
  } else if (method->value == "TextInput.setEditingState") {
    if (ime_) {
      auto args_it = root.FindMember("args");
      if (args_it == root.MemberEnd() || !args_it->value.IsObject()) {
        return false;
      }
      const auto& args = args_it->value;
      fuchsia::ui::input::TextInputState state;
      state.text = "";
      // TODO(bugg??): Deserialize state.
      auto text = args.FindMember("text");
      if (text != args.MemberEnd() && text->value.IsString())
        state.text = text->value.GetString();
      auto selection_base = args.FindMember("selectionBase");
      if (selection_base != args.MemberEnd() && selection_base->value.IsInt())
        state.selection.base = selection_base->value.GetInt();
      auto selection_extent = args.FindMember("selectionExtent");
      if (selection_extent != args.MemberEnd() &&
          selection_extent->value.IsInt())
        state.selection.extent = selection_extent->value.GetInt();
      auto selection_affinity = args.FindMember("selectionAffinity");
      if (selection_affinity != args.MemberEnd() &&
          selection_affinity->value.IsString() &&
          selection_affinity->value == "TextAffinity.upstream")
        state.selection.affinity = fuchsia::ui::input::TextAffinity::UPSTREAM;
      else
        state.selection.affinity = fuchsia::ui::input::TextAffinity::DOWNSTREAM;
      // We ignore selectionIsDirectional because that concept doesn't exist on
      // Fuchsia.
      auto composing_base = args.FindMember("composingBase");
      if (composing_base != args.MemberEnd() && composing_base->value.IsInt())
        state.composing.start = composing_base->value.GetInt();
      auto composing_extent = args.FindMember("composingExtent");
      if (composing_extent != args.MemberEnd() &&
          composing_extent->value.IsInt())
        state.composing.end = composing_extent->value.GetInt();
      ime_->SetState(std::move(state));
    }
  } else if (method->value == "TextInput.clearClient") {
    current_text_input_client_ = 0;
    last_text_state_.reset();
    DeactivateIme();
  } else {
    FX_LOG(ERROR) << "Unknown " << message->channel << " method "
                  << method->value.GetString();
  }

  // Just generate an empty response, regardless.
  return false;
}

bool ScenicPlatformBridge::HandleFlutterPlatformViewsChannelPlatformMessage(
    const FlutterPlatformMessage* message) {
  FX_DCHECK(message->channel == kFlutterPlatformViewsChannel);
  rapidjson::Document document;
  document.Parse(reinterpret_cast<const char*>(message->message),
                 message->message_size);
  if (document.HasParseError() || !document.IsObject()) {
    FX_LOG(ERROR) << "Could not parse document";
    return false;
  }
  auto root = document.GetObject();
  auto method = root.FindMember("method");
  if (method == root.MemberEnd() || !method->value.IsString()) {
    return false;
  }

  if (method->value == "View.enableWireframe") {
    auto args_it = root.FindMember("args");
    if (args_it == root.MemberEnd() || !args_it->value.IsObject()) {
      FX_LOG(ERROR) << "No arguments found.";
      return false;
    }
    const auto& args = args_it->value;

    auto enable = args.FindMember("enable");
    if (!enable->value.IsBool()) {
      FX_LOG(ERROR) << "Argument 'enable' is not a bool";
      return false;
    }

    render_dispatch_table_.view_enable_wireframe_callback(
        enable->value.GetBool());
  } else {
    FX_LOG(ERROR) << "Unknown " << message->channel << " method "
                  << method->value.GetString();
  }

  // Just generate an empty response, regardless.
  return false;
}

void ScenicPlatformBridge::ActivateIme() {
  text_sync_service_->GetInputMethodEditor(
      fuchsia::ui::input::KeyboardType::TEXT,       // keyboard type
      fuchsia::ui::input::InputMethodAction::DONE,  // input method action
      *last_text_state_,                            // initial state
      ime_client_.NewBinding(),                     // client
      ime_.NewRequest()                             // editor
  );
}

void ScenicPlatformBridge::DeactivateIme() {
  if (ime_) {
    text_sync_service_->HideKeyboard();
    ime_ = nullptr;
  }
  if (ime_client_.is_bound()) {
    ime_client_.Unbind();
  }
}

// flutter::PointerDataDispatcherMaker
// ScenicPlatformBridge::GetDispatcherMaker()
// {
//   return [](flutter::DefaultPointerDataDispatcher::Delegate& delegate) {
//     return std::make_unique<flutter::SmoothPointerDataDispatcher>(delegate);
//   };
// }

}  // namespace flutter_runner
