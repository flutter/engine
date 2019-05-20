// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import io.flutter.view.AccessibilityBridge;

/**
 * Facilitates interaction between the accessibility bridge and embedded platform views.
 */
public interface PlatformViewsAccessibilityDelegate extends PlatformViewsResolver {

    /**
     * Attaches an accessibility bridge for this platform views accessibility delegate.
     *
     * Accessibility events originating in platform views belonging to this delegate will be delegated
     * to this accessibility bridge.
     */
    void attachAccessibilityBridge(AccessibilityBridge accessibilityBridge);

    /**
     * Detaches the current accessibility bridge.
     *
     * Any accessibility events sent by platform views belonging to this delegate will be ignored until
     * a new accessibility bridge is attached.
     */
    void detachAccessibiltyBridge();
}
