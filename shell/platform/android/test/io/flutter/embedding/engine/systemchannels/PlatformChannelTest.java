package io.flutter.embedding.engine.systemchannels;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.refEq;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.res.AssetManager;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.dart.PlatformMessageHandler;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicReference;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(AndroidJUnit4.class)
public class PlatformChannelTest {
  @Test
  public void platformChannel_hasStringsMessage() {
    MethodChannel rawChannel = mock(MethodChannel.class);
    FlutterJNI mockFlutterJNI = mock(FlutterJNI.class);
    DartExecutor dartExecutor = new DartExecutor(mockFlutterJNI, mock(AssetManager.class));
    PlatformChannel fakePlatformChannel = new PlatformChannel(dartExecutor);
    PlatformChannel.PlatformMessageHandler mockMessageHandler =
        mock(PlatformChannel.PlatformMessageHandler.class);
    fakePlatformChannel.setPlatformMessageHandler(mockMessageHandler);
    Boolean returnValue = true;
    when(mockMessageHandler.clipboardHasStrings()).thenReturn(returnValue);
    MethodCall methodCall = new MethodCall("Clipboard.hasStrings", null);
    MethodChannel.Result mockResult = mock(MethodChannel.Result.class);
    fakePlatformChannel.parsingMethodCallHandler.onMethodCall(methodCall, mockResult);

    JSONObject expected = new JSONObject();
    try {
      expected.put("value", returnValue);
    } catch (JSONException e) {
    }
    verify(mockResult).success(refEq(expected));
  }

  @Test
  public void synchronousChannelHandlerIsCalledSynchronously() {
    final FlutterJNI flutterJNI = mock(FlutterJNI.class);
    final DartExecutor dartExecutor = new DartExecutor(flutterJNI, mock(AssetManager.class));
    final PlatformChannel platformChannel = new PlatformChannel(dartExecutor);
    final ArgumentCaptor<PlatformMessageHandler> handlerCaptor =
        ArgumentCaptor.forClass(PlatformMessageHandler.class);
    // This call sets flutterJNI's method handler to dartExecutor.
    dartExecutor.onAttachedToJNI();
    verify(flutterJNI).setPlatformMessageHandler(handlerCaptor.capture());

    // This is a DartMessenger instance.
    final PlatformMessageHandler handler = handlerCaptor.getValue();

    final MethodCall methodCall = new MethodCall("TextScale.apply", 42.0);
    final ByteBuffer buffer = JSONMethodCodec.INSTANCE.encodeMethodCall(methodCall);
    buffer.rewind();
    final ArgumentCaptor<ByteBuffer> replyBufferCaptor = ArgumentCaptor.forClass(ByteBuffer.class);

    final AtomicReference<Float> scaledFontSize = new AtomicReference<>(-1.0f);
    final Thread thread =
        new Thread(
            () -> {
              handler.handleMessageFromDart(
                  "flutter/platformSynchronous", buffer, /*replyId=*/ 1234, /*messageData=*/ 0);
              verify(flutterJNI, times(1))
                  .invokePlatformMessageResponseCallback(
                      eq(1234), replyBufferCaptor.capture(), anyInt());
              final ByteBuffer replyBuffer = replyBufferCaptor.getValue();
              replyBuffer.rewind();
              final Object reply = JSONMethodCodec.INSTANCE.decodeEnvelope(replyBuffer);
              scaledFontSize.set(((Number) reply).floatValue());
            });

    platformChannel.setSynchronousPlatformMessageHandler(
        new PlatformChannel.SynchronousPlatformMessageHandler() {
          @Override
          public float applyTextScale(float fontSize) {
            assertEquals(Thread.currentThread(), thread);
            return fontSize * 2;
          }
        });

    thread.start();
    assertEquals(-1.0f, scaledFontSize.get(), 0.0f);
    try {
      thread.join();
    } catch (InterruptedException e) {
      throw new RuntimeException(e);
    }
    assertEquals(42.0f * 2, scaledFontSize.get(), 0.0f);
  }
}
