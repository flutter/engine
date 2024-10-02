// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

import static io.flutter.Build.API_LEVELS;
import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.annotation.TargetApi;
import android.content.Context;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.systemchannels.ScribeChannel;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

@RunWith(AndroidJUnit4.class)
public class ScribePluginTest {
  private final Context ctx = ApplicationProvider.getApplicationContext();

  @Config(sdk = API_LEVELS.API_34)
  @TargetApi(API_LEVELS.API_34)
  @Test
  public void scribePluginIsStylusHandwritingAvailable() {
    ScribeChannel mockScribeChannel = mock(ScribeChannel.class);
    View testView = new View(ctx);
    InputMethodManager mockImm = mock(InputMethodManager.class);
    when(mockImm.isStylusHandwritingAvailable()).thenReturn(true);
    ScribePlugin scribePlugin = new ScribePlugin(testView, mockImm, mockScribeChannel);

    assertEquals(scribePlugin.isStylusHandwritingAvailable(), true);

    verify(mockImm).isStylusHandwritingAvailable();
  }

  @Config(sdk = API_LEVELS.API_34)
  @TargetApi(API_LEVELS.API_34)
  @Test
  public void scribePluginStartStylusHandwriting() {
    ScribeChannel mockScribeChannel = mock(ScribeChannel.class);
    View testView = new View(ctx);
    InputMethodManager mockImm = mock(InputMethodManager.class);
    when(mockImm.isStylusHandwritingAvailable()).thenReturn(true);
    ScribePlugin scribePlugin = new ScribePlugin(testView, mockImm, mockScribeChannel);

    scribePlugin.startStylusHandwriting();

    verify(mockImm).startStylusHandwriting(testView);
  }
}
