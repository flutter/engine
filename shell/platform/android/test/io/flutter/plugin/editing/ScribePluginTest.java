package io.flutter.plugin.editing;

import static io.flutter.Build.API_LEVELS;
import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.annotation.TargetApi;
import android.content.Context;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.systemchannels.ScribeChannel;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.StandardMethodCodec;
import java.nio.ByteBuffer;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.robolectric.annotation.Config;

@RunWith(AndroidJUnit4.class)
public class ScribePluginTest {
  private final Context ctx = ApplicationProvider.getApplicationContext();

  private static void sendToBinaryMessageHandler(
      BinaryMessenger.BinaryMessageHandler binaryMessageHandler, String method) {
    MethodCall methodCall = new MethodCall(method, null);
    ByteBuffer encodedMethodCall = StandardMethodCodec.INSTANCE.encodeMethodCall(methodCall);
    binaryMessageHandler.onMessage(
        (ByteBuffer) encodedMethodCall.flip(), mock(BinaryMessenger.BinaryReply.class));
  }

  @SuppressWarnings("deprecation")
  // setMessageHandler is deprecated.
  @Test
  public void respondsToStartStylusHandwriting() {
    ArgumentCaptor<BinaryMessenger.BinaryMessageHandler> binaryMessageHandlerCaptor =
        ArgumentCaptor.forClass(BinaryMessenger.BinaryMessageHandler.class);
    DartExecutor mockBinaryMessenger = mock(DartExecutor.class);
    ScribeChannel.ScribeMethodHandler mockHandler = mock(ScribeChannel.ScribeMethodHandler.class);
    ScribeChannel scribeChannel = new ScribeChannel(mockBinaryMessenger);

    scribeChannel.setScribeMethodHandler(mockHandler);

    verify(mockBinaryMessenger, times(1))
        .setMessageHandler(any(String.class), binaryMessageHandlerCaptor.capture());

    BinaryMessenger.BinaryMessageHandler binaryMessageHandler =
        binaryMessageHandlerCaptor.getValue();

    sendToBinaryMessageHandler(binaryMessageHandler, "Scribe.startStylusHandwriting");

    verify(mockHandler).startStylusHandwriting();

    // TODO(justinmc): Ensure mImm.startStylusHandwriting was called.
  }

  @SuppressWarnings("deprecation")
  // setMessageHandler is deprecated.
  @Test
  public void respondsToIsStylusHandwritingAvailable() {
    ArgumentCaptor<BinaryMessenger.BinaryMessageHandler> binaryMessageHandlerCaptor =
        ArgumentCaptor.forClass(BinaryMessenger.BinaryMessageHandler.class);
    DartExecutor mockBinaryMessenger = mock(DartExecutor.class);
    ScribeChannel.ScribeMethodHandler mockHandler = mock(ScribeChannel.ScribeMethodHandler.class);
    ScribeChannel scribeChannel = new ScribeChannel(mockBinaryMessenger);

    scribeChannel.setScribeMethodHandler(mockHandler);

    verify(mockBinaryMessenger, times(1))
        .setMessageHandler(any(String.class), binaryMessageHandlerCaptor.capture());

    BinaryMessenger.BinaryMessageHandler binaryMessageHandler =
        binaryMessageHandlerCaptor.getValue();

    sendToBinaryMessageHandler(binaryMessageHandler, "Scribe.isStylusHandwritingAvailable");

    verify(mockHandler).isStylusHandwritingAvailable();

    // TODO(justinmc): Ensure mImm.isStylusHandwritingAvailable was called.
  }

  // TODO(justinmc): Maybe move the other tests to a ScribeChannel test file?
  // TODO(justinmc): What happens when at another API level?
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
}
