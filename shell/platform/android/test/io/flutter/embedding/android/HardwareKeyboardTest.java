package io.flutter.embedding.android;

import static junit.framework.TestCase.assertEquals;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.isNull;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.notNull;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.annotation.TargetApi;
import android.view.KeyEvent;
import android.view.View;
import androidx.annotation.NonNull;
import io.flutter.embedding.android.HardwareKeyboard;
import io.flutter.util.FakeKeyEvent;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
@TargetApi(28)
public class HardwareKeyboardTest {
  @Mock FlutterJNI mockFlutterJni;

  @Before
  public void setUp() {
    MockitoAnnotations.initMocks(this);
    when(mockFlutterJni.isAttached()).thenReturn(true);
  }

  @Test
  public void convertPressHoldReleaseSingleKeyTest() {
    final HardwareKeyboard keyboard = new HardwareKeyboard();

    keyboard.convertEvent(new KeyEvent(
      100,          // downTime
      100,          // eventTime
      ACTION_DOWN,  // action
      100,          // code
      0,            // repeat
      0,            // metaState
      0,            // deviceId
      100,          // scanCode
      flags          // scanCode
    ));
  }
}
