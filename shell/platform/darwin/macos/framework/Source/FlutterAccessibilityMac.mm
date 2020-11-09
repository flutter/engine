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



void FlutterAccessibilityMac::OnAccessibilityEvent(AXEventGenerator::TargetedEvent event) {

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
