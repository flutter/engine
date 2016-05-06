// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.domokit.editing;

import android.content.ClipboardManager;
import android.content.ClipDescription;
import android.content.Context;
import android.view.inputmethod.InputMethodManager;

import org.chromium.mojo.system.MojoException;
import org.chromium.mojom.editing.ClipData;
import org.chromium.mojom.editing.Clipboard;

/**
 * Android implementation of Clipboard.
 */
public class ClipboardImpl implements Clipboard {
    private Context mContext;

    public ClipboardImpl(Context context) {
        mContext = context;
    }

    @Override
    public void close() {
    }

    @Override
    public void onConnectionError(MojoException e) {}

    @Override
    public void setClipData(ClipData incomingClip) {
        ClipboardManager clipboard =
            (ClipboardManager) mContext.getSystemService(Context.CLIPBOARD_SERVICE);
        android.content.ClipData clip =
            android.content.ClipData.newPlainText("text label?", incomingClip.text);
        clipboard.setPrimaryClip(clip);
    }

    @Override
    public void getClipData(GetClipDataResponse callback) {
        ClipboardManager clipboard =
            (ClipboardManager) mContext.getSystemService(Context.CLIPBOARD_SERVICE);
        android.content.ClipData clip = clipboard.getPrimaryClip();
        if (clip == null ||
            !clip.getDescription().hasMimeType(ClipDescription.MIMETYPE_TEXT_PLAIN)) {
            callback.call(null);
            return;
        }

        android.content.ClipData.Item item = clip.getItemAt(0);
        ClipData clipResult = new ClipData();
        clipResult.text = item.getText().toString();
        callback.call(clipResult);
    }
}
