// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterAccessibilityMac.h"

#include <ctype.h>
#include <future>
#include <functional>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterAppDelegate.h"

#include "flutter/third_party/accessibility/accessibility_bridge.h"
#include "flutter/third_party/accessibility/ax/ax_node_position.h"
#include "flutter/third_party/accessibility/ax/ax_action_data.h"
#include "flutter/third_party/accessibility/ax/platform/ax_platform_node.h"
#include "flutter/third_party/accessibility/ax/platform/ax_platform_node_base.h"

using AXTextMarkerRangeRef = CFTypeRef;
using AXTextMarkerRef = CFTypeRef;

namespace ax { // namespace

bool SystemVersionEqualTo(NSOperatingSystemVersion target) {
  NSOperatingSystemVersion systemVersion = [[NSProcessInfo processInfo] operatingSystemVersion];
  return target.majorVersion == systemVersion.majorVersion && target.minorVersion == systemVersion.minorVersion;
}

bool SystemVersionGreaterOrEqualTo(NSOperatingSystemVersion target) {
  NSOperatingSystemVersion systemVersion = [[NSProcessInfo processInfo] operatingSystemVersion];
  if (systemVersion.majorVersion < target.majorVersion) {
    return false;
  }
  else if (systemVersion.majorVersion > target.majorVersion) {
    return true;
  }
  return systemVersion.minorVersion >= target.minorVersion;
}

bool SystemVersionLessOrEqualTo(NSOperatingSystemVersion target) {
  NSOperatingSystemVersion systemVersion = [[NSProcessInfo processInfo] operatingSystemVersion];
  if (systemVersion.majorVersion < target.majorVersion) {
    return true;
  }
  else if (systemVersion.majorVersion > target.majorVersion) {
    return false;
  }
  return systemVersion.minorVersion <= target.minorVersion;
}

// Native mac notifications fired.
NSString* const NSAccessibilityAutocorrectionOccurredNotification =
    @"AXAutocorrectionOccurred";
NSString* const NSAccessibilityLoadCompleteNotification = @"AXLoadComplete";
NSString* const NSAccessibilityInvalidStatusChangedNotification =
    @"AXInvalidStatusChanged";
NSString* const NSAccessibilityLiveRegionCreatedNotification =
    @"AXLiveRegionCreated";
NSString* const NSAccessibilityLiveRegionChangedNotification =
    @"AXLiveRegionChanged";
NSString* const NSAccessibilityExpandedChanged = @"AXExpandedChanged";
NSString* const NSAccessibilityMenuItemSelectedNotification =
    @"AXMenuItemSelected";

// Attributes used for NSAccessibilitySelectedTextChangedNotification and
// NSAccessibilityValueChangedNotification.
NSString* const NSAccessibilityTextStateChangeTypeKey =
    @"AXTextStateChangeType";
NSString* const NSAccessibilityTextStateSyncKey = @"AXTextStateSync";
NSString* const NSAccessibilityTextChangeElement = @"AXTextChangeElement";
NSString* const NSAccessibilityTextEditType = @"AXTextEditType";
NSString* const NSAccessibilityTextChangeValue = @"AXTextChangeValue";
NSString* const NSAccessibilityChangeValueStartMarker =
    @"AXTextChangeValueStartMarker";
NSString* const NSAccessibilityTextChangeValueLength =
    @"AXTextChangeValueLength";
NSString* const NSAccessibilityTextChangeValues = @"AXTextChangeValues";

enum AXTextStateChangeType {
  AXTextStateChangeTypeUnknown,
  AXTextStateChangeTypeEdit,
  AXTextStateChangeTypeSelectionMove,
  AXTextStateChangeTypeSelectionExtend
};

enum AXTextEditType {
  AXTextEditTypeUnknown,
  AXTextEditTypeDelete,
  AXTextEditTypeInsert,
  AXTextEditTypeTyping,
  AXTextEditTypeDictation,
  AXTextEditTypeCut,
  AXTextEditTypePaste,
  AXTextEditTypeAttributesChange
};

const int kLiveRegionChangeIntervalMS = 20;

extern "C" {

// The following are private accessibility APIs required for cursor navigation
// and text selection. VoiceOver started relying on them in Mac OS X 10.11.

AXTextMarkerRef AXTextMarkerCreate(CFAllocatorRef allocator,
                                   const UInt8* bytes,
                                   CFIndex length);


}  // extern "C"

// Static.
FlutterAccessibility* FlutterAccessibility::Create() {
  return new FlutterAccessibilityMac();
}

FlutterAccessibilityMac::FlutterAccessibilityMac() {
  ax_platform_node_ = AXPlatformNode::Create(this);
  FML_DCHECK(ax_platform_node_);
}


FlutterAccessibilityMac::~FlutterAccessibilityMac() {
  ax_platform_node_->Destroy();
}

void FlutterAccessibilityMac::Init(AccessibilityBridge* bridge, AXNode* node) {
  FlutterAccessibility::Init(bridge, node);
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
  old_text_editing_value_ = convert.from_bytes(GetAXNode()->GetValueForControl());
}

void FlutterAccessibilityMac::OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event) {
  FML_DCHECK(targeted_event.node == GetAXNode());
  ax::AXEventGenerator::Event event_type = targeted_event.event_params.event;
  gfx::NativeViewAccessible native_node = GetNativeViewAccessible();
  FML_DCHECK(native_node);

  NSString* mac_notification = nil;

  switch (event_type) {
    case ax::AXEventGenerator::Event::ACTIVE_DESCENDANT_CHANGED:
      if (GetData().role == ax::Role::kTree) {
        mac_notification = NSAccessibilitySelectedRowsChangedNotification;
      } else if (GetData().role == ax::Role::kTextFieldWithComboBox) {
        // Even though the selected item in the combo box has changed, we don't
        // want to post a focus change because this will take the focus out of
        // the combo box where the user might be typing.
        mac_notification = NSAccessibilitySelectedChildrenChangedNotification;
      } else {
        // In all other cases we should post
        // |NSAccessibilityFocusedUIElementChangedNotification|, but this is
        // handled elsewhere.
        return;
      }
      break;
    case ax::AXEventGenerator::Event::LOAD_COMPLETE:
      // On MacOS 10.15, firing AXLoadComplete causes focus to move to the
      // webpage and read content, despite the "Automatically speak the webpage"
      // checkbox in Voiceover utility being unchecked. The checkbox is
      // unchecked by default in 10.15 so we don't fire AXLoadComplete events to
      // support the default behavior.
      if (SystemVersionEqualTo({ .majorVersion = 10, .minorVersion = 15 }))
        return;
      mac_notification = NSAccessibilityLoadCompleteNotification;
      break;
    case ax::AXEventGenerator::Event::INVALID_STATUS_CHANGED:
      mac_notification = NSAccessibilityInvalidStatusChangedNotification;
      break;
    case ax::AXEventGenerator::Event::SELECTED_CHILDREN_CHANGED:
      if (ax::IsTableLike(GetData().role)) {
        mac_notification = NSAccessibilitySelectedRowsChangedNotification;
      } else {
        // VoiceOver does not read anything if selection changes on the
        // currently focused object, and the focus did not move. Fire a
        // selection change if the focus did not change.
        NSAccessibilityElement* native_accessibility_node = (NSAccessibilityElement*)native_node;
        if (native_accessibility_node.accessibilityFocusedUIElement &&
            GetData().HasState(ax::State::kMultiselectable) &&
            !IsInGeneratedEventBatch(
                ax::AXEventGenerator::Event::ACTIVE_DESCENDANT_CHANGED) &&
            !IsInGeneratedEventBatch(
                ax::AXEventGenerator::Event::FOCUS_CHANGED)) {
          // Force announcement of current focus / activedescendant, even though
          // it's not changing. This way, the user can hear the new selection
          // state of the current object. Because VoiceOver ignores focus events
          // to an already focused object, this is done by destroying the native
          // object and creating a new one that receives focus.

          // static_cast<BrowserAccessibilityMac*>(focus)->ReplaceNativeObject();

          // Don't fire selected children change, it will sometimes override
          // announcement of current focus.
          return;
        }

        mac_notification = NSAccessibilitySelectedChildrenChangedNotification;
      }
      break;
    case ax::AXEventGenerator::Event::DOCUMENT_SELECTION_CHANGED: {
      // This event always fires at root
      mac_notification = NSAccessibilitySelectedTextChangedNotification;
      // WebKit fires a notification both on the focused object and the page
      // root.
      const AXTreeData& tree_data = GetBridge()->GetAXTree()->data();
      int32_t focus = tree_data.focus_id;
      if (focus == AXNode::kInvalidAXID || focus != tree_data.sel_anchor_object_id)
        break; // Just fire a notification on the root.
      FlutterAccessibility* focus_node = GetBridge()->GetFlutterAccessibilityFromID(focus);
      if (!focus_node)
        break; // Just fire a notification on the root.
      FireNativeMacNotification(focus_node->GetNativeViewAccessible(),
                                        mac_notification);
      break;
    }
    case ax::AXEventGenerator::Event::CHECKED_STATE_CHANGED:
      mac_notification = NSAccessibilityValueChangedNotification;
      break;
    case ax::AXEventGenerator::Event::RANGE_VALUE_CHANGED:
    case ax::AXEventGenerator::Event::SELECTED_VALUE_CHANGED:
    case ax::AXEventGenerator::Event::VALUE_IN_TEXT_FIELD_CHANGED:
      mac_notification = NSAccessibilityValueChangedNotification;
      if (SystemVersionGreaterOrEqualTo({ .majorVersion = 10, .minorVersion = 11 }) && GetData().HasState(ax::State::kEditable)) {
        std::u16string deleted_text;
        std::u16string inserted_text;
        id edit_text_marker = nil;
        computeTextEdit(deleted_text, inserted_text, &edit_text_marker);
        NSDictionary* user_info = GetUserInfoForValueChangedNotification(
            native_node, deleted_text, inserted_text, edit_text_marker);
        if (user_info) {
          FireNativeMacNotificationWithUserInfo(
              native_node, mac_notification, user_info);
          FireNativeMacNotificationWithUserInfo(
              GetBridge()->GetFlutterAccessibilityFromID(kRootNode)->GetNativeViewAccessible(), mac_notification, user_info);
        }
        return;
      }
      break;
    case ax::AXEventGenerator::Event::LIVE_REGION_CREATED:
      mac_notification = NSAccessibilityLiveRegionCreatedNotification;
      break;
    case ax::AXEventGenerator::Event::ALERT: {
      FireNativeMacNotification(
          native_node, NSAccessibilityLiveRegionCreatedNotification);
      // Voiceover requires a live region changed notification to actually
      // announce the live region.
      AXEventGenerator::EventParams event_params(ax::AXEventGenerator::Event::LIVE_REGION_CHANGED,
                ax::EventFrom::kNone,
                {});
      AXEventGenerator::TargetedEvent live_region(GetAXNode(), event_params);
      OnAccessibilityEvent(live_region);
      return;
    }
    case ax::AXEventGenerator::Event::LIVE_REGION_CHANGED: {
      if (SystemVersionLessOrEqualTo({ .majorVersion = 10, .minorVersion = 13 })) {
        // Use the announcement API to get around OS <= 10.13 VoiceOver bug
        // where it stops announcing live regions after the first time focus
        // leaves any content area.
        // Unfortunately this produces an annoying boing sound with each live
        // announcement, but the alternative is almost no live region support.
        NSString* announcement = [[NSString alloc] initWithUTF8String:GetLiveRegionText().c_str()];
        NSDictionary* notification_info = @{
          NSAccessibilityAnnouncementKey : announcement,
          NSAccessibilityPriorityKey : @(NSAccessibilityPriorityLow)
        };
        // Trigger VoiceOver speech and show on Braille display, if available.
        // The Braille will only appear for a few seconds, and then will be replaced
        // with the previous announcement.
        FireNativeMacNotificationWithUserInfo(
            [NSApp mainWindow], NSAccessibilityAnnouncementRequestedNotification,
            notification_info);
        return;
      }

      // Use native VoiceOver support for live regions.
      
      std::async(std::launch::async, [&native_node, this] () {
          // Use sleep_for to wait specified time (or sleep_until).
          std::this_thread::sleep_for( std::chrono::milliseconds{kLiveRegionChangeIntervalMS});
          FireNativeMacNotification(
              native_node, NSAccessibilityLiveRegionChangedNotification);
      });
      return;
    }
    case ax::AXEventGenerator::Event::ROW_COUNT_CHANGED:
      mac_notification = NSAccessibilityRowCountChangedNotification;
      break;
    case ax::AXEventGenerator::Event::EXPANDED:
      if (GetData().role == ax::Role::kRow ||
          GetData().role == ax::Role::kTreeItem) {
        mac_notification = NSAccessibilityRowExpandedNotification;
      } else {
        mac_notification = NSAccessibilityExpandedChanged;
      }
      break;
    case ax::AXEventGenerator::Event::COLLAPSED:
      if (GetData().role == ax::Role::kRow ||
          GetData().role == ax::Role::kTreeItem) {
        mac_notification = NSAccessibilityRowCollapsedNotification;
      } else {
        mac_notification = NSAccessibilityExpandedChanged;
      }
      break;
    case ax::AXEventGenerator::Event::MENU_ITEM_SELECTED:
      mac_notification = NSAccessibilityMenuItemSelectedNotification;
      break;
    case ax::AXEventGenerator::Event::CHILDREN_CHANGED: {
      // NSAccessibilityCreatedNotification seems to be the only way to let
      // Voiceover pick up layout changes.
      FireNativeMacNotification([NSApp mainWindow], NSAccessibilityCreatedNotification);
      return;
    }
    case ax::AXEventGenerator::Event::SUBTREE_CREATED:
    case ax::AXEventGenerator::Event::ACCESS_KEY_CHANGED:
    case ax::AXEventGenerator::Event::ATK_TEXT_OBJECT_ATTRIBUTE_CHANGED:
    case ax::AXEventGenerator::Event::ATOMIC_CHANGED:
    case ax::AXEventGenerator::Event::AUTO_COMPLETE_CHANGED:
    case ax::AXEventGenerator::Event::BUSY_CHANGED:
    case ax::AXEventGenerator::Event::CONTROLS_CHANGED:
    case ax::AXEventGenerator::Event::CLASS_NAME_CHANGED:
    case ax::AXEventGenerator::Event::DESCRIBED_BY_CHANGED:
    case ax::AXEventGenerator::Event::DESCRIPTION_CHANGED:
    case ax::AXEventGenerator::Event::DOCUMENT_TITLE_CHANGED:
    case ax::AXEventGenerator::Event::DROPEFFECT_CHANGED:
    case ax::AXEventGenerator::Event::EDITABLE_TEXT_CHANGED:
    case ax::AXEventGenerator::Event::ENABLED_CHANGED:
    case ax::AXEventGenerator::Event::FOCUS_CHANGED:
    case ax::AXEventGenerator::Event::FLOW_FROM_CHANGED:
    case ax::AXEventGenerator::Event::FLOW_TO_CHANGED:
    case ax::AXEventGenerator::Event::GRABBED_CHANGED:
    case ax::AXEventGenerator::Event::HASPOPUP_CHANGED:
    case ax::AXEventGenerator::Event::HIERARCHICAL_LEVEL_CHANGED:
    case ax::AXEventGenerator::Event::IGNORED_CHANGED:
    case ax::AXEventGenerator::Event::IMAGE_ANNOTATION_CHANGED:
    case ax::AXEventGenerator::Event::KEY_SHORTCUTS_CHANGED:
    case ax::AXEventGenerator::Event::LABELED_BY_CHANGED:
    case ax::AXEventGenerator::Event::LANGUAGE_CHANGED:
    case ax::AXEventGenerator::Event::LAYOUT_INVALIDATED:
    case ax::AXEventGenerator::Event::LIVE_REGION_NODE_CHANGED:
    case ax::AXEventGenerator::Event::LIVE_RELEVANT_CHANGED:
    case ax::AXEventGenerator::Event::LIVE_STATUS_CHANGED:
    case ax::AXEventGenerator::Event::LOAD_START:
    case ax::AXEventGenerator::Event::MULTILINE_STATE_CHANGED:
    case ax::AXEventGenerator::Event::MULTISELECTABLE_STATE_CHANGED:
    case ax::AXEventGenerator::Event::NAME_CHANGED:
    case ax::AXEventGenerator::Event::OBJECT_ATTRIBUTE_CHANGED:
    case ax::AXEventGenerator::Event::OTHER_ATTRIBUTE_CHANGED:
    case ax::AXEventGenerator::Event::PARENT_CHANGED:
    case ax::AXEventGenerator::Event::PLACEHOLDER_CHANGED:
    case ax::AXEventGenerator::Event::PORTAL_ACTIVATED:
    case ax::AXEventGenerator::Event::POSITION_IN_SET_CHANGED:
    case ax::AXEventGenerator::Event::READONLY_CHANGED:
    case ax::AXEventGenerator::Event::RELATED_NODE_CHANGED:
    case ax::AXEventGenerator::Event::REQUIRED_STATE_CHANGED:
    case ax::AXEventGenerator::Event::ROLE_CHANGED:
    case ax::AXEventGenerator::Event::SCROLL_HORIZONTAL_POSITION_CHANGED:
    case ax::AXEventGenerator::Event::SCROLL_VERTICAL_POSITION_CHANGED:
    case ax::AXEventGenerator::Event::SELECTED_CHANGED:
    case ax::AXEventGenerator::Event::SELECTION_IN_TEXT_FIELD_CHANGED:
    case ax::AXEventGenerator::Event::SET_SIZE_CHANGED:
    case ax::AXEventGenerator::Event::SORT_CHANGED:
    case ax::AXEventGenerator::Event::STATE_CHANGED:
    case ax::AXEventGenerator::Event::TEXT_ATTRIBUTE_CHANGED:
    case ax::AXEventGenerator::Event::RANGE_VALUE_MAX_CHANGED:
    case ax::AXEventGenerator::Event::RANGE_VALUE_MIN_CHANGED:
    case ax::AXEventGenerator::Event::RANGE_VALUE_STEP_CHANGED:
    case ax::AXEventGenerator::Event::WIN_IACCESSIBLE_STATE_CHANGED:
      // There are some notifications that aren't meaningful on Mac.
      // It's okay to skip them.
      return;
  }
  FireNativeMacNotification(native_node, mac_notification);
}

void FlutterAccessibilityMac::FireNativeMacNotification(
    gfx::NativeViewAccessible native_node,
    NSString* mac_notification) {
  FML_DCHECK(mac_notification);
  // FML_DCHECK(native_node);
  NSAccessibilityPostNotification(native_node, mac_notification);
}

void FlutterAccessibilityMac::FireNativeMacNotificationWithUserInfo(
    gfx::NativeViewAccessible native_node,
    NSString* mac_notification,
    NSDictionary* user_info) {
  FML_DCHECK(mac_notification);
  FML_DCHECK(native_node);
  FML_DCHECK(user_info);
  NSAccessibilityPostNotificationWithUserInfo(native_node, mac_notification, user_info);
}

bool FlutterAccessibilityMac::IsInGeneratedEventBatch(
    ax::AXEventGenerator::Event event_type) const {
  for (const auto& event : *(GetBridge()->GetEventGenerator())) {
    if (event.event_params.event == event_type)
      return true;  // Any side effects will have already been handled.
  }
  return false;
}

// FlutterAccessibilityMac override
void FlutterAccessibilityMac::DispatchAccessibilityAction(uint16_t target, FlutterSemanticsAction action, uint8_t* data, size_t data_size) {
  [GetFlutterEngine() dispatchSemanticsAction:target
                                       action:action
                                         data:data
                                     dataSize:data_size];
}

gfx::NativeViewAccessible FlutterAccessibilityMac::GetNativeViewAccessible() {
  FML_DCHECK(ax_platform_node_);
  return ax_platform_node_->GetNativeViewAccessible();
}

gfx::NativeViewAccessible FlutterAccessibilityMac::GetParent() {
  gfx::NativeViewAccessible parent = FlutterAccessibility::GetParent();
  if (!parent) {
    return GetFlutterEngine().viewController.view;
  }
  return parent;
}

SkRect FlutterAccessibilityMac::GetBoundsRect(const AXCoordinateSystem coordinate_system,
                     const AXClippingBehavior clipping_behavior,
                      AXOffscreenResult* offscreen_result) const {
  // TODO: consider screen dpr.
  const bool clip_bounds = clipping_behavior == ax::AXClippingBehavior::kClipped;
  bool offscreen = false;
  SkRect bounds = GetBridge()->GetAXTree()->RelativeToTreeBounds(GetAXNode(), GetData().relative_bounds.bounds,
                                                      &offscreen, clip_bounds);
  // Applies window transform.
  NSRect local_bound;
  local_bound.origin.x = bounds.x();
  local_bound.origin.y = bounds.y();
  local_bound.size.width = bounds.width();
  local_bound.size.height = bounds.height();
  FlutterEngine* engine = GetFlutterEngine();
  // local_bounds is flipped.
  local_bound.origin.y = -local_bound.origin.y - local_bound.size.height;

  NSRect view_bounds = [engine.viewController.view convertRectFromBacking:local_bound];
  NSRect window_bounds = [engine.viewController.view convertRect:view_bounds toView:nil];
  NSRect global_bounds = [[engine.viewController.view window] convertRectToScreen:window_bounds];
  // NSWindow is flipped.
  NSScreen* screen = [[NSScreen screens] firstObject];
  NSRect screen_bounds = [screen frame];
  global_bounds.origin.y =
    screen_bounds.size.height - global_bounds.origin.y - global_bounds.size.height;
  SkRect result = SkRect::MakeXYWH(global_bounds.origin.x, global_bounds.origin.y, global_bounds.size.width, global_bounds.size.height);
  return result;
}

// Private method
std::string FlutterAccessibilityMac::GetLiveRegionText() const {
  if (GetAXNode()->IsIgnored())
    return "";

  std::string text = GetData().GetStringAttribute(ax::StringAttribute::kName);
  if (!text.empty())
    return text;

  for (int32_t child : GetData().child_ids) {
    const FlutterAccessibilityMac* accessibility_child = (FlutterAccessibilityMac*)GetBridge()->GetFlutterAccessibilityFromID(child);
    if (!accessibility_child)
      continue;

    text += accessibility_child->GetLiveRegionText();
  }
  return text;
}


void FlutterAccessibilityMac::computeTextEdit(std::u16string& inserted_text, std::u16string& deleted_text, id* edit_text_marker) {
  if (!GetData().IsTextField())
    return;

  // Starting from macOS 10.11, if the user has edited some text we need to
  // dispatch the actual text that changed on the value changed notification.
  // We run this code on all macOS versions to get the highest test coverage.
  std::u16string old_value = old_text_editing_value_;
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
  std::u16string new_value = convert.from_bytes(GetAXNode()->GetValueForControl());
  old_text_editing_value_ = new_value;
  if (old_value.empty() && new_value.empty())
    return;

  size_t i;
  size_t j;
  // Sometimes Blink doesn't use the same UTF16 characters to represent
  // whitespace.
  for (i = 0;
       i < old_value.length() && i < new_value.length() &&
       (old_value[i] == new_value[i] || (iswspace(old_value[i]) &&
                                       iswspace(new_value[i])));
       ++i) {
  }
  for (j = 0;
       (i + j) < old_value.length() && (i + j) < new_value.length() &&
       (old_value[old_value.length() - j - 1] ==
            new_value[new_value.length() - j - 1] ||
        (iswspace(old_value[old_value.length() - j - 1]) &&
         iswspace(new_value[new_value.length() - j - 1])));
       ++j) {
  }
  FML_DCHECK(i + j <= old_value.length());
  FML_DCHECK(i + j <= new_value.length());

  std::u16string current_deleted_text = old_value.substr(i, old_value.length() - i - j);
  std::u16string current_inserted_text = new_value.substr(i, new_value.length() - i - j);

  // Heuristic for editable combobox. If more than 1 character is inserted or
  // deleted, and the caret is at the end of the field, assume the entire text
  // field changed.
  // TODO(nektar) Remove this once editing intents are implemented,
  // and the actual inserted and deleted text is passed over from Blink.
  if (GetData().role == ax::Role::kTextFieldWithComboBox &&
      (current_deleted_text.length() > 1 || current_inserted_text.length() > 1)) {
    int sel_start, sel_end;
    GetData().GetIntAttribute(ax::IntAttribute::kTextSelStart, &sel_start);
    GetData().GetIntAttribute(ax::IntAttribute::kTextSelEnd, &sel_end);
    if (static_cast<size_t>(sel_start) == new_value.length() &&
        static_cast<size_t>(sel_end) == new_value.length()) {
      // Don't include old_value as it would be announced -- very confusing.
      inserted_text = new_value;
      return;
    }
  }
  inserted_text = current_inserted_text;
  deleted_text = current_deleted_text;
  // Creates text marker
  AXNodePosition::AXPositionInstance position = AXNodePosition::CreateTextPosition(
      GetAXNode()->tree()->GetAXTreeID(), GetAXNode()->id(), i,  ax::TextAffinity::kDownstream);
  AXNodePosition::SerializedPosition serialized = position->Serialize();
  AXTextMarkerRef cf_text_marker = AXTextMarkerCreate(
      kCFAllocatorDefault, reinterpret_cast<const UInt8*>(&serialized),
      sizeof(AXNodePosition::SerializedPosition));
  *edit_text_marker = (__bridge id)(cf_text_marker);
  return;
}

NSDictionary*
FlutterAccessibilityMac::GetUserInfoForValueChangedNotification(
    const id native_node,
    const std::u16string& deleted_text,
    const std::u16string& inserted_text,
    id edit_text_marker) {
  if (deleted_text.empty() && inserted_text.empty())
    return nil;

  NSMutableArray* changes = [[NSMutableArray alloc] init];
  if (!deleted_text.empty()) {
    const unichar* p_u16chars = (const unichar *) deleted_text.c_str();
    NSMutableDictionary* change =
        [NSMutableDictionary dictionaryWithDictionary:@{
          NSAccessibilityTextEditType: @(AXTextEditTypeDelete),
          NSAccessibilityTextChangeValueLength: @(deleted_text.length()),
          NSAccessibilityTextChangeValue: [NSString stringWithCharacters:p_u16chars length:deleted_text.size()]
        }];
    if (edit_text_marker) {
      change[NSAccessibilityChangeValueStartMarker] = edit_text_marker;
    }
    [changes addObject:change];
  }
  if (!inserted_text.empty()) {
    // TODO(nektar): Figure out if this is a paste, insertion or typing.
    // Changes to Blink would be required. A heuristic is currently used.
    auto edit_type = inserted_text.length() > 1 ? @(AXTextEditTypeInsert)
                                                : @(AXTextEditTypeTyping);
    const unichar* p_u16chars = (const unichar *) inserted_text.c_str();
    NSMutableDictionary* change =
        [NSMutableDictionary dictionaryWithDictionary:@{
          NSAccessibilityTextEditType: edit_type,
          NSAccessibilityTextChangeValueLength: @(inserted_text.length()),
          NSAccessibilityTextChangeValue: [NSString stringWithCharacters:p_u16chars length:inserted_text.size()]
        }];
    if (edit_text_marker) {
      change[NSAccessibilityChangeValueStartMarker] = edit_text_marker;
    }
    [changes addObject:change];
  }

  return @{
    NSAccessibilityTextStateChangeTypeKey: @(AXTextStateChangeTypeEdit),
    NSAccessibilityTextChangeValues: changes,
    NSAccessibilityTextChangeElement: native_node,
  };
}

FlutterEngine* FlutterAccessibilityMac::GetFlutterEngine() const {
  return (__bridge FlutterEngine*)GetBridge()->GetUserData();
}

gfx::NativeViewAccessible FlutterAccessibilityMac::GetNSWindow() {
  FlutterAppDelegate* appDelegate = (FlutterAppDelegate*)[NSApp delegate];
  return appDelegate.mainFlutterWindow;
}

}  // namespace flutter
