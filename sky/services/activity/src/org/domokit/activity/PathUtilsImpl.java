// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.domokit.activity;

import android.content.Context;
import org.chromium.mojo.system.MojoException;
import org.chromium.mojom.activity.PathUtils;
import android.util.Log;

/**
 * Android implementation of PathUtils.
 */
public class PathUtilsImpl implements PathUtils {
    private static final String TAG = "PathUtilsImpl";
    private static android.content.Context sCurrentContext;

    public PathUtilsImpl() {
        Log.e("MP", "PathUtilsImpl");
    }

    public static void setCurrentContext(android.content.Context context) {
        Log.e("MP", "setCC");
        sCurrentContext = context;
    }

    @Override
    public void close() {
        Log.e("MP", "close");
    }

    @Override
    public void onConnectionError(MojoException e) {
        Log.e("MP", "connectionError");
    }

    @Override
    public void getFilesDir(GetFilesDirResponse callback) {
        Log.e("MP", "getFilesDir");
        String path = "hi";
        if (sCurrentContext != null)
            path = sCurrentContext.getFilesDir().getPath();
        callback.call(path);
    }

    @Override
    public void getCacheDir(GetCacheDirResponse callback) {
        Log.e("MP", "getCacheDir");
        String path = null;
        if (sCurrentContext != null)
            path = sCurrentContext.getCacheDir().getPath();
        callback.call(path);
    }
}
