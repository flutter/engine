package io.flutter.embedding.engine.systemchannels;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import android.annotation.TargetApi;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(
    manifest = Config.NONE,
    shadows = {})
@RunWith(RobolectricTestRunner.class)
@TargetApi(24)
public class AccessibilityChannelTest {
  @Test
  public void repliesWhenNoAccessibilityHandler() throws JSONException {
    AccessibilityChannel accessibilityChannel = new AccessibilityChannel(mock(DartExecutor.class));
    JSONObject arguments = new JSONObject();
    arguments.put("message", "my message");
    MethodCall call = new MethodCall("announce", arguments);
    MethodChannel.Result result = mock(MethodChannel.Result.class);
    accessibilityChannel.parsingMethodHandler.onMethodCall(call, result);
    verify(result).success(null);
  }
}
