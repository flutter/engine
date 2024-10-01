// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

import android.view.View;
import android.view.inputmethod.InputMethodManager;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.systemchannels.ScribeChannel;

/**
 * {@link ScribePlugin} is the implementation of all functionality needed for handwriting stylus
 * text input.
 *
 * <p>The plugin handles requests for scribe sent by the {@link
 * io.flutter.embedding.engine.systemchannels.ScribeChannel}.
 */
public class ScribePlugin implements ScribeChannel.ScribeMethodHandler {

  private final ScribeChannel mScribeChannel;
  private final InputMethodManager mImm;
  @NonNull private final View mView;

  public ScribePlugin(
      @NonNull View view, @NonNull InputMethodManager imm, @NonNull ScribeChannel scribeChannel) {
    view.setAutoHandwritingEnabled(false);

    mView = view;
    mImm = imm;
    mScribeChannel = scribeChannel;

    mScribeChannel.setScribeMethodHandler(this);
  }

  /**
   * Unregisters this {@code ScribePlugin} as the {@code ScribeChannel.ScribeMethodHandler}, for the
   * {@link io.flutter.embedding.engine.systemchannels.ScribeChannel}.
   *
   * <p>Do not invoke any methods on a {@code ScribePlugin} after invoking this method.
   */
  public void destroy() {
    mScribeChannel.setScribeMethodHandler(null);
  }

  /**
   * Returns true if the InputMethodManager supports Scribe stylus handwriting input.
   *
   * <p>Call this before calling startStylusHandwriting to make sure it's available.
   */
  @Override
  public boolean isStylusHandwritingAvailable() {
    return mImm.isStylusHandwritingAvailable();
  }

  /**
   * Starts stylus handwriting input.
   *
   * <p>Typically isStylusHandwritingAvailable should be called first to determine whether this is
   * supported by the IME.
   */
  @Override
  public void startStylusHandwriting() {
    if (!mImm.isStylusHandwritingAvailable()) {
      // TODO(justinmc): Maybe I should throw an error here. Or maybe I should
      // expose this method and call it from the framework first.
      return;
    }
    mImm.startStylusHandwriting(mView);
  }
}
