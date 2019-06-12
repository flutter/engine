// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import java.util.HashMap;
import java.util.Map;

import io.flutter.Log;

class PlatformViewRegistryImpl implements PlatformViewRegistry {

    PlatformViewRegistryImpl() {
        viewFactories = new HashMap<>();
    }

    // Maps a platform view type id to its factory.
    private final Map<String, PlatformViewFactory> viewFactories;

    @Override
    public boolean registerViewFactory(String viewTypeId, PlatformViewFactory factory) {
        if (viewFactories.containsKey(viewTypeId))
            return false;
        Log.d("REPRO", "Registering " + viewTypeId + " -> " + factory + ", this: " + this);
        viewFactories.put(viewTypeId, factory);
        return true;
    }

    PlatformViewFactory getFactory(String viewTypeId) {
        Log.d("REPRO", "Being asked for factory for " + viewTypeId + "... -> " + viewFactories.get(viewTypeId) + ", this: " + this);
        return viewFactories.get(viewTypeId);
    }
}
