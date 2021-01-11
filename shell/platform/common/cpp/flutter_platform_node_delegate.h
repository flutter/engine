// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_FLUTTER_PLATFORM_NODE_DELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_FLUTTER_PLATFORM_NODE_DELEGATE_H_

#include "flutter/shell/platform/embedder/embedder.h"

#include "flutter/third_party/accessibility/ax/ax_event_generator.h"
#include "flutter/third_party/accessibility/ax/platform/ax_platform_node_delegate_base.h"

namespace flutter {

class AccessibilityBridge;

//------------------------------------------------------------------------------
/// The accessibility node delegate to be used in accessibility bridge. This
/// class is responsible for providing native accessibility object with
/// appropriate information, such as accessibility label/value/bounds.
///
/// While most methods have default implementations and are ready to be used
/// as-is, the subclasses must override the GetNativeViewAccessible to return
/// native accessibility objects. To do that, subclasses should create and
/// maintain AXPlatformNode[s] which delegate their accessibility attributes to
/// this class.
///
/// For desktop platforms, subclasses also need to override the GetBoundsRect
/// to apply window-to-screen transform.
class FlutterPlatformNodeDelegate : public ui::AXPlatformNodeDelegateBase {
 public:
  FlutterPlatformNodeDelegate();
  ~FlutterPlatformNodeDelegate() override;

  //------------------------------------------------------------------------------
  /// @brief      Gets the accessibility bridge to which this accessibility node
  ///             belongs.
  AccessibilityBridge* GetBridge() const;

  //------------------------------------------------------------------------------
  /// @brief      Gets the underlying ax node for this accessibility node.
  ui::AXNode* GetAXNode() const;

  // ui::AXPlatformNodeDelegateBase override;
  const ui::AXNodeData& GetData() const override;
  bool AccessibilityPerformAction(const ui::AXActionData& data) override;
  gfx::NativeViewAccessible GetParent() override;
  gfx::NativeViewAccessible GetFocus() override;
  int GetChildCount() const override;
  gfx::NativeViewAccessible ChildAtIndex(int index) override;
  gfx::Rect GetBoundsRect(
      const ui::AXCoordinateSystem coordinate_system,
      const ui::AXClippingBehavior clipping_behavior,
      ui::AXOffscreenResult* offscreen_result) const override;
  //------------------------------------------------------------------------------
  /// @brief      Called only once, immediately after construction. The
  ///             constructor doesn't take any arguments because in the Windows
  ///             subclass we use a special function to construct a COM object.
  ///             Subclasses must call super.
  virtual void Init(AccessibilityBridge* bridge, ui::AXNode* node);

 private:
  ui::AXNode* ax_node_;
  AccessibilityBridge* bridge_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_FLUTTER_PLATFORM_NODE_DELEGATE_H_
