// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

import android.view.inputmethod.InputMethodManager;
import android.view.textservice.SentenceSuggestionsInfo;
import android.view.textservice.SuggestionsInfo;
import android.view.textservice.TextInfo;
import android.view.View;
import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import io.flutter.embedding.engine.systemchannels.ScribeChannel;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.localization.LocalizationPlugin;

/**
 * {@link ScribePlugin} is the implementation of all functionality needed for
 * handwriting stylus text input.
 *
 * <p>The plugin handles requests for scribe sent by the {@link
 * io.flutter.embedding.engine.systemchannels.ScribeChannel}.
 */
public class ScribePlugin
    implements ScribeChannel.ScribeMethodHandler {

  private final ScribeChannel mScribeChannel;
  private final InputMethodManager mImm;
  @NonNull private final View mView;

  public ScribePlugin(
      @NonNull View view,
      @NonNull InputMethodManager imm,
      @NonNull ScribeChannel scribeChannel) {
    view.setAutoHandwritingEnabled(false);

    mView = view;
    mImm = imm;
    mScribeChannel = scribeChannel;

    mScribeChannel.setScribeMethodHandler(this);
  }

  /**
   * Unregisters this {@code ScribePlugin} as the {@code
   * ScribeChannel.ScribeMethodHandler}, for the {@link
   * io.flutter.embedding.engine.systemchannels.ScribeChannel}.
   *
   * <p>Do not invoke any methods on a {@code ScribePlugin} after invoking this method.
   */
  public void destroy() {
    mScribeChannel.setScribeMethodHandler(null);
  }

  /**
   * Starts stylus handwriting input.
   */
  @Override
  public void startStylusHandwriting() {
    mImm.startStylusHandwriting(mView);
  }
}
