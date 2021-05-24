// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/accessibility_bridge.h"

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterViewController_Internal.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/accessibility_text_entry.h"
#import "flutter/shell/platform/darwin/ios/platform_view_ios.h"

#pragma GCC diagnostic error "-Wundeclared-selector"

FLUTTER_ASSERT_NOT_ARC

namespace flutter {
namespace {

constexpr int32_t kSemanticObjectIdInvalid = -1;

class DefaultIosDelegate : public AccessibilityBridge::IosDelegate {
 public:
  bool IsFlutterViewControllerPresentingModalViewController(
      FlutterViewController* view_controller) override {
    if (view_controller) {
      return view_controller.isPresentingViewController;
    } else {
      return false;
    }
  }

  void PostAccessibilityNotification(UIAccessibilityNotifications notification,
                                     id argument) override {
    UIAccessibilityPostNotification(notification, argument);
  }
};
}  // namespace

AccessibilityBridge::AccessibilityBridge(
    FlutterViewController* view_controller,
    PlatformViewIOS* platform_view,
    std::shared_ptr<FlutterPlatformViewsController> platform_views_controller,
    std::unique_ptr<IosDelegate> ios_delegate)
    : view_controller_(view_controller),
      platform_view_(platform_view),
      platform_views_controller_(platform_views_controller),
      last_focused_semantics_object_id_(kSemanticObjectIdInvalid),
      objects_([[NSMutableDictionary alloc] init]),
      weak_factory_(this),
      previous_route_id_(0),
      previous_routes_({}),
      ios_delegate_(ios_delegate ? std::move(ios_delegate)
                                 : std::make_unique<DefaultIosDelegate>()) {
  accessibility_channel_.reset([[FlutterBasicMessageChannel alloc]
         initWithName:@"flutter/accessibility"
      binaryMessenger:platform_view->GetOwnerViewController().get().engine.binaryMessenger
                codec:[FlutterStandardMessageCodec sharedInstance]]);
  [accessibility_channel_.get() setMessageHandler:^(id message, FlutterReply reply) {
    HandleEvent((NSDictionary*)message);
  }];
}

AccessibilityBridge::~AccessibilityBridge() {
  [accessibility_channel_.get() setMessageHandler:nil];
  clearState();
  view_controller_.view.accessibilityElements = nil;
}

UIView<UITextInput>* AccessibilityBridge::textInputView() {
  return [[platform_view_->GetOwnerViewController().get().engine textInputPlugin] textInputView];
}

void AccessibilityBridge::AccessibilityObjectDidBecomeFocused(int32_t id) {
  last_focused_semantics_object_id_ = id;
}

void AccessibilityBridge::AccessibilityObjectDidLoseFocus(int32_t id) {
  if (last_focused_semantics_object_id_ == id) {
    last_focused_semantics_object_id_ = kSemanticObjectIdInvalid;
  }
}

void AccessibilityBridge::UpdateSemantics(flutter::SemanticsNodeUpdates nodes,
                                          flutter::CustomAccessibilityActionUpdates actions) {
  BOOL layoutChanged = NO;
  BOOL scrollOccured = NO;
  BOOL needsAnnouncement = NO;
  for (const auto& entry : actions) {
    const flutter::CustomAccessibilityAction& action = entry.second;
    actions_[action.id] = action;
  }
  for (const auto& entry : nodes) {
    const flutter::SemanticsNode& node = entry.second;
    SemanticsObject* object = GetOrCreateObject(node.id, nodes);
    layoutChanged = layoutChanged || [object nodeWillCauseLayoutChange:&node];
    scrollOccured = scrollOccured || [object nodeWillCauseScroll:&node];
    needsAnnouncement = [object nodeShouldTriggerAnnouncement:&node];
    [object setSemanticsNode:&node];
    NSUInteger newChildCount = node.childrenInTraversalOrder.size();
    NSMutableArray* newChildren =
        [[[NSMutableArray alloc] initWithCapacity:newChildCount] autorelease];
    for (NSUInteger i = 0; i < newChildCount; ++i) {
      SemanticsObject* child = GetOrCreateObject(node.childrenInTraversalOrder[i], nodes);
      [newChildren addObject:child];
    }
    object.children = newChildren;
    action_overrides_of_semantics_objects_.erase([object uid]);

    if (node.customAccessibilityActions.size() > 0) {
      NSMutableArray<FlutterCustomAccessibilityAction*>* accessibilityCustomActions =
          [[[NSMutableArray alloc] init] autorelease];
      std::vector<FlutterCustomAccessibilityAction*> action_overrides;
      for (int32_t action_id : node.customAccessibilityActions) {
        flutter::CustomAccessibilityAction& action = actions_[action_id];
        if (action.label.empty())
          continue;
        NSString* label = @(action.label.data());
        SEL selector = @selector(onCustomAccessibilityAction:);
        FlutterCustomAccessibilityAction* customAction =
            [[[FlutterCustomAccessibilityAction alloc] initWithName:label
                                                             target:object
                                                           selector:selector] autorelease];

        if (action.overrideId == -1) {
          customAction.uid = action_id;
          customAction.overrideActionId = -1;
          [accessibilityCustomActions addObject:customAction];
        } else if (view_controller_.switchControlEnabled) {
          // Action overrides are meant to make scrolling action shows up in the context menu
          // when the SwitchControl is enabled.
          customAction.uid = action_id;
          customAction.overrideActionId = action.overrideId;
          action_overrides.push_back(customAction);
        }
      }
      // The accessibilityCustomActions of the object will be set in the
      // MarkAttendanceAndSetCustomActions.
      if ([accessibilityCustomActions count] > 0)
        object.customSemanticsActions = accessibilityCustomActions;
      if (!action_overrides.empty())
        // The action overrides will be apply to its descendants and will not be
        // added to this semantics object.
        action_overrides_of_semantics_objects_[[object uid]] = action_overrides;
    }

    if (object.node.IsPlatformViewNode()) {
      auto controller = GetPlatformViewsController();
      if (controller) {
        object.platformViewSemanticsContainer = [[[FlutterPlatformViewSemanticsContainer alloc]
            initWithSemanticsObject:object] autorelease];
      }
    } else if (object.platformViewSemanticsContainer) {
      object.platformViewSemanticsContainer = nil;
    }
    if (needsAnnouncement) {
      // Try to be more polite - iOS 11+ supports
      // UIAccessibilitySpeechAttributeQueueAnnouncement which should avoid
      // interrupting system notifications or other elements.
      // Expectation: roughly match the behavior of polite announcements on
      // Android.
      NSString* announcement =
          [[[NSString alloc] initWithUTF8String:object.node.label.c_str()] autorelease];
      if (@available(iOS 11.0, *)) {
        UIAccessibilityPostNotification(
            UIAccessibilityAnnouncementNotification,
            [[[NSAttributedString alloc]
                initWithString:announcement
                    attributes:@{
                      UIAccessibilitySpeechAttributeQueueAnnouncement : @YES
                    }] autorelease]);
      } else {
        UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, announcement);
      }
    }
  }

  SemanticsObject* root = objects_.get()[@(kRootNodeId)];

  bool routeChanged = false;
  SemanticsObject* lastAdded = nil;

  if (root) {
    if (!view_controller_.view.accessibilityElements) {
      view_controller_.view.accessibilityElements = @[ [root accessibilityContainer] ];
    }
    NSMutableArray<SemanticsObject*>* newRoutes = [[[NSMutableArray alloc] init] autorelease];
    [root collectRoutes:newRoutes];
    // Finds the last route that is not in the previous routes.
    for (SemanticsObject* route in newRoutes) {
      if (std::find(previous_routes_.begin(), previous_routes_.end(), [route uid]) ==
          previous_routes_.end()) {
        lastAdded = route;
      }
    }
    // If all the routes are in the previous route, get the last route.
    if (lastAdded == nil && [newRoutes count] > 0) {
      int index = [newRoutes count] - 1;
      lastAdded = [newRoutes objectAtIndex:index];
    }
    // There are two cases if lastAdded != nil
    // 1. lastAdded is not in previous routes. In this case,
    //    [lastAdded uid] != previous_route_id_
    // 2. All new routes are in previous routes and
    //    lastAdded = newRoutes.last.
    // In the first case, we need to announce new route. In the second case,
    // we need to announce if one list is shorter than the other.
    if (lastAdded != nil &&
        ([lastAdded uid] != previous_route_id_ || [newRoutes count] != previous_routes_.size())) {
      previous_route_id_ = [lastAdded uid];
      routeChanged = true;
    }
    previous_routes_.clear();
    for (SemanticsObject* route in newRoutes) {
      previous_routes_.push_back([route uid]);
    }
  } else {
    view_controller_.view.accessibilityElements = nil;
  }

  NSMutableArray<NSNumber*>* doomed_uids = [NSMutableArray arrayWithArray:[objects_.get() allKeys]];
  if (root) {
    std::unordered_map<int32_t, FlutterCustomAccessibilityAction*> initial_overrides;
    MarkAttendanceAndSetCustomActions(root, doomed_uids, initial_overrides);
  }
  [objects_ removeObjectsForKeys:doomed_uids];

  for (NSNumber* uid in doomed_uids) {
    action_overrides_of_semantics_objects_.erase([uid intValue]);
  }

  if (!ios_delegate_->IsFlutterViewControllerPresentingModalViewController(view_controller_)) {
    layoutChanged = layoutChanged || [doomed_uids count] > 0;

    if (routeChanged) {
      NSString* routeName = [lastAdded routeName];
      ios_delegate_->PostAccessibilityNotification(UIAccessibilityScreenChangedNotification,
                                                   routeName);
    }

    if (layoutChanged) {
      ios_delegate_->PostAccessibilityNotification(UIAccessibilityLayoutChangedNotification,
                                                   FindNextFocusableIfNecessary());
    } else if (scrollOccured) {
      // TODO(chunhtai): figure out what string to use for notification. At this
      // point, it is guarantee the previous focused object is still in the tree
      // so that we don't need to worry about focus lost. (e.g. "Screen 0 of 3")
      ios_delegate_->PostAccessibilityNotification(UIAccessibilityPageScrolledNotification,
                                                   FindNextFocusableIfNecessary());
    }
  }
}

void AccessibilityBridge::DispatchSemanticsAction(int32_t uid, flutter::SemanticsAction action) {
  platform_view_->DispatchSemanticsAction(uid, action, {});
}

void AccessibilityBridge::DispatchSemanticsAction(int32_t uid,
                                                  flutter::SemanticsAction action,
                                                  fml::MallocMapping args) {
  platform_view_->DispatchSemanticsAction(uid, action, std::move(args));
}

static void ReplaceSemanticsObject(SemanticsObject* oldObject,
                                   SemanticsObject* newObject,
                                   NSMutableDictionary<NSNumber*, SemanticsObject*>* objects) {
  // `newObject` should represent the same id as `oldObject`.
  assert(oldObject.node.id == newObject.node.id);
  NSNumber* nodeId = @(oldObject.node.id);
  NSUInteger positionInChildlist = [oldObject.parent.children indexOfObject:oldObject];
  [objects removeObjectForKey:nodeId];
  [oldObject.parent replaceChildAtIndex:positionInChildlist withChild:newObject];
  objects[nodeId] = newObject;
}

static SemanticsObject* CreateObject(const flutter::SemanticsNode& node,
                                     fml::WeakPtr<AccessibilityBridge> weak_ptr) {
  if (node.HasFlag(flutter::SemanticsFlags::kIsTextField) &&
      !node.HasFlag(flutter::SemanticsFlags::kIsReadOnly)) {
    // Text fields are backed by objects that implement UITextInput.
    return [[[TextInputSemanticsObject alloc] initWithBridge:weak_ptr uid:node.id] autorelease];
  } else if (node.HasFlag(flutter::SemanticsFlags::kHasToggledState) ||
             node.HasFlag(flutter::SemanticsFlags::kHasCheckedState)) {
    return [[[FlutterSwitchSemanticsObject alloc] initWithBridge:weak_ptr uid:node.id] autorelease];
  } else {
    return [[[FlutterSemanticsObject alloc] initWithBridge:weak_ptr uid:node.id] autorelease];
  }
}

static bool DidFlagChange(const flutter::SemanticsNode& oldNode,
                          const flutter::SemanticsNode& newNode,
                          SemanticsFlags flag) {
  return oldNode.HasFlag(flag) != newNode.HasFlag(flag);
}

SemanticsObject* AccessibilityBridge::GetOrCreateObject(int32_t uid,
                                                        flutter::SemanticsNodeUpdates& updates) {
  SemanticsObject* object = objects_.get()[@(uid)];
  if (!object) {
    object = CreateObject(updates[uid], GetWeakPtr());
    objects_.get()[@(uid)] = object;
  } else {
    // Existing node case
    auto nodeEntry = updates.find(object.node.id);
    if (nodeEntry != updates.end()) {
      // There's an update for this node
      flutter::SemanticsNode node = nodeEntry->second;
      if (DidFlagChange(object.node, node, flutter::SemanticsFlags::kIsTextField) ||
          DidFlagChange(object.node, node, flutter::SemanticsFlags::kIsReadOnly) ||
          DidFlagChange(object.node, node, flutter::SemanticsFlags::kHasCheckedState) ||
          DidFlagChange(object.node, node, flutter::SemanticsFlags::kHasToggledState)) {
        // The node changed its type. In this case, we cannot reuse the existing
        // SemanticsObject implementation. Instead, we replace it with a new
        // instance.
        SemanticsObject* newSemanticsObject = CreateObject(node, GetWeakPtr());
        ReplaceSemanticsObject(object, newSemanticsObject, objects_.get());
        object = newSemanticsObject;
      }
    }
  }
  return object;
}

void AccessibilityBridge::MarkAttendanceAndSetCustomActions(
    SemanticsObject* object,
    NSMutableArray<NSNumber*>* doomed_uids,
    std::unordered_map<int32_t, FlutterCustomAccessibilityAction*> inherited_action_overrides) {
  [doomed_uids removeObject:@(object.uid)];

  if (!inherited_action_overrides.empty()) {
    NSMutableArray<FlutterCustomAccessibilityAction*>* inheritedAccessibilityCustomActions;
    if (object.customSemanticsActions) {
      inheritedAccessibilityCustomActions =
          [[object.customSemanticsActions mutableCopy] autorelease];
    } else {
      inheritedAccessibilityCustomActions = [[[NSMutableArray alloc] init] autorelease];
    }
    for (auto override : inherited_action_overrides) {
      [inheritedAccessibilityCustomActions addObject:override.second];
    }
    object.accessibilityCustomActions = inheritedAccessibilityCustomActions;
  } else {
    object.accessibilityCustomActions = object.customSemanticsActions;
  }
  if (action_overrides_of_semantics_objects_.find(object.uid) !=
      action_overrides_of_semantics_objects_.end()) {
    std::vector<FlutterCustomAccessibilityAction*> overrides =
        action_overrides_of_semantics_objects_[object.uid];
    for (FlutterCustomAccessibilityAction* override : overrides) {
      inherited_action_overrides[override.uid] = override;
    }
  }

  for (SemanticsObject* child in [object children])
    MarkAttendanceAndSetCustomActions(child, doomed_uids, inherited_action_overrides);
}

SemanticsObject* AccessibilityBridge::FindNextFocusableIfNecessary() {
  // This property will be -1 if the focus is outside of the flutter
  // application. In this case, we should not refocus anything.
  if (last_focused_semantics_object_id_ == kSemanticObjectIdInvalid) {
    return nil;
  }

  // Tries to refocus the previous focused semantics object to avoid random jumps.
  return FindFirstFocusable([objects_.get() objectForKey:@(last_focused_semantics_object_id_)]);
}

SemanticsObject* AccessibilityBridge::FindFirstFocusable(SemanticsObject* parent) {
  SemanticsObject* currentObject = parent ?: objects_.get()[@(kRootNodeId)];
  ;
  if (!currentObject)
    return nil;

  if (currentObject.isAccessibilityElement) {
    return currentObject;
  }

  for (SemanticsObject* child in [currentObject children]) {
    SemanticsObject* candidate = FindFirstFocusable(child);
    if (candidate) {
      return candidate;
    }
  }
  return nil;
}

void AccessibilityBridge::HandleEvent(NSDictionary<NSString*, id>* annotatedEvent) {
  NSString* type = annotatedEvent[@"type"];
  if ([type isEqualToString:@"announce"]) {
    NSString* message = annotatedEvent[@"data"][@"message"];
    ios_delegate_->PostAccessibilityNotification(UIAccessibilityAnnouncementNotification, message);
  }
}

fml::WeakPtr<AccessibilityBridge> AccessibilityBridge::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void AccessibilityBridge::clearState() {
  [objects_ removeAllObjects];
  previous_route_id_ = 0;
  previous_routes_.clear();
}

}  // namespace flutter
