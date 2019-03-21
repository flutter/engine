// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import io.flutter.view.AccessibilityBridge;

/**
 * A package-private wrapper around the accessibility bridge.
 *
 * This is used to propagate an updatable reference to the accessibility bridge within the package.
 */
class CurrentAccessibilityBridge {
    private AccessibilityBridge accessibilityBridge;

    public AccessibilityBridge getCurrentAccessibilityBridge() {
        return accessibilityBridge;
    }

    public void setCurrentAccessibilityBridge(AccessibilityBridge accessibilityBridge) {
        this.accessibilityBridge = accessibilityBridge;
    }
}
