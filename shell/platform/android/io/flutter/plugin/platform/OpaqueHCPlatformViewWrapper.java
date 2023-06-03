// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import android.content.Context;
import android.util.AttributeSet;

/**
 * Wraps a platform view for Opaque hybrid composition mode.
 */
class OpaqueHCPlatformViewWrapper extends PlatformViewWrapper {
    public OpaqueHCPlatformViewWrapper(Context context) {
        super(context);
    }
    
    public OpaqueHCPlatformViewWrapper(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    public OpaqueHCPlatformViewWrapper(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }
}
