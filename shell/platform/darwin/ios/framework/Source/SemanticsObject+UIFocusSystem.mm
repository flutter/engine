// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "SemanticsObject.h"
#include "flutter/lib/ui/semantics/semantics_node.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"

FLUTTER_ASSERT_ARC

// The SemanticsObject class conforms to UIFocusItem and UIFocusItemContainer
// protocols, so the SemanticsObject tree can also be used to represent
// interactive UI components on screen that can receive UIFocusSystem focus.
//
// Typically, physical key events received by the FlutterViewController is
// first delivered to the framework, but that stopped working for navigation keys
// since iOS 15 when full keyboard access (FKA) is on, because those events are
// consumed by the UIFocusSystem and never dispatched to the UIResponders in the
// application (see
// https://developer.apple.com/documentation/uikit/uikeycommand/3780513-wantspriorityoversystembehavior
// ). FKA relies on the iOS focus engine, to enable FKA on iOS 15+, we use
// SemanticsObject to provide the iOS focus engine with the required hierarchical
// information and geometric context.
//
// The focus engine focus is different from accessibility focus, or even the
// currentFocus of the Flutter FocusManager in the framework. On iOS 15+, FKA
// key events are dispatched to the current iOS focus engine focus (and
// translated to calls such as -[NSObject accessibilityActivate]), while most
// other key events are dispatched to the framework.
@interface SemanticsObject (UIFocusSystem) <UIFocusItem, UIFocusItemContainer>
@end

@implementation SemanticsObject (UIFocusSystem)

#pragma mark - UIFocusEnvironment Conformance

- (void)setNeedsFocusUpdate {
}

- (void)updateFocusIfNeeded {
}

- (BOOL)shouldUpdateFocusInContext:(UIFocusUpdateContext*)context {
  return YES;
}

- (void)didUpdateFocusInContext:(UIFocusUpdateContext*)context
       withAnimationCoordinator:(UIFocusAnimationCoordinator*)coordinator {
}

- (id<UIFocusEnvironment>)parentFocusEnvironment {
  // The root SemanticsObject node's parent is the FlutterView.
  return self.parent ?: self.bridge->view();
}

- (NSArray<id<UIFocusEnvironment>>*)preferredFocusEnvironments {
  return nil;
}

- (id<UIFocusItemContainer>)focusItemContainer {
  return self;
}

#pragma mark - UIFocusItem Conformance

- (BOOL)canBecomeFocused {
  if ((self.node.flags & static_cast<int32_t>(flutter::SemanticsFlags::kIsHidden)) != 0) {
    return NO;
  }
  // Currently only supports SemanticsObjects that handle
  // -[NSObject accessibilityActivate].
  return self.node.HasAction(flutter::SemanticsAction::kTap);
}

- (CGRect)frame {
  return self.accessibilityFrame;
}

#pragma mark - UIFocusItemContainer Conformance

- (NSArray<id<UIFocusItem>>*)focusItemsInRect:(CGRect)rect {
  // It seems the iOS focus system relies heavily on focusItemsInRect
  // (instead of preferredFocusEnvironments) for directional navigation.
  //
  // The order of the items seems to be important, menus and dialogs become
  // unreachable via FKA if the returned children are organized
  // in hit-test order.
  //
  // This method is only supposed to return items within the given
  // rect but returning everything in the subtree seems to work fine.
  NSMutableArray<SemanticsObject*>* reversedItems =
      [[NSMutableArray alloc] initWithCapacity:self.childrenInHitTestOrder.count];
  for (NSUInteger i = 0; i < self.childrenInHitTestOrder.count; ++i) {
    [reversedItems
        addObject:self.childrenInHitTestOrder[self.childrenInHitTestOrder.count - 1 - i]];
  }
  return reversedItems;
}

- (id<UICoordinateSpace>)coordinateSpace {
  return self.bridge->view();
}
@end

@interface FlutterScrollableSemanticsObject ()
@property(nonatomic, readonly) UIScrollView* scrollView;
@end

@interface FlutterScrollableSemanticsObject (UIFocusItemScrollableContainer) <
    UIFocusItemScrollableContainer>
@end

@implementation FlutterScrollableSemanticsObject (UIFocusItemScrollableContainer)
- (CGPoint)contentOffset {
  // After the focus system calls setContentOffset, this value may not
  // immediately reflect the new offset value as the accessibility updates from
  // the framework only comes in once per frame.
  return self.scrollView.contentOffset;
}

- (void)setContentOffset:(CGPoint)contentOffset {
  if (![self isAccessibilityBridgeAlive]) {
    return;
  }

  // The following code assumes the memory layout of CGPoint is exactly the
  // same as an array defined as: `double coords[] = { x, y };`. When converted
  // to dart:typed_data it's a Float64List.
  //
  // The size of a CGFloat is architecture-dependent and it's typically the word
  // size. The last iOS with 32-bit support was iOS 10.
  static_assert(sizeof(CGPoint) == sizeof(CGFloat) * 2);
  static_assert(sizeof(CGFloat) == sizeof(double));
  constexpr size_t bytesPerElement = sizeof(CGFloat);
  CGFloat* data = reinterpret_cast<CGFloat*>(malloc(3 * bytesPerElement));

  static_assert(sizeof(uint64_t) == bytesPerElement);

  // Assumes the host is little endian.
  ((uint64_t*)data)[0] = 11ULL         // 11 means the payload is a Float64List.
                         | 2ULL << 8;  // 2 elements in the Float64List.
  memcpy(data + 1, &contentOffset, sizeof(CGPoint));

  self.bridge->DispatchSemanticsAction(
      self.uid, flutter::SemanticsAction::kScrollToOffset,
      fml::MallocMapping(reinterpret_cast<uint8_t*>(data), 3 * bytesPerElement));
}

- (CGSize)contentSize {
  return self.scrollView.contentSize;
}

- (CGSize)visibleSize {
  return self.scrollView.frame.size;
}

@end
