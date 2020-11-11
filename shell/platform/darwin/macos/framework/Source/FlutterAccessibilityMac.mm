// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterAccessibilityMac.h"

#include <functional>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterAppDelegate.h"

#include "flutter/third_party/accessibility/ax/platform/ax_platform_node.h"
#include "flutter/third_party/accessibility/accessibility_bridge.h"


namespace ax { // namespace

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
  bridge_ = bridge;
  ax_node_ = node;
}



void FlutterAccessibilityMac::OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event) {
  ax::AXEventGenerator::Event event_type = targeted_event.event_params.event;
  FlutterAccessibilityMac* node = this;
  gfx::NativeViewAccessible native_node = node->GetNativeViewAccessible();
  FML_DCHECK(native_node);

  NSString* mac_notification = nullptr;

  switch (event_type) {
    case ui::AXEventGenerator::Event::ACTIVE_DESCENDANT_CHANGED:
      if (node->GetRole() == ax::mojom::Role::kTree) {
        mac_notification = NSAccessibilitySelectedRowsChangedNotification;
      } else if (node->GetRole() == ax::mojom::Role::kTextFieldWithComboBox) {
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
    case ui::AXEventGenerator::Event::LOAD_COMPLETE:
      // On MacOS 10.15, firing AXLoadComplete causes focus to move to the
      // webpage and read content, despite the "Automatically speak the webpage"
      // checkbox in Voiceover utility being unchecked. The checkbox is
      // unchecked by default in 10.15 so we don't fire AXLoadComplete events to
      // support the default behavior.
      if (base::mac::IsOS10_15())
        return;
      // |NSAccessibilityLoadCompleteNotification| should only be fired on the
      // top document and when the document is not Chrome's new tab page.
      if (IsRootTree() && !IsChromeNewTabPage()) {
        mac_notification = NSAccessibilityLoadCompleteNotification;
      } else {
        // Voiceover moves focus to the web content when it receives an
        // AXLoadComplete event. On Chrome's new tab page, focus should stay
        // in the omnibox, so we purposefully do not fire the AXLoadComplete
        // event in this case.
        return;
      }
      break;
    case ui::AXEventGenerator::Event::INVALID_STATUS_CHANGED:
      mac_notification = NSAccessibilityInvalidStatusChangedNotification;
      break;
    case ui::AXEventGenerator::Event::SELECTED_CHILDREN_CHANGED:
      if (ui::IsTableLike(node->GetRole())) {
        mac_notification = NSAccessibilitySelectedRowsChangedNotification;
      } else {
        // VoiceOver does not read anything if selection changes on the
        // currently focused object, and the focus did not move. Fire a
        // selection change if the focus did not change.
        BrowserAccessibility* focus = GetFocus();
        BrowserAccessibility* container =
            focus->PlatformGetSelectionContainer();

        if (focus && node == container &&
            container->HasState(ax::mojom::State::kMultiselectable) &&
            !IsInGeneratedEventBatch(
                ui::AXEventGenerator::Event::ACTIVE_DESCENDANT_CHANGED) &&
            !IsInGeneratedEventBatch(
                ui::AXEventGenerator::Event::FOCUS_CHANGED)) {
          // Force announcement of current focus / activedescendant, even though
          // it's not changing. This way, the user can hear the new selection
          // state of the current object. Because VoiceOver ignores focus events
          // to an already focused object, this is done by destroying the native
          // object and creating a new one that receives focus.
          static_cast<BrowserAccessibilityMac*>(focus)->ReplaceNativeObject();
          // Don't fire selected children change, it will sometimes override
          // announcement of current focus.
          return;
        }

        mac_notification = NSAccessibilitySelectedChildrenChangedNotification;
      }
      break;
    case ui::AXEventGenerator::Event::DOCUMENT_SELECTION_CHANGED: {
      mac_notification = NSAccessibilitySelectedTextChangedNotification;
      // WebKit fires a notification both on the focused object and the page
      // root.
      BrowserAccessibility* focus = GetFocus();
      if (!focus)
        break;  // Just fire a notification on the root.

      if (base::mac::IsAtLeastOS10_11()) {
        // |NSAccessibilityPostNotificationWithUserInfo| should be used on OS X
        // 10.11 or later to notify Voiceover about text selection changes. This
        // API has been present on versions of OS X since 10.7 but doesn't
        // appear to be needed by Voiceover before version 10.11.
        NSDictionary* user_info =
            GetUserInfoForSelectedTextChangedNotification();

        BrowserAccessibilityManager* root_manager = GetRootManager();
        if (!root_manager)
          return;
        BrowserAccessibility* root = root_manager->GetRoot();
        if (!root)
          return;

        NSAccessibilityPostNotificationWithUserInfo(
            ToBrowserAccessibilityCocoa(focus), mac_notification, user_info);
        NSAccessibilityPostNotificationWithUserInfo(
            ToBrowserAccessibilityCocoa(root), mac_notification, user_info);
        return;
      } else {
        NSAccessibilityPostNotification(ToBrowserAccessibilityCocoa(focus),
                                        mac_notification);
      }
      break;
    }
    case ui::AXEventGenerator::Event::CHECKED_STATE_CHANGED:
      mac_notification = NSAccessibilityValueChangedNotification;
      break;
    case ui::AXEventGenerator::Event::RANGE_VALUE_CHANGED:
    case ui::AXEventGenerator::Event::SELECTED_VALUE_CHANGED:
    case ui::AXEventGenerator::Event::VALUE_IN_TEXT_FIELD_CHANGED:
      mac_notification = NSAccessibilityValueChangedNotification;
      if (base::mac::IsAtLeastOS10_11() && !text_edits_.empty()) {
        base::string16 deleted_text;
        base::string16 inserted_text;
        int32_t node_id = node->GetId();
        const auto iterator = text_edits_.find(node_id);
        id edit_text_marker = nil;
        if (iterator != text_edits_.end()) {
          AXTextEdit text_edit = iterator->second;
          deleted_text = text_edit.deleted_text;
          inserted_text = text_edit.inserted_text;
          edit_text_marker = text_edit.edit_text_marker;
        }

        NSDictionary* user_info = GetUserInfoForValueChangedNotification(
            native_node, deleted_text, inserted_text, edit_text_marker);

        BrowserAccessibility* root = GetRoot();
        if (!root)
          return;

        NSAccessibilityPostNotificationWithUserInfo(
            native_node, mac_notification, user_info);
        NSAccessibilityPostNotificationWithUserInfo(
            ToBrowserAccessibilityCocoa(root), mac_notification, user_info);
        return;
      }
      break;
    case ui::AXEventGenerator::Event::LIVE_REGION_CREATED:
      mac_notification = NSAccessibilityLiveRegionCreatedNotification;
      break;
    case ui::AXEventGenerator::Event::ALERT:
      NSAccessibilityPostNotification(
          native_node, NSAccessibilityLiveRegionCreatedNotification);
      // Voiceover requires a live region changed notification to actually
      // announce the live region.
      FireGeneratedEvent(ui::AXEventGenerator::Event::LIVE_REGION_CHANGED,
                         node);
      return;
    case ui::AXEventGenerator::Event::LIVE_REGION_CHANGED: {
      // Voiceover seems to drop live region changed notifications if they come
      // too soon after a live region created notification.
      // TODO(nektar): Limit the number of changed notifications as well.

      // if (never_suppress_or_delay_events_for_testing_) {
      //   NSAccessibilityPostNotification(
      //       native_node, NSAccessibilityLiveRegionChangedNotification);
      //   return;
      // }

      if (base::mac::IsAtMostOS10_13()) {
        // Use the announcement API to get around OS <= 10.13 VoiceOver bug
        // where it stops announcing live regions after the first time focus
        // leaves any content area.
        // Unfortunately this produces an annoying boing sound with each live
        // announcement, but the alternative is almost no live region support.
        PostAnnouncementNotification(
            base::SysUTF8ToNSString(node->GetLiveRegionText()));
        return;
      }

      // Use native VoiceOver support for live regions.
      base::scoped_nsobject<BrowserAccessibilityCocoa> retained_node(
          [native_node retain]);
      GetUIThreadTaskRunner({})->PostDelayedTask(
          FROM_HERE,
          base::BindOnce(
              [](base::scoped_nsobject<BrowserAccessibilityCocoa> node) {
                if (node && [node instanceActive]) {
                  NSAccessibilityPostNotification(
                      node, NSAccessibilityLiveRegionChangedNotification);
                }
              },
              std::move(retained_node)),
          base::TimeDelta::FromMilliseconds(kLiveRegionChangeIntervalMS));
      return;
    }
    case ui::AXEventGenerator::Event::ROW_COUNT_CHANGED:
      mac_notification = NSAccessibilityRowCountChangedNotification;
      break;
    case ui::AXEventGenerator::Event::EXPANDED:
      if (node->GetRole() == ax::mojom::Role::kRow ||
          node->GetRole() == ax::mojom::Role::kTreeItem) {
        mac_notification = NSAccessibilityRowExpandedNotification;
      } else {
        mac_notification = NSAccessibilityExpandedChanged;
      }
      break;
    case ui::AXEventGenerator::Event::COLLAPSED:
      if (node->GetRole() == ax::mojom::Role::kRow ||
          node->GetRole() == ax::mojom::Role::kTreeItem) {
        mac_notification = NSAccessibilityRowCollapsedNotification;
      } else {
        mac_notification = NSAccessibilityExpandedChanged;
      }
      break;
    case ui::AXEventGenerator::Event::MENU_ITEM_SELECTED:
      mac_notification = NSAccessibilityMenuItemSelectedNotification;
      break;
    case ui::AXEventGenerator::Event::ACCESS_KEY_CHANGED:
    case ui::AXEventGenerator::Event::ATK_TEXT_OBJECT_ATTRIBUTE_CHANGED:
    case ui::AXEventGenerator::Event::ATOMIC_CHANGED:
    case ui::AXEventGenerator::Event::AUTO_COMPLETE_CHANGED:
    case ui::AXEventGenerator::Event::BUSY_CHANGED:
    case ui::AXEventGenerator::Event::CHILDREN_CHANGED:
    case ui::AXEventGenerator::Event::CONTROLS_CHANGED:
    case ui::AXEventGenerator::Event::CLASS_NAME_CHANGED:
    case ui::AXEventGenerator::Event::DESCRIBED_BY_CHANGED:
    case ui::AXEventGenerator::Event::DESCRIPTION_CHANGED:
    case ui::AXEventGenerator::Event::DOCUMENT_TITLE_CHANGED:
    case ui::AXEventGenerator::Event::DROPEFFECT_CHANGED:
    case ui::AXEventGenerator::Event::EDITABLE_TEXT_CHANGED:
    case ui::AXEventGenerator::Event::ENABLED_CHANGED:
    case ui::AXEventGenerator::Event::FOCUS_CHANGED:
    case ui::AXEventGenerator::Event::FLOW_FROM_CHANGED:
    case ui::AXEventGenerator::Event::FLOW_TO_CHANGED:
    case ui::AXEventGenerator::Event::GRABBED_CHANGED:
    case ui::AXEventGenerator::Event::HASPOPUP_CHANGED:
    case ui::AXEventGenerator::Event::HIERARCHICAL_LEVEL_CHANGED:
    case ui::AXEventGenerator::Event::IGNORED_CHANGED:
    case ui::AXEventGenerator::Event::IMAGE_ANNOTATION_CHANGED:
    case ui::AXEventGenerator::Event::KEY_SHORTCUTS_CHANGED:
    case ui::AXEventGenerator::Event::LABELED_BY_CHANGED:
    case ui::AXEventGenerator::Event::LANGUAGE_CHANGED:
    case ui::AXEventGenerator::Event::LAYOUT_INVALIDATED:
    case ui::AXEventGenerator::Event::LIVE_REGION_NODE_CHANGED:
    case ui::AXEventGenerator::Event::LIVE_RELEVANT_CHANGED:
    case ui::AXEventGenerator::Event::LIVE_STATUS_CHANGED:
    case ui::AXEventGenerator::Event::LOAD_START:
    case ui::AXEventGenerator::Event::MULTILINE_STATE_CHANGED:
    case ui::AXEventGenerator::Event::MULTISELECTABLE_STATE_CHANGED:
    case ui::AXEventGenerator::Event::NAME_CHANGED:
    case ui::AXEventGenerator::Event::OBJECT_ATTRIBUTE_CHANGED:
    case ui::AXEventGenerator::Event::OTHER_ATTRIBUTE_CHANGED:
    case ui::AXEventGenerator::Event::PARENT_CHANGED:
    case ui::AXEventGenerator::Event::PLACEHOLDER_CHANGED:
    case ui::AXEventGenerator::Event::PORTAL_ACTIVATED:
    case ui::AXEventGenerator::Event::POSITION_IN_SET_CHANGED:
    case ui::AXEventGenerator::Event::READONLY_CHANGED:
    case ui::AXEventGenerator::Event::RELATED_NODE_CHANGED:
    case ui::AXEventGenerator::Event::REQUIRED_STATE_CHANGED:
    case ui::AXEventGenerator::Event::ROLE_CHANGED:
    case ui::AXEventGenerator::Event::SCROLL_HORIZONTAL_POSITION_CHANGED:
    case ui::AXEventGenerator::Event::SCROLL_VERTICAL_POSITION_CHANGED:
    case ui::AXEventGenerator::Event::SELECTED_CHANGED:
    case ui::AXEventGenerator::Event::SELECTION_IN_TEXT_FIELD_CHANGED:
    case ui::AXEventGenerator::Event::SET_SIZE_CHANGED:
    case ui::AXEventGenerator::Event::SORT_CHANGED:
    case ui::AXEventGenerator::Event::STATE_CHANGED:
    case ui::AXEventGenerator::Event::SUBTREE_CREATED:
    case ui::AXEventGenerator::Event::TEXT_ATTRIBUTE_CHANGED:
    case ui::AXEventGenerator::Event::RANGE_VALUE_MAX_CHANGED:
    case ui::AXEventGenerator::Event::RANGE_VALUE_MIN_CHANGED:
    case ui::AXEventGenerator::Event::RANGE_VALUE_STEP_CHANGED:
    case ui::AXEventGenerator::Event::WIN_IACCESSIBLE_STATE_CHANGED:
      // There are some notifications that aren't meaningful on Mac.
      // It's okay to skip them.
      return;
  }

  FireNativeMacNotification(mac_notification, node);
}

void FlutterAccessibilityMac::FireNativeMacNotification(
    NSString* mac_notification,
    gfx::NativeViewAccessible native_node) {
  DCHECK(mac_notification);
  DCHECK(native_node);
  NSAccessibilityPostNotification(native_node, mac_notification);
}

// AXPlatformNodeDelegateBase override
const AXNodeData& FlutterAccessibilityMac::GetData() const {
  return ax_node_->data();
}

gfx::NativeViewAccessible FlutterAccessibilityMac::GetNativeViewAccessible() {
  FML_DCHECK(ax_platform_node_);
  return ax_platform_node_->GetNativeViewAccessible();
}

gfx::NativeViewAccessible FlutterAccessibilityMac::GetParent() {
  if (!ax_node_->parent()) {
    return nullptr;
  }
  return bridge_->GetFlutterAccessibilityFromID(ax_node_->parent()->id())->GetNativeViewAccessible();
}

SkRect FlutterAccessibilityMac::GetBoundsRect(const AXCoordinateSystem coordinate_system,
                     const AXClippingBehavior clipping_behavior,
                      AXOffscreenResult* offscreen_result) const {
  // TODO: consider screen dpr and figureout what is offscreen_result.
  // switch (coordinate_system) {
  //   case AXCoordinateSystem::kScreenPhysicalPixels:
  //     NSLog(@"AXCoordinateSystem::kScreenPhysicalPixels");
  //     break;
  //   case AXCoordinateSystem::kRootFrame:
  //     NSLog(@"AXCoordinateSystem::kRootFrame");
  //     break;
  //   case AXCoordinateSystem::kFrame:
  //     NSLog(@"AXCoordinateSystem::kFrame");
  //     break;
  //   case AXCoordinateSystem::kScreenDIPs:
  //     NSLog(@"AXCoordinateSystem::kScreenDIPs");
  //     break;
  // }
  const bool clip_bounds = clipping_behavior == ax::AXClippingBehavior::kClipped;
  bool offscreen = false;
  SkRect bounds = bridge_->GetAXTree()->RelativeToTreeBounds(ax_node_, GetData().relative_bounds.bounds,
                                                      &offscreen, clip_bounds);
  // Applies window transform.
  CGRect cgBounds;
  cgBounds.origin.x = bounds.x();
  cgBounds.origin.y = bounds.y();
  cgBounds.size.width = bounds.width();
  cgBounds.size.height = bounds.height();
  FlutterEngine* engine = (__bridge FlutterEngine*)bridge_->GetUserData();
  // if (GetData().id >5 && GetData().id< 10) {
  // NSLog(@"id %d, view is %@", GetData().id, engine.viewController.view);
  // NSLog(@"cgBounds %@", NSStringFromRect(cgBounds));
  // }
  cgBounds.origin.y = -cgBounds.origin.y - cgBounds.size.height;
  // if (GetData().id >5 && GetData().id< 10) {
  // NSLog(@"after flipped %@", NSStringFromRect(cgBounds));
  // }
  CGRect view_bounds = [engine.viewController.view convertRectFromBacking:cgBounds];
  // if (GetData().id >5 && GetData().id< 10) {
  // NSLog(@"view_bounds %@", NSStringFromRect(view_bounds));
  // }
  CGRect window_bounds = [engine.viewController.view convertRect:view_bounds toView:nil];
  // NSLog(@"the contentview is %@", [[engine.viewController.view window] contentView]);
  // NSLog(@"engine.viewController.view.frame %@", NSStringFromRect(engine.viewController.view.frame));
  // CGRect viewFrame = engine.viewController.view.frame;
  // CGFloat flippedY = -view_bounds.origin.y;
  // view_bounds.origin.y = flippedY;
  // NSLog(@"after inverted %@", NSStringFromRect(view_bounds));
  // if (GetData().id >5 && GetData().id< 10) {
  // NSLog(@"window_bounds %@", NSStringFromRect(window_bounds));
  // }
  CGRect global_bounds = [[engine.viewController.view window] convertRectToScreen:window_bounds];
  // if (GetData().id >5 && GetData().id< 10) {
  // NSLog(@"global_bounds %@", NSStringFromRect(global_bounds));
  // }
  NSScreen* screen = [[NSScreen screens] firstObject];
  NSRect screen_bounds = [screen frame];
  global_bounds.origin.y =
    screen_bounds.size.height - global_bounds.origin.y - global_bounds.size.height;
  // CGRect test = [[engine.viewController.view window] convertRectToScreen:engine.viewController.view.bounds];
  // NSLog(@"screen bound %@", NSStringFromRect(test));
  // NSLog(@"engine.viewController.view accessibilityFrame screen bound %@", NSStringFromRect(engine.viewController.view.accessibilityFrame));
  // if (GetData().id >5 && GetData().id< 10) {
  // NSLog(@"global_bounds after flipped %@", NSStringFromRect(global_bounds));
  // }
  SkRect result = SkRect::MakeXYWH(global_bounds.origin.x, global_bounds.origin.y, global_bounds.size.width, global_bounds.size.height);
  return result;
  // switch (coordinate_system) {
  //   case AXCoordinateSystem::kScreenPhysicalPixels:
  //   case AXCoordinateSystem::kRootFrame:
  //   case AXCoordinateSystem::kFrame:
  //   case AXCoordinateSystem::kScreenDIPs: {
  //     // Find all parent transforms.
  //     SkMatrix global_transform = GetData().relative_bounds.transform;
  //     AXNode* parent = ax_node_->parent();
  //     while(parent) {
  //       FlutterAccessibilityMac* parent_delegate = (FlutterAccessibilityMac*)bridge_->GetFlutterAccessibilityFromID(parent->id());
  //       global_transform = parent_delegate->GetData().relative_bounds.transform * global_transform;
  //       parent = parent_delegate->ax_node_->parent();
  //     }
  //     // Applies window transform.
  //     FlutterEngine* engine = (__bridge FlutterEngine*)bridge_->GetUserData();
  //     NSAffineTransformStruct matrixStruct = [[engine getWindowTransform] transformStruct];
  //     // Assume there is no skew or rotation.
  //     SkMatrix window_transform;
  //     window_transform.setTranslateX(matrixStruct.tX);
  //     window_transform.setTranslateY(matrixStruct.tY);
  //     window_transform.setScaleX(matrixStruct.m11);
  //     window_transform.setScaleY(matrixStruct.m22);
  //     global_transform = window_transform * global_transform;
  //     SkRect result;
  //     global_transform.mapRect(&result, GetData().relative_bounds.bounds);
  //     return result;
  //   }
  // }
}

int FlutterAccessibilityMac::GetChildCount() const {
  return static_cast<int>(ax_node_->GetUnignoredChildCount());
}

gfx::NativeViewAccessible FlutterAccessibilityMac::ChildAtIndex(int index) {
  int32_t child = ax_node_->GetUnignoredChildAtIndex(index)->id();
  return bridge_->GetFlutterAccessibilityFromID(child)->GetNativeViewAccessible();
}

gfx::NativeViewAccessible FlutterAccessibilityMac::GetNSWindow() {
  FlutterAppDelegate* appDelegate = (FlutterAppDelegate*)[NSApp delegate];
  return appDelegate.mainFlutterWindow;
}

}  // namespace flutter
