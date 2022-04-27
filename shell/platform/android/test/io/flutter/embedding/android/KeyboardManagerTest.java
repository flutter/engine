package io.flutter.embedding.android;

import static junit.framework.TestCase.assertEquals;
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
      BinaryMessenger mockMessenger = mock(BinaryMessenger.class);
      doAnswer(invocation -> onChannelMessage(invocation))
          .when(mockMessenger)
          .send(any(String.class), any(ByteBuffer.class));
      doAnswer(invocation -> onChannelMessage(invocation))
          .when(mockMessenger)
          .send(any(String.class), any(ByteBuffer.class), any(BinaryMessenger.BinaryReply.class));

      mockView = mock(KeyboardManager.ViewDelegate.class);
      doAnswer(invocation -> mockMessenger).when(mockView).getBinaryMessenger();
      doAnswer(
              invocation -> {
                keyboardManager.handleEvent((KeyEvent) invocation.getArguments()[1]);
                return null;
              })
          .when(mockView)
          .redispatch(any(KeyEvent.class));

      keyboardManager = new KeyboardManager(mockView);
    }

    public @Mock KeyboardManager.ViewDelegate mockView;
    public KeyboardManager keyboardManager;

    private ChannelCallHandler channelHandler;

    public void recordChannelCallsTo(@NonNull ArrayList<CallRecord> storage) {
      channelHandler =
          (ChannelCallData data, Consumer<Boolean> reply) -> {
            storage.add(CallRecord.channelCall(data, reply));
          };
    }

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

  // @Test
  // public void primaryRespondersHaveTheHighestPrecedence() {
  //   final KeyboardTester tester = new KeyboardTester();
  //   final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
  //   final boolean result = keyboardManager.handleEvent(keyEvent);

  //   assertEquals(true, result);
  //   // assertEquals(keyEvent, fakeResponder.mLastKeyEvent);

  //   // Don't send the key event to the text plugin if the only primary responder
  //   // hasn't responded.
  //   verify(tester.mockView, times(0)).onTextInputKeyEvent(any(KeyEvent.class));
  //   verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));

  //   // If a primary responder handles the key event the propagation stops.
  //   assertNotNull(fakeResponder.mLastKeyEventHandledCallback);
  //   fakeResponder.eventHandled(true);
  //   verify(mockTextInputPlugin, times(0)).handleKeyEvent(any(KeyEvent.class));
  //   verify(mockRootView, times(0)).dispatchKeyEvent(any(KeyEvent.class));
  // }

  // @Test
  // public void zeroRespondersTest() {
  //   keyboardManager =
  //       new KeyboardManager(mockView, mockTextInputPlugin, new KeyboardManager.Responder[] {});
  //   final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
  //   final boolean result = keyboardManager.handleEvent(keyEvent);
  //   assertEquals(true, result);

  //   // Send the key event to the text plugin since there's 0 primary responders.
  //   verify(mockTextInputPlugin, times(1)).handleKeyEvent(any(KeyEvent.class));
  // }

  // @Test
  // public void multipleRespondersTest() {
  //   final FakeResponder fakeResponder1 = new FakeResponder();
  //   final FakeResponder fakeResponder2 = new FakeResponder();
  //   keyboardManager =
  //       new KeyboardManager(
  //           mockView,
  //           mockTextInputPlugin,
  //           new KeyboardManager.Responder[] {fakeResponder1, fakeResponder2});
  //   final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
  //   final boolean result = keyboardManager.handleEvent(keyEvent);

  //   assertEquals(true, result);
  //   assertEquals(keyEvent, fakeResponder1.mLastKeyEvent);
  //   assertEquals(keyEvent, fakeResponder2.mLastKeyEvent);

  //   fakeResponder2.eventHandled(false);
  //   // Don't send the key event to the text plugin, since fakeResponder1
  //   // hasn't responded.
  //   verify(mockTextInputPlugin, times(0)).handleKeyEvent(any(KeyEvent.class));

  //   fakeResponder1.eventHandled(false);
  //   verify(mockTextInputPlugin, times(1)).handleKeyEvent(any(KeyEvent.class));
  // }

  // @Test
  // public void multipleRespondersTest2() {
  //   final FakeResponder fakeResponder1 = new FakeResponder();
  //   final FakeResponder fakeResponder2 = new FakeResponder();
  //   keyboardManager =
  //       new KeyboardManager(
  //           mockView,
  //           mockTextInputPlugin,
  //           new KeyboardManager.Responder[] {fakeResponder1, fakeResponder2});
  //   final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
  //   final boolean result = keyboardManager.handleEvent(keyEvent);

  //   fakeResponder2.eventHandled(false);
  //   fakeResponder1.eventHandled(true);

  //   // Handled by primary responders, propagation stops.
  //   verify(mockTextInputPlugin, times(0)).handleKeyEvent(any(KeyEvent.class));
  // }

  // @Test
  // public void multipleRespondersTest3() {
  //   final FakeResponder fakeResponder1 = new FakeResponder();
  //   final FakeResponder fakeResponder2 = new FakeResponder();
  //   keyboardManager =
  //       new KeyboardManager(
  //           mockView,
  //           mockTextInputPlugin,
  //           new KeyboardManager.Responder[] {fakeResponder1, fakeResponder2});
  //   final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
  //   final boolean result = keyboardManager.handleEvent(keyEvent);

  //   fakeResponder2.eventHandled(false);

  //   Exception exception = null;
  //   try {
  //     fakeResponder2.eventHandled(false);
  //   } catch (Exception e) {
  //     exception = e;
  //   }
  //   // Throws since the same handle is called twice.
  //   assertNotNull(exception);
  // }

  // @Test
  // public void textInputPluginHasTheSecondHighestPrecedence() {
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

  //   // If no primary responder handles the key event the propagates to the text
  //   // input plugin.
  //   assertNotNull(fakeResponder.mLastKeyEventHandledCallback);
  //   // Let text input plugin handle the key event.
  //   when(mockTextInputPlugin.handleKeyEvent(any())).thenAnswer(invocation -> true);
  //   fakeResponder.eventHandled(false);

  //   verify(mockTextInputPlugin, times(1)).handleKeyEvent(keyEvent);
  //   verify(mockRootView, times(0)).dispatchKeyEvent(any(KeyEvent.class));

  //   // It's not redispatched to the keyboard manager.
  //   verify(keyboardManager, times(1)).handleEvent(any(KeyEvent.class));
  // }

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
