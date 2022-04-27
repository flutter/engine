package io.flutter.embedding.android;

import static junit.framework.TestCase.assertEquals;
import static junit.framework.TestCase.assertNotNull;
import static junit.framework.TestCase.assertNull;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.annotation.TargetApi;
import android.view.KeyEvent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.JSONMessageCodec;
import io.flutter.util.FakeKeyEvent;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.function.BiConsumer;
import java.util.function.Consumer;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.invocation.InvocationOnMock;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(AndroidJUnit4.class)
@TargetApi(28)
public class KeyboardManagerTest {
  static class CallRecord {
    static enum Type {
      kChannel,
    }

    public CallRecord() {}

    Type type;

    public Consumer<Boolean> reply;
    public ChannelCallData channelData;

    static CallRecord channelCall(
        @NonNull ChannelCallData channelData, @Nullable Consumer<Boolean> reply) {
      final CallRecord record = new CallRecord();
      record.type = Type.kChannel;
      record.channelData = channelData;
      record.reply = reply;
      return record;
    }
  }

  static class ChannelCallData {
    ChannelCallData(@NonNull String channel, @NonNull JSONObject message) {
      this.channel = channel;
      this.message = message;
    }

    public String channel;
    public JSONObject message;
  }

  static ByteBuffer buildResponseMessage(boolean handled) {
    JSONObject body = new JSONObject();
    try {
      body.put("handled", handled);
    } catch (JSONException e) {
      assertNull(e);
    }
    ByteBuffer binaryReply = JSONMessageCodec.INSTANCE.encodeMessage(body);
    binaryReply.rewind();
    return binaryReply;
  }

  @FunctionalInterface
  static interface ChannelCallHandler extends BiConsumer<ChannelCallData, Consumer<Boolean>> {}

  static class KeyboardTester {
    public KeyboardTester() {
      respondToChannelCallsWith(false);
      respondToTextInputWith(false);
      redispatchedCount = 0;

      BinaryMessenger mockMessenger = mock(BinaryMessenger.class);
      doAnswer(invocation -> onChannelMessage(invocation))
          .when(mockMessenger)
          .send(any(String.class), any(ByteBuffer.class));
      doAnswer(invocation -> onChannelMessage(invocation))
          .when(mockMessenger)
          .send(any(String.class), any(ByteBuffer.class), any(BinaryMessenger.BinaryReply.class));

      mockView = mock(KeyboardManager.ViewDelegate.class);
      doAnswer(invocation -> mockMessenger).when(mockView).getBinaryMessenger();
      doAnswer(invocation -> textInputResult)
          .when(mockView)
          .onTextInputKeyEvent(any(KeyEvent.class));
      doAnswer(
              invocation -> {
                boolean handled = keyboardManager.handleEvent((KeyEvent) invocation.getArguments()[1]);
                assertEquals(handled, false);
                redispatchedCount += 1;
                return null;
              })
          .when(mockView)
          .redispatch(any(KeyEvent.class));

      keyboardManager = new KeyboardManager(mockView);
    }

    public @Mock KeyboardManager.ViewDelegate mockView;
    public KeyboardManager keyboardManager;
    public int redispatchedCount;

    public void respondToChannelCallsWith(boolean handled) {
      channelHandler =
          (ChannelCallData data, Consumer<Boolean> reply) -> {
            if (reply != null) {
              reply.accept(handled);
            }
          };
    }

    public void recordChannelCallsTo(@NonNull ArrayList<CallRecord> storage) {
      channelHandler =
          (ChannelCallData data, Consumer<Boolean> reply) -> {
            storage.add(CallRecord.channelCall(data, reply));
          };
    }

    public void respondToTextInputWith(boolean response) {
      textInputResult = response;
    }

    private ChannelCallHandler channelHandler;
    private Boolean textInputResult;

    private Object onChannelMessage(@NonNull InvocationOnMock invocation) {
      final String channel = invocation.getArgument(0);
      final ByteBuffer buffer = invocation.getArgument(1);
      buffer.rewind();
      final JSONObject jsonObject = (JSONObject) JSONMessageCodec.INSTANCE.decodeMessage(buffer);
      final BinaryMessenger.BinaryReply reply = invocation.getArgument(2);
      final Consumer<Boolean> jsonReply =
          reply == null
              ? null
              : handled -> {
                reply.reply(buildResponseMessage(handled));
              };
      channelHandler.accept(new ChannelCallData(channel, jsonObject), jsonReply);
      return null;
    }
  }

  // TODO: Add more test items.
  static void assertChannelEventEquals(
      @NonNull ChannelCallData data, @NonNull String type, @NonNull Integer keyCode) {
    final JSONObject message = data.message;
    assertEquals(data.channel, "flutter/keyevent");
    try {
      assertEquals(type, message.get("type"));
      assertEquals("android", message.get("keymap"));
      assertEquals(keyCode, message.get("keyCode"));
    } catch (JSONException e) {
      assertNull(e);
    }
  }

  @Before
  public void setUp() {
    MockitoAnnotations.openMocks(this);
  }

  // Tests start

  @Test
  public void respondsTrueWhenHandlingNewEvents() {
    final KeyboardTester tester = new KeyboardTester();
    final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    final ArrayList<CallRecord> calls = new ArrayList<CallRecord>();

    tester.recordChannelCallsTo(calls);

    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertChannelEventEquals(calls.get(0).channelData, "keydown", 65);

    // Don't send the key event to the text plugin if the only primary responder
    // hasn't responded.
    verify(tester.mockView, times(0)).onTextInputKeyEvent(any(KeyEvent.class));
    verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));
  }

  @Test
  public void primaryRespondersHaveTheHighestPrecedence() {
    final KeyboardTester tester = new KeyboardTester();
    final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    final ArrayList<CallRecord> calls = new ArrayList<CallRecord>();

    tester.recordChannelCallsTo(calls);

    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertChannelEventEquals(calls.get(0).channelData, "keydown", 65);

    // Don't send the key event to the text plugin if the only primary responder
    // hasn't responded.
    verify(tester.mockView, times(0)).onTextInputKeyEvent(any(KeyEvent.class));
    verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));

    // If a primary responder handles the key event the propagation stops.
    assertNotNull(calls.get(0).reply);
    calls.get(0).reply.accept(true);
    verify(tester.mockView, times(0)).onTextInputKeyEvent(any(KeyEvent.class));
    verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));
  }

  @Test
  public void textInputPluginHasTheSecondHighestPrecedence() {
    final KeyboardTester tester = new KeyboardTester();
    final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    final ArrayList<CallRecord> calls = new ArrayList<CallRecord>();

    tester.recordChannelCallsTo(calls);

    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertChannelEventEquals(calls.get(0).channelData, "keydown", 65);

    // Don't send the key event to the text plugin if the only primary responder
    // hasn't responded.
    verify(tester.mockView, times(0)).onTextInputKeyEvent(any(KeyEvent.class));
    verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));

    // If no primary responder handles the key event the propagates to the text
    // input plugin.
    assertNotNull(calls.get(0).reply);
    // Let text input plugin handle the key event.
    tester.respondToTextInputWith(true);
    calls.get(0).reply.accept(false);

    verify(tester.mockView, times(1)).onTextInputKeyEvent(keyEvent);
    verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));

    // It's not redispatched to the keyboard manager.
    assertEquals(0, tester.redispatchedCount);
  }

  // @Test
  // public void RedispatchKeyEventIfTextInputPluginFailsToHandle() {
  //   final FakeResponder fakeResponder = new FakeResponder();
  //   keyboardManager =
  //       spy(
  //           new KeyboardManager(
  //               mockView, mockTextInputPlugin, new KeyboardManager.Responder[] {fakeResponder}));
  //   final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
  //   final boolean result = keyboardManager.handleEvent(keyEvent);

  //   assertEquals(true, result);
  //   assertEquals(keyEvent, fakeResponder.mLastKeyEvent);

  //   // Don't send the key event to the text plugin if the only primary responder
  //   // hasn't responded.
  //   verify(mockTextInputPlugin, times(0)).handleKeyEvent(any(KeyEvent.class));
  //   verify(mockRootView, times(0)).dispatchKeyEvent(any(KeyEvent.class));

  //   // Neither the primary responders nor text input plugin handles the event.
  //   when(mockTextInputPlugin.handleKeyEvent(any())).thenAnswer(invocation -> false);
  //   fakeResponder.mLastKeyEvent = null;
  //   fakeResponder.eventHandled(false);

  //   verify(mockTextInputPlugin, times(1)).handleKeyEvent(keyEvent);
  //   verify(mockRootView, times(1)).dispatchKeyEvent(keyEvent);
  // }

  // @Test
  // public void respondsFalseWhenHandlingRedispatchedEvents() {
  //   final FakeResponder fakeResponder = new FakeResponder();
  //   keyboardManager =
  //       spy(
  //           new KeyboardManager(
  //               mockView, mockTextInputPlugin, new KeyboardManager.Responder[] {fakeResponder}));
  //   final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
  //   final boolean result = keyboardManager.handleEvent(keyEvent);

  //   assertEquals(true, result);
  //   assertEquals(keyEvent, fakeResponder.mLastKeyEvent);

  //   // Don't send the key event to the text plugin if the only primary responder
  //   // hasn't responded.
  //   verify(mockTextInputPlugin, times(0)).handleKeyEvent(any(KeyEvent.class));
  //   verify(mockRootView, times(0)).dispatchKeyEvent(any(KeyEvent.class));

  //   // Neither the primary responders nor text input plugin handles the event.
  //   when(mockTextInputPlugin.handleKeyEvent(any())).thenAnswer(invocation -> false);
  //   fakeResponder.mLastKeyEvent = null;
  //   fakeResponder.eventHandled(false);

  //   verify(mockTextInputPlugin, times(1)).handleKeyEvent(keyEvent);
  //   verify(mockRootView, times(1)).dispatchKeyEvent(keyEvent);

  //   // It's redispatched to the keyboard manager, but not the primary
  //   // responders.
  //   verify(keyboardManager, times(2)).handleEvent(any(KeyEvent.class));
  //   assertNull(fakeResponder.mLastKeyEvent);
  // }
}
