// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AX_FLUTTER_ACCESSIBILITY_H
#define AX_FLUTTER_ACCESSIBILITY_H

#include "ax/platform/ax_platform_node_delegate_base.h"
#include "ax/ax_event_generator.h"

namespace ax {

class AccessibilityBridge;

class FlutterAccessibility : public AXPlatformNodeDelegateBase {
 public:
  // Creates a platform specific BrowserAccessibility. Ownership passes to the
  // caller.
  static FlutterAccessibility* Create();

  FlutterAccessibility();
  ~FlutterAccessibility() override;

  // Called only once, immediately after construction. The constructor doesn't
  // take any arguments because in the Windows subclass we use a special
  // function to construct a COM object.
  virtual void Init(AccessibilityBridge* bridge, AXNode* node);
  virtual void OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event);
};

} // namespace ax

#endif // AX_FLUTTER_ACCESSIBILITY_H
