// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_AX_PLATFORM_AX_PLATFORM_NODE_MAC_H_
#define ACCESSIBILITY_AX_PLATFORM_AX_PLATFORM_NODE_MAC_H_

#import <Cocoa/Cocoa.h>

#include "flutter/fml/macros.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"

#include "ax/ax_export.h"

#include "ax_platform_node_base.h"

@class AXPlatformNodeCocoa;

namespace ax {

class AXPlatformNodeMac : public AXPlatformNodeBase {
 public:
  AXPlatformNodeMac();

  // AXPlatformNode.
  gfx::NativeViewAccessible GetNativeViewAccessible() override;
  void NotifyAccessibilityEvent(ax::Event event_type) override;
  void AnnounceText(const std::u16string& text) override;

  // AXPlatformNodeBase.
  void Destroy() override;
  bool IsPlatformCheckable() const override;

 protected:
  void AddAttributeToList(const char* name,
                          const char* value,
                          PlatformAttributeList* attributes) override;

 private:
  ~AXPlatformNodeMac() override;

  fml::scoped_nsobject<AXPlatformNodeCocoa> native_node_;

  FML_DISALLOW_COPY_AND_ASSIGN(AXPlatformNodeMac);
};

// Convenience function to determine whether an internal object role should
// expose its accessible name in AXValue (as opposed to AXTitle/AXDescription).
AX_EXPORT bool IsNameExposedInAXValueForRole(ax::Role role);

}  // namespace ax

AX_EXPORT
@interface AXPlatformNodeCocoa : NSAccessibilityElement <NSAccessibility>

// Maps AX roles to native roles. Returns NSAccessibilityUnknownRole if not
// found.
+ (NSString*)nativeRoleFromAXRole:(ax::Role)role;

// Maps AX roles to native subroles. Returns nil if not found.
+ (NSString*)nativeSubroleFromAXRole:(ax::Role)role;

// Maps AX events to native notifications. Returns nil if not found.
+ (NSString*)nativeNotificationFromAXEvent:(ax::Event)event;

- (instancetype)initWithNode:(ax::AXPlatformNodeBase*)node;
- (void)detach;

@property(nonatomic, readonly) NSRect boundsInScreen;
@property(nonatomic, readonly) ax::AXPlatformNodeBase* node;

@end

#endif  // ACCESSIBILITY_AX_PLATFORM_AX_PLATFORM_NODE_MAC_H_
