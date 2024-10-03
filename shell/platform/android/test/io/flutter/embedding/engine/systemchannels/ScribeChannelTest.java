// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.StandardMethodCodec;
import java.nio.ByteBuffer;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;

@RunWith(AndroidJUnit4.class)
public class ScribeChannelTest {
  private static void sendToBinaryMessageHandler(
      BinaryMessenger.BinaryMessageHandler binaryMessageHandler, String method) {
    MethodCall methodCall = new MethodCall(method, null);
    ByteBuffer encodedMethodCall = StandardMethodCodec.INSTANCE.encodeMethodCall(methodCall);
    binaryMessageHandler.onMessage(
        (ByteBuffer) encodedMethodCall.flip(), mock(BinaryMessenger.BinaryReply.class));
  }

  ScribeChannel.ScribeMethodHandler mockHandler;
  BinaryMessenger.BinaryMessageHandler binaryMessageHandler;

  @SuppressWarnings("deprecation")
  // setMessageHandler is deprecated.
  @Before
  public void setUp() {
    ArgumentCaptor<BinaryMessenger.BinaryMessageHandler> binaryMessageHandlerCaptor =
        ArgumentCaptor.forClass(BinaryMessenger.BinaryMessageHandler.class);
    DartExecutor mockBinaryMessenger = mock(DartExecutor.class);
    mockHandler = mock(ScribeChannel.ScribeMethodHandler.class);
    ScribeChannel scribeChannel = new ScribeChannel(mockBinaryMessenger);

    scribeChannel.setScribeMethodHandler(mockHandler);

    verify(mockBinaryMessenger, times(1))
        .setMessageHandler(any(String.class), binaryMessageHandlerCaptor.capture());

    binaryMessageHandler = binaryMessageHandlerCaptor.getValue();
  }

  @Test
  public void respondsToStartStylusHandwriting() {
    sendToBinaryMessageHandler(binaryMessageHandler, ScribeChannel.METHOD_START_STYLUS_HANDWRITING);

    verify(mockHandler).startStylusHandwriting();
  }

  @Test
  public void respondsToIsStylusHandwritingAvailable() {
    sendToBinaryMessageHandler(
        binaryMessageHandler, ScribeChannel.METHOD_IS_STYLUS_HANDWRITING_AVAILABLE);

    verify(mockHandler).isStylusHandwritingAvailable();
  }
}
