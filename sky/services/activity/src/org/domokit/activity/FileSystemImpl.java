// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.domokit.activity;

import android.content.Context;
import org.chromium.mojo.system.MojoException;
import org.chromium.mojom.activity.FileSystem;

/**
 * Android implementation of FileSystem.
 */
public class FileSystemImpl implements FileSystem {
    private static final String TAG = "FileSystemImpl";
    private static android.content.Context sCurrentContext;

    public FileSystemImpl() {
    }

    public static void setCurrentContext(android.content.Context context) {
        sCurrentContext = context;
    }

    @Override
    public void close() {}

    @Override
    public void onConnectionError(MojoException e) {}

    @Override
    public void getFilesDir(GetFilesDirResponse callback) {
        String path = "hi";
        if (sCurrentContext != null)
            path = sCurrentContext.getFilesDir().getPath();
        callback.call(path);
    }

    @Override
    public void getCacheDir(GetCacheDirResponse callback) {
        String path = null;
        if (sCurrentContext != null)
            path = sCurrentContext.getCacheDir().getPath();
        callback.call(path);
    }
}
