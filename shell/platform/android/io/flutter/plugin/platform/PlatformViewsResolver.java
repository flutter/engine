// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import android.view.View;

public interface PlatformViewsResolver {
    /**
     * Returns the root of the view hierarchy for the platform view with the requested id, or null if there is no
     * corresponding view.
     */
    View getPlatformViewById(Integer id);
}
