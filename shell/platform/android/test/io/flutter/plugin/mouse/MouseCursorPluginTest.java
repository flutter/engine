package io.flutter.plugin.mouse;

import android.annotation.TargetApi;
import android.view.PointerIcon;

import org.json.JSONException;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import java.util.HashMap;

import io.flutter.embedding.android.FlutterView;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.systemchannels.MouseCursorChannel;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

//import static org.junit.Assert.assertEquals;
//import static org.junit.Assert.assertTrue;
//import static org.mockito.AdditionalMatchers.aryEq;
//import static org.mockito.AdditionalMatchers.geq;
//import static org.mockito.Matchers.anyInt;
//import static org.mockito.Mockito.eq;
//import static org.mockito.Mockito.isNull;
//import static org.mockito.Mockito.notNull;
//import static org.mockito.Mockito.when;

@Config(manifest = Config.NONE, shadows = {})
@RunWith(RobolectricTestRunner.class)
@TargetApi(24)
public class MouseCursorPluginTest {
  @Test
  public void mouseCursorPlugin_SetsSystemCursorOnRequest() throws JSONException {
    // Initialize a general MouseCursorPlugin.
    FlutterView testView = spy(new FlutterView(RuntimeEnvironment.application));
//    DartExecutor dartExecutor = mock(DartExecutor.class);
    MouseCursorChannel mouseCursorChannel = new MouseCursorChannel(mock(DartExecutor.class));

    MouseCursorPlugin mouseCursorPlugin =
        new MouseCursorPlugin(testView, mouseCursorChannel, testView.getContext());

    mouseCursorChannel.synthesizeMethodCall(
        new MethodCall("flutter/mousecursor", new HashMap<>(){
          {
            put("kind", "text");
          }
        }),
        new MethodChannel.Result() {
          @Override
          public void success(Object result) {
          }

          @Override
          public void error(String errorCode, String errorMessage, Object errorDetails) {
          }

          @Override
          public void notImplemented() {
          }
        });
    verify(testView, times(2))
        .setPointerIcon(any(PointerIcon.class));

//    ArgumentCaptor<String> channelCaptor = ArgumentCaptor.forClass(String.class);
//    ArgumentCaptor<ByteBuffer> bufferCaptor = ArgumentCaptor.forClass(ByteBuffer.class);

//    verify(dartExecutor, times(1))
//        .send(
//            channelCaptor.capture(),
//            bufferCaptor.capture(),
//            any(BinaryMessenger.BinaryReply.class));
//    assertEquals("flutter/textinput", channelCaptor.getValue());
//    verifyMethodCall(bufferCaptor.getValue(), "MouseCursorClient.requestExistingInputState", null);
  }
}
