// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_accessibility.h"



namespace ax {

FlutterAccessibility::FlutterAccessibility() = default;

FlutterAccessibility::~FlutterAccessibility() = default;

void FlutterAccessibility::Init(AccessibilityBridge* bridge, AXNode* node) {}
void FlutterAccessibility::OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event) {}
}  // namespace ax
