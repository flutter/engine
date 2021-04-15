package io.flutter.embedding.android;

import static junit.framework.TestCase.assertEquals;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.doAnswer;

import android.annotation.TargetApi;
import android.view.KeyEvent;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel.EventResponseHandler;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel.FlutterKeyEvent;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
@TargetApi(28)
public class KeyChannelResponderTest {
  @Mock KeyEventChannel keyEventChannel;
  KeyChannelResponder channelResponder;

  @Before
  public void setUp() {
    MockitoAnnotations.initMocks(this);
    channelResponder = new KeyChannelResponder(keyEventChannel);
  }

  @Test
  public void primaryResponderTest() {
    final int[] completionCallbackInvocationCounter = {0};

    doAnswer(
            invocation -> {
              invocation.getArgumentAt(2, EventResponseHandler.class).onFrameworkResponse(true);
              return null;
            })
        .when(keyEventChannel)
        .sendFlutterKeyEvent(
            any(FlutterKeyEvent.class), any(boolean.class), any(EventResponseHandler.class));

    final KeyEvent keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, 65);
    channelResponder.handleEvent(
        keyEvent,
        (canHandleEvent) -> {
          completionCallbackInvocationCounter[0]++;
        });
    assertEquals(completionCallbackInvocationCounter[0], 1);
  }
}
