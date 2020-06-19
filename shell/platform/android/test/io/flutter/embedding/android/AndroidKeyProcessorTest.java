package io.flutter.embedding.android;

import static junit.framework.Assert.assertFalse;
import static junit.framework.Assert.assertNotNull;
import static junit.framework.TestCase.assertEquals;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.annotation.TargetApi;
import android.app.Application;
import android.content.Context;
import android.view.KeyEvent;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel;
import io.flutter.embedding.engine.systemchannels.TextInputChannel;
import io.flutter.plugin.editing.TextInputPlugin;
import io.flutter.util.FakeKeyEvent;
import java.util.Map;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
@TargetApi(28)
public class AndroidKeyProcessorTest {
  @Mock FlutterJNI mockFlutterJni;

  @Before
  public void setUp() {
    MockitoAnnotations.initMocks(this);
    when(mockFlutterJni.isAttached()).thenReturn(true);
  }

  @Test
  public void sendsKeyDownEventsToEventResponder() {
    FlutterEngine flutterEngine = mockFlutterEngine();
    KeyEventChannel fakeKeyEventChannel = flutterEngine.getKeyEventChannel();
    TextInputPlugin fakeTextInputPlugin = mock(TextInputPlugin.class);

    AndroidKeyProcessor processor =
        new AndroidKeyProcessor(
            RuntimeEnvironment.application, fakeKeyEventChannel, fakeTextInputPlugin);

    processor.onKeyDown(new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65));
    assertFalse(processor.eventResponder.dispatchingKeyEvent);
    verify(fakeKeyEventChannel, times(1)).keyDown(any(KeyEventChannel.FlutterKeyEvent.class));
    verify(fakeKeyEventChannel, times(0)).keyUp(any(KeyEventChannel.FlutterKeyEvent.class));
    assertEquals(1, processor.eventResponder.pendingEvents.size());
    Map.Entry<Long, KeyEvent> firstPendingEvent =
        processor.eventResponder.pendingEvents.peekFirst();
    assertNotNull(firstPendingEvent);
    processor.eventResponder.onKeyEventHandled(firstPendingEvent.getKey());
    assertEquals(0, processor.eventResponder.pendingEvents.size());
  }

  @Test
  public void unhandledKeyEventsAreSynthesized() {
    FlutterEngine flutterEngine = mockFlutterEngine();
    KeyEventChannel fakeKeyEventChannel = flutterEngine.getKeyEventChannel();
    TextInputPlugin fakeTextInputPlugin = mock(TextInputPlugin.class);
    Application spiedApplication = spy(RuntimeEnvironment.application);
    Context spiedContext = spy(spiedApplication.getBaseContext());
    when(spiedApplication.getBaseContext()).thenReturn(spiedContext);

    AndroidKeyProcessor processor =
        new AndroidKeyProcessor(spiedApplication, fakeKeyEventChannel, fakeTextInputPlugin);
    AndroidKeyProcessor.EventResponder eventResponder =
        spy(new AndroidKeyProcessor.EventResponder(spiedContext));
    processor.setEventResponder(eventResponder);

    KeyEvent event = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    processor.onKeyDown(event);
    assertFalse(processor.eventResponder.dispatchingKeyEvent);
    assertEquals(1, processor.eventResponder.pendingEvents.size());
    Map.Entry<Long, KeyEvent> firstPendingEvent =
        processor.eventResponder.pendingEvents.peekFirst();
    assertNotNull(firstPendingEvent);
    processor.eventResponder.onKeyEventNotHandled(firstPendingEvent.getKey());
    assertEquals(0, processor.eventResponder.pendingEvents.size());
    verify(eventResponder).dispatchKeyEvent(event);
  }

  @Test
  public void sendsKeyUpEventsToEventResponder() {
    FlutterEngine flutterEngine = mockFlutterEngine();
    KeyEventChannel fakeKeyEventChannel = flutterEngine.getKeyEventChannel();
    TextInputPlugin fakeTextInputPlugin = mock(TextInputPlugin.class);

    AndroidKeyProcessor processor =
        new AndroidKeyProcessor(
            RuntimeEnvironment.application, fakeKeyEventChannel, fakeTextInputPlugin);

    processor.onKeyUp(new FakeKeyEvent(KeyEvent.ACTION_UP, 65));
    assertFalse(processor.eventResponder.dispatchingKeyEvent);
    verify(fakeKeyEventChannel, times(0)).keyDown(any(KeyEventChannel.FlutterKeyEvent.class));
    verify(fakeKeyEventChannel, times(1)).keyUp(any(KeyEventChannel.FlutterKeyEvent.class));
    Map.Entry<Long, KeyEvent> firstPendingEvent =
        processor.eventResponder.pendingEvents.peekFirst();
    assertNotNull(firstPendingEvent);
    processor.eventResponder.onKeyEventHandled(firstPendingEvent.getKey());
    assertEquals(0, processor.eventResponder.pendingEvents.size());
  }

  @NonNull
  private FlutterEngine mockFlutterEngine() {
    // Mock FlutterEngine and all of its required direct calls.
    FlutterEngine engine = mock(FlutterEngine.class);
    when(engine.getKeyEventChannel()).thenReturn(mock(KeyEventChannel.class));
    when(engine.getTextInputChannel()).thenReturn(mock(TextInputChannel.class));

    return engine;
  }
}
