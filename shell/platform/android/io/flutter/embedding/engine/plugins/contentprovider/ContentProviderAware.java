// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.contentprovider;

import android.support.annotation.NonNull;

/**
 * {@link FlutterPlugin} that wants to know when it is running within a {@link ContentProvider}.
 */
public interface ContentProviderAware {
  /**
   * This {@code ContentProviderAware} {@link FlutterPlugin} is now associated with a
   * {@link ContentProvider}.
   */
  void onAttachedToContentProvider(@NonNull ContentProviderPluginBinding binding);

  /**
   * This plugin has been detached from a {@link ContentProvider}.
   */
  void onDetachedFromContentProvider();
}
