// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import static io.flutter.Build.API_LEVELS;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.annotation.TargetApi;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;
import java.nio.ByteBuffer;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.robolectric.annotation.Config;

@RunWith(AndroidJUnit4.class)
public class ScribeChannelTest {
  private static BinaryMessenger.BinaryReply sendToBinaryMessageHandler(
      BinaryMessenger.BinaryMessageHandler binaryMessageHandler, String method) {
    MethodCall methodCall = new MethodCall(method, null);
    ByteBuffer encodedMethodCall = JSONMethodCodec.INSTANCE.encodeMethodCall(methodCall);
    BinaryMessenger.BinaryReply mockReply = mock(BinaryMessenger.BinaryReply.class);
    binaryMessageHandler.onMessage((ByteBuffer) encodedMethodCall.flip(), mockReply);
    return mockReply;
  }

  ScribeChannel.ScribeMethodHandler mockHandler;
  BinaryMessenger.BinaryMessageHandler binaryMessageHandler;

  @Before
  public void setUp() {
    ArgumentCaptor<BinaryMessenger.BinaryMessageHandler> binaryMessageHandlerCaptor =
        ArgumentCaptor.forClass(BinaryMessenger.BinaryMessageHandler.class);
    DartExecutor mockBinaryMessenger = mock(DartExecutor.class);
    mockHandler = mock(ScribeChannel.ScribeMethodHandler.class);
    ScribeChannel scribeChannel = new ScribeChannel(mockBinaryMessenger);

    scribeChannel.setScribeMethodHandler(mockHandler);

    verify((BinaryMessenger) mockBinaryMessenger, times(1))
        .setMessageHandler(any(String.class), binaryMessageHandlerCaptor.capture());

    binaryMessageHandler = binaryMessageHandlerCaptor.getValue();
  }

  @Config(minSdk = API_LEVELS.API_34)
  @TargetApi(API_LEVELS.API_34)
  @Test
  public void respondsToStartStylusHandwriting() {
    BinaryMessenger.BinaryReply mockReply =
        sendToBinaryMessageHandler(
            binaryMessageHandler, ScribeChannel.METHOD_START_STYLUS_HANDWRITING);

    verify(mockReply).reply(any(ByteBuffer.class));
    verify(mockHandler).startStylusHandwriting();
  }

  @Config(minSdk = API_LEVELS.API_34)
  @TargetApi(API_LEVELS.API_34)
  @Test
  public void respondsToIsStylusHandwritingAvailable() {
    BinaryMessenger.BinaryReply mockReply =
        sendToBinaryMessageHandler(
            binaryMessageHandler, ScribeChannel.METHOD_IS_STYLUS_HANDWRITING_AVAILABLE);

    verify(mockReply).reply(any(ByteBuffer.class));
    verify(mockHandler).isStylusHandwritingAvailable();
  }

  @Config(sdk = API_LEVELS.API_32)
  @TargetApi(API_LEVELS.API_32)
  @Test
  public void respondsToStartStylusHandwritingWhenAPILevelUnsupported() {
    BinaryMessenger.BinaryReply mockReply =
        sendToBinaryMessageHandler(
            binaryMessageHandler, ScribeChannel.METHOD_START_STYLUS_HANDWRITING);

    verify(mockReply).reply(any(ByteBuffer.class));
    verify(mockHandler, never()).startStylusHandwriting();
  }

  @Config(sdk = API_LEVELS.API_33)
  @TargetApi(API_LEVELS.API_33)
  @Test
  public void respondsToIsStylusHandwritingAvailableWhenAPILevelUnsupported() {
    BinaryMessenger.BinaryReply mockReply =
        sendToBinaryMessageHandler(
            binaryMessageHandler, ScribeChannel.METHOD_IS_STYLUS_HANDWRITING_AVAILABLE);

    verify(mockReply).reply(any(ByteBuffer.class));
    verify(mockHandler, never()).isStylusHandwritingAvailable();
  }
}
