// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AX_FLUTTER_ACCESSIBILITY_H
#define AX_FLUTTER_ACCESSIBILITY_H

#include "flutter/shell/platform/embedder/embedder.h"

#include "ax/platform/ax_platform_node_delegate_base.h"
#include "ax/ax_event_generator.h"

namespace ax {

class AccessibilityBridge;

class FlutterAccessibility : public AXPlatformNodeDelegateBase {
 public:
  // Creates a platform specific FlutterAccessibility. Ownership passes to the
  // caller.
  static FlutterAccessibility* Create();

  FlutterAccessibility();
  ~FlutterAccessibility() override;

  AccessibilityBridge* GetBridge() const;
  AXNode* GetAXNode() const;
  bool AccessibilityPerformAction(const AXActionData& data) override;

  /**
   * Called only once, immediately after construction. The constructor doesn't
   * take any arguments because in the Windows subclass we use a special
   * function to construct a COM object.
   * 
   * Subclass must call super.
   */
  virtual void Init(AccessibilityBridge* bridge, AXNode* node);
  virtual void OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event) = 0;
  virtual void DispatchAccessibilityAction(uint16_t target, FlutterSemanticsAction action, uint8_t* data, size_t data_size) = 0;
 private:
  AXNode* ax_node_;
  AccessibilityBridge* bridge_;
};
} // namespace ax

#endif // AX_FLUTTER_ACCESSIBILITY_H
