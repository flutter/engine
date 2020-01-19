// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define RAPIDJSON_HAS_STDSTRING 1

#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_input_handler.h"

#include <zircon/types.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
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

template <class T>
void SetInterfaceErrorHandler(
    fidl::Binding<T>& binding,
    ScenicInputHandler::ErrorCallback error_callback) {
  binding.set_error_handler([error_callback](zx_status_t status) {
    FML_LOG(ERROR) << "Interface error (binding) << " << status
                   << " on: ";  // TODO << T::Name_;
    error_callback();
  });
}

template <class T>
void SetInterfaceErrorHandler(
    fidl::InterfacePtr<T>& interface,
    ScenicInputHandler::ErrorCallback error_callback) {
  interface.set_error_handler([error_callback](zx_status_t status) {
    FML_LOG(ERROR) << "Interface error " << status
                   << " on: ";  // TODO << T::Name_;
    error_callback();
  });
}

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

ScenicInputHandler::ScenicInputHandler(
    FlutterEngine& bound_engine,
    RenderDispatchTable render_dispatch_table,
    ErrorCallback error_callback,
    fuchsia::ui::views::ViewRef view_ref,
    std::shared_ptr<sys::ServiceDirectory> runner_services)
    : flutter_engine_(bound_engine),
      accessibility_bridge_(*this, runner_services, std::move(view_ref)),
      render_dispatch_table_(std::move(render_dispatch_table)),
      error_callback_(std::move(error_callback)),
      ime_client_(this),
      ime_(nullptr),
      text_sync_service_(
          runner_services->Connect<fuchsia::ui::input::ImeService>()) {
  SetInterfaceErrorHandler(ime_client_, error_callback_);
  SetInterfaceErrorHandler(ime_, error_callback);
  SetInterfaceErrorHandler(text_sync_service_, error_callback);

  RegisterPlatformMessageHandlers();
}

ScenicInputHandler::~ScenicInputHandler() = default;

void ScenicInputHandler::OnScenicEvent(
    std::vector<fuchsia::ui::scenic::Event> events) {
  TRACE_EVENT0("flutter", "ScenicInputHandler::OnScenicEvent");
  for (const auto& event : events) {
    switch (event.Which()) {
      case fuchsia::ui::scenic::Event::Tag::kGfx:
        switch (event.gfx().Which()) {
          case fuchsia::ui::gfx::Event::Tag::kMetrics: {
            auto scenic_metrics = std::move(event.gfx().metrics().metrics);
            // render_dispatch_table_.metrics_changed_callback(scenic_metrics);
            UpdateViewportMetrics(std::move(scenic_metrics));
            break;
          }
          case fuchsia::ui::gfx::Event::Tag::kViewPropertiesChanged: {
            auto view_properties =
                std::move(event.gfx().view_properties_changed().properties);
            OnPropertiesChanged(std::move(view_properties));
            break;
          }
          case fuchsia::ui::gfx::Event::Tag::kViewConnected:
            render_dispatch_table_.child_view_connected_callback(
                event.gfx().view_connected().view_holder_id);
            break;
          case fuchsia::ui::gfx::Event::Tag::kViewDisconnected:
            render_dispatch_table_.child_view_disconnected_callback(
                event.gfx().view_disconnected().view_holder_id);
            break;
          case fuchsia::ui::gfx::Event::Tag::kViewStateChanged:
            render_dispatch_table_.child_view_state_changed_callback(
                event.gfx().view_state_changed().view_holder_id,
                event.gfx().view_state_changed().state.is_rendering);
            break;
          case fuchsia::ui::gfx::Event::Tag::Invalid:
            FML_DCHECK(false) << "Flutter ScenicInputHandler::OnScenicEvent: "
                                 "Got an invalid GFX event.";
            break;
          default:
            // We don't care about some event types, so not handling them is OK.
            break;
        }
        break;
      case fuchsia::ui::scenic::Event::Tag::kInput:
        switch (event.input().Which()) {
          case fuchsia::ui::input::InputEvent::Tag::kFocus: {
            OnHandleFocusEvent(event.input().focus());
            break;
          }
          case fuchsia::ui::input::InputEvent::Tag::kPointer: {
            OnHandlePointerEvent(event.input().pointer());
            break;
          }
          case fuchsia::ui::input::InputEvent::Tag::kKeyboard: {
            OnHandleKeyboardEvent(event.input().keyboard());
            break;
          }
          case fuchsia::ui::input::InputEvent::Tag::Invalid: {
            FML_DCHECK(false) << "Flutter ScenicInputHandler::OnScenicEvent: "
                                 "Got an invalid INPUT event.";
          }
        }
        break;
      default: {
        break;
      }
    }
  }
}

void ScenicInputHandler::PlatformMessageResponse(
    const FlutterPlatformMessage* message) {
  FML_DCHECK(message);
  FML_DCHECK(flutter_engine_);

  bool result = false;
  auto found = platform_message_handlers_.find(message->channel);
  if (found != platform_message_handlers_.end()) {
    result = found->second(message);
  } else {
    FML_LOG(ERROR)
        << "Platform received message on channel '" << message->channel
        << "' with no registered handler. And empty response will be "
           "generated. Please implement the native message handler.";
  }

  if (!result) {
    FlutterEngineSendPlatformMessageResponse(nullptr, message->response_handle,
                                             nullptr, 0);
  }
}

void ScenicInputHandler::UpdateSemanticsNode(const FlutterSemanticsNode* node) {
  accessibility_bridge_.AddSemanticsNodeUpdate(node);
}

void ScenicInputHandler::UpdateSemanticsCustomAction(
    const FlutterSemanticsCustomAction* action) {
  accessibility_bridge_.AddSemanticsCustomActionUpdate(action);
}

void ScenicInputHandler::DidUpdateState(
    fuchsia::ui::input::TextInputState state,
    std::unique_ptr<fuchsia::ui::input::InputEvent> input_event) {
  FML_DCHECK(flutter_engine_);

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
  auto result = FlutterEngineSendPlatformMessage(flutter_engine_, &message);
  FML_DCHECK(result == kSuccess);

  last_text_state_.emplace(std::move(state));

  // Handle keyboard input events for HID keys only.
  // TODO(SCN-1189): Are we done here?
  if (input_event && input_event->keyboard().hid_usage != 0) {
    OnHandleKeyboardEvent(input_event->keyboard());
  }
}

void ScenicInputHandler::OnAction(
    fuchsia::ui::input::InputMethodAction action) {
  FML_DCHECK(flutter_engine_);

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
  auto result = FlutterEngineSendPlatformMessage(flutter_engine_, &message);
  FML_DCHECK(result == kSuccess);
}

void ScenicInputHandler::SetSemanticsEnabled(bool enabled) {
  FML_DCHECK(flutter_engine_);

  if (enabled) {
    auto result = FlutterEngineUpdateAccessibilityFeatures(
        flutter_engine_, kFlutterAccessibilityFeatureAccessibleNavigation);
    FML_DCHECK(result == kSuccess);
  } else {
    const FlutterAccessibilityFeature kFlutterAccessibilityFeatureEmpty =
        static_cast<FlutterAccessibilityFeature>(0);
    auto result = FlutterEngineUpdateAccessibilityFeatures(
        flutter_engine_, kFlutterAccessibilityFeatureEmpty);
    FML_DCHECK(result == kSuccess);
  }
}

void ScenicInputHandler::OnPropertiesChanged(
    const fuchsia::ui::gfx::ViewProperties& view_properties) {
  FML_DCHECK(flutter_engine_);

  fuchsia::ui::gfx::BoundingBox layout_box =
      ViewPropertiesLayoutBox(view_properties);

  fuchsia::ui::gfx::vec3 logical_size =
      Max(Subtract(layout_box.max, layout_box.min), 0.f);

  current_metrics_.width = logical_size.x;
  current_metrics_.height = logical_size.y;

  auto result =
      FlutterEngineSendWindowMetricsEvent(flutter_engine_, &current_metrics_);
  FML_DCHECK(result == kSuccess);
}

void ScenicInputHandler::UpdateViewportMetrics(
    const fuchsia::ui::gfx::Metrics& metrics) {
  FML_DCHECK(flutter_engine_);

  current_metrics_.pixel_ratio = metrics.scale_x;

  auto result =
      FlutterEngineSendWindowMetricsEvent(flutter_engine_, &current_metrics_);
  FML_DCHECK(result == kSuccess);
}

bool ScenicInputHandler::OnHandlePointerEvent(
    const fuchsia::ui::input::PointerEvent& pointer) {
  FML_DCHECK(flutter_engine_);
  TRACE_EVENT0("flutter", "ScenicInputHandler::OnHandlePointerEvent");

  // TODO(SCN-1278): Use proper trace_id for tracing flow.
  trace_flow_id_t trace_id =
      PointerTraceHACK(pointer.radius_major, pointer.radius_minor);
  TRACE_FLOW_END("input", "dispatch_event_to_client", trace_id);

  FlutterPointerEvent pointer_event;
  pointer_event.struct_size = sizeof(FlutterPointerEvent);
  pointer_event.phase = GetFlutterPhaseFromFuchsiaPhase(pointer.phase);
  pointer_event.timestamp = pointer.event_time / 1000;
  pointer_event.x = pointer.x * current_metrics_.pixel_ratio;
  pointer_event.y = pointer.y * current_metrics_.pixel_ratio;
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
        FML_DLOG(ERROR) << "Received add event for down pointer.";
      }
      break;
    case FlutterPointerPhase::kRemove:
      if (down_pointers_.count(pointer_event.device) != 0) {
        FML_DLOG(ERROR) << "Received remove event for down pointer.";
      }
      break;
    case FlutterPointerPhase::kHover:
      if (down_pointers_.count(pointer_event.device) != 0) {
        FML_DLOG(ERROR) << "Received hover event for down pointer.";
      }
      break;
  }

  auto result =
      FlutterEngineSendPointerEvent(flutter_engine_, &pointer_event, 1);
  FML_DCHECK(result == kSuccess);

  return true;
}

bool ScenicInputHandler::OnHandleKeyboardEvent(
    const fuchsia::ui::input::KeyboardEvent& keyboard) {
  FML_DCHECK(flutter_engine_);

  const char* type = nullptr;
  if (keyboard.phase == fuchsia::ui::input::KeyboardEventPhase::PRESSED) {
    type = "keydown";
  } else if (keyboard.phase == fuchsia::ui::input::KeyboardEventPhase::REPEAT) {
    type = "keydown";  // TODO change this to keyrepeat
  } else if (keyboard.phase ==
             fuchsia::ui::input::KeyboardEventPhase::RELEASED) {
    type = "keyup";
  }

  if (type == nullptr) {
    FML_DLOG(ERROR) << "Unknown key event phase.";
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
  auto result = FlutterEngineSendPlatformMessage(flutter_engine_, &message);
  FML_DCHECK(result == kSuccess);

  return true;
}

bool ScenicInputHandler::OnHandleFocusEvent(
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

void ScenicInputHandler::RegisterPlatformMessageHandlers() {
  platform_message_handlers_[kFlutterPlatformChannel] = std::bind(
      &ScenicInputHandler::HandleFlutterPlatformChannelPlatformMessage, this,
      std::placeholders::_1);
  platform_message_handlers_[kTextInputChannel] = std::bind(
      &ScenicInputHandler::HandleFlutterTextInputChannelPlatformMessage, this,
      std::placeholders::_1);
  platform_message_handlers_[kAccessibilityChannel] =
      std::bind(&ScenicInputHandler::HandleAccessibilityChannelPlatformMessage,
                this, std::placeholders::_1);
  platform_message_handlers_[kFlutterPlatformViewsChannel] = std::bind(
      &ScenicInputHandler::HandleFlutterPlatformViewsChannelPlatformMessage,
      this, std::placeholders::_1);
}

bool ScenicInputHandler::HandleAccessibilityChannelPlatformMessage(
    const FlutterPlatformMessage* message) {
  FML_DCHECK(message->channel == kAccessibilityChannel);
  return false;
}

bool ScenicInputHandler::HandleFlutterPlatformChannelPlatformMessage(
    const FlutterPlatformMessage* message) {
  FML_DCHECK(message->channel == kFlutterPlatformChannel);
  return false;
}

bool ScenicInputHandler::HandleFlutterTextInputChannelPlatformMessage(
    const FlutterPlatformMessage* message) {
  FML_DCHECK(message->channel == kTextInputChannel);
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
    // TODO(abarth): Read the keyboard type from the configuration.
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
      // TODO(abarth): Deserialize state.
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
    FML_DLOG(ERROR) << "Unknown " << message->channel << " method "
                    << method->value.GetString();
  }

  // Just generate an empty response, regardless.
  return false;
}

bool ScenicInputHandler::HandleFlutterPlatformViewsChannelPlatformMessage(
    const FlutterPlatformMessage* message) {
  FML_DCHECK(message->channel == kFlutterPlatformViewsChannel);
  rapidjson::Document document;
  document.Parse(reinterpret_cast<const char*>(message->message),
                 message->message_size);
  if (document.HasParseError() || !document.IsObject()) {
    FML_LOG(ERROR) << "Could not parse document";
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
      FML_LOG(ERROR) << "No arguments found.";
      return false;
    }
    const auto& args = args_it->value;

    auto enable = args.FindMember("enable");
    if (!enable->value.IsBool()) {
      FML_LOG(ERROR) << "Argument 'enable' is not a bool";
      return false;
    }

    render_dispatch_table_.view_enable_wireframe_callback(
        enable->value.GetBool());
  } else {
    FML_DLOG(ERROR) << "Unknown " << message->channel << " method "
                    << method->value.GetString();
  }

  // Just generate an empty response, regardless.
  return false;
}

void ScenicInputHandler::ActivateIme() {
  text_sync_service_->GetInputMethodEditor(
      fuchsia::ui::input::KeyboardType::TEXT,       // keyboard type
      fuchsia::ui::input::InputMethodAction::DONE,  // input method action
      *last_text_state_,                            // initial state
      ime_client_.NewBinding(),                     // client
      ime_.NewRequest()                             // editor
  );
}

void ScenicInputHandler::DeactivateIme() {
  if (ime_) {
    text_sync_service_->HideKeyboard();
    ime_ = nullptr;
  }
  if (ime_client_.is_bound()) {
    ime_client_.Unbind();
  }
}

// flutter::PointerDataDispatcherMaker ScenicInputHandler::GetDispatcherMaker()
// {
//   return [](flutter::DefaultPointerDataDispatcher::Delegate& delegate) {
//     return std::make_unique<flutter::SmoothPointerDataDispatcher>(delegate);
//   };
// }

}  // namespace flutter_runner
