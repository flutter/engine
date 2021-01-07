// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_FLUTTER_ACCESSIBILITY_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_FLUTTER_ACCESSIBILITY_H_

#include "flutter/shell/platform/embedder/embedder.h"

#include "flutter/third_party/accessibility/ax/ax_event_generator.h"
#include "flutter/third_party/accessibility/ax/platform/ax_platform_node_delegate_base.h"

namespace ui {

class AccessibilityBridge;

//------------------------------------------------------------------------------
/// The accessibility node to be used in accessibility bridge. This class is
/// responsible for providing native accessibility object with appropriate
/// information, such as accessibility label/value/bounds.
///
/// While most methods have default implementations and are ready to be used
/// as-is, the subclasses must override the GetNativeViewAccessible to return
/// native accessibility objects. To do that, subclasses should create and
/// maintain AXPlatformNode[s] which delegate their accessibility attributes to
/// this class.
///
/// For desktop platforms, subclasses also need to override the GetBoundsRect
/// to apply window-to-screen transform.
///
/// Lastly, each platform needs to implement the FlutterAccessibility::Create
/// static method to inject its sublcass into accessibility bridge.
class FlutterAccessibility : public AXPlatformNodeDelegateBase {
 public:
  //------------------------------------------------------------------------------
  /// @brief      Creates a platform specific FlutterAccessibility. Ownership
  ///             passes to the caller. This method will be called by
  ///             accessibility bridge when it creates accessibility node. Each
  ///             platform needs to implement this method in order to inject its
  ///             subclass into the accessibility bridge.
  static FlutterAccessibility* Create();

  FlutterAccessibility();
  ~FlutterAccessibility() override;

  //------------------------------------------------------------------------------
  /// @brief      Gets the accessibility bridge to which this accessibility node
  ///             belongs.
  AccessibilityBridge* GetBridge() const;

  //------------------------------------------------------------------------------
  /// @brief      Gets the underlying ax node for this accessibility node.
  AXNode* GetAXNode() const;

  // AXPlatformNodeDelegateBase override;
  const AXNodeData& GetData() const override;
  bool AccessibilityPerformAction(const AXActionData& data) override;
  gfx::NativeViewAccessible GetParent() override;
  gfx::NativeViewAccessible GetFocus() override;
  int GetChildCount() const override;
  gfx::NativeViewAccessible ChildAtIndex(int index) override;
  gfx::Rect GetBoundsRect(const AXCoordinateSystem coordinate_system,
                       const AXClippingBehavior clipping_behavior,
                       AXOffscreenResult* offscreen_result) const override;
  //------------------------------------------------------------------------------
  /// @brief      Called only once, immediately after construction. The
  ///             constructor doesn't take any arguments because in the Windows
  ///             subclass we use a special function to construct a COM object.
  ///             Subclasses must call super.
  virtual void Init(AccessibilityBridge* bridge, AXNode* node);

 private:
  AXNode* ax_node_;
  AccessibilityBridge* bridge_;
};

}  // namespace ui

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_FLUTTER_ACCESSIBILITY_H_
