package io.flutter.embedding.android;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.view.KeyEvent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.JSONMessageCodec;
import io.flutter.util.FakeKeyEvent;
import io.flutter.util.KeyCodes;
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
public class KeyboardManagerTest {
  static public final int SCAN_KEY_A = 0x1e;
  static public final int SCAN_SHIFT_LEFT = 0x2a;


  /**
   * Records a message that {@link KeyboardManager} sends to outside.
   *
   * <p>A call record can originate from many sources, indicated by its {@link type}. Different
   * types will have different fields filled, leaving others empty.
   */
  static class CallRecord {
    static enum Type {
      /**
       * The channel responder sent a message through the key event channel.
       *
       * <p>This call record will have a non-null {@link channelObject}, with an optional {@link
       * reply}.
       */
      kChannel,
      /**
       * The embedder responder sent a message through the key data channel.
       *
       * <p>This call record will have a non-null {@link embedderObject}, with an optional {@link
       * reply}.
       */
      kEmbedder,
    }

    /**
     * Construct an empty call record.
     *
     * <p>Use the static functions to constuct specific types instead.
     */
    private CallRecord() {}

    Type type;

    /**
     * The callback given by the keyboard manager.
     *
     * <p>It might be null, which probably means it is a synthesized event and requires no reply.
     * Otherwise, invoke this callback with whether the event is handled for the keyboard manager to
     * continue processing the key event.
     */
    public Consumer<Boolean> reply;
    /** The data for a call record of type {@link Type.kChannel}. */
    public JSONObject channelObject;
    /** The data for a call record of type {@link Type.kEmbedder}. */
    public KeyData embedderObject;

    /** Construct a call record of type {@link Type.kChannel}. */
    static CallRecord channelCall(
        @NonNull JSONObject channelObject, @Nullable Consumer<Boolean> reply) {
      final CallRecord record = new CallRecord();
      record.type = Type.kChannel;
      record.channelObject = channelObject;
      record.reply = reply;
      return record;
    }

    /** Construct a call record of type {@link Type.kEmbedder}. */
    static CallRecord embedderCall(
        @NonNull KeyData embedderObject, @Nullable Consumer<Boolean> reply) {
      final CallRecord record = new CallRecord();
      record.type = Type.kEmbedder;
      record.embedderObject = embedderObject;
      record.reply = reply;
      return record;
    }
  }

  /**
   * Build a response to a channel message sent by the channel responder.
   *
   * @param handled whether the event is handled.
   */
  static ByteBuffer buildJsonResponse(boolean handled) {
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

  /**
   * Build a response to an embedder message sent by the embedder responder.
   *
   * @param handled whether the event is handled.
   */
  static ByteBuffer buildBinaryResponse(boolean handled) {
    byte[] body = new byte[1];
    body[0] = (byte) (handled ? 1 : 0);
    final ByteBuffer binaryReply = ByteBuffer.wrap(body);
    binaryReply.rewind();
    return binaryReply;
  }

  /**
   * Used to configure how to process a channel message.
   *
   * <p>When the channel responder sends a channel message, this functional interface will be
   * invoked. Its first argument will be the detailed data. The second argument will be a nullable
   * reply callback, which should be called to mock the reply from the framework.
   */
  @FunctionalInterface
  static interface ChannelCallHandler extends BiConsumer<JSONObject, Consumer<Boolean>> {}

  /**
   * Used to configure how to process an embedder message.
   *
   * <p>When the embedder responder sends a key data, this functional interface will be invoked. Its
   * first argument will be the detailed data. The second argument will be a nullable reply
   * callback, which should be called to mock the reply from the framework.
   */
  @FunctionalInterface
  static interface EmbedderCallHandler extends BiConsumer<KeyData, Consumer<Boolean>> {}

  static class KeyboardTester {
    public KeyboardTester() {
      respondToChannelCallsWith(false);
      respondToEmbedderCallsWith(false);
      respondToTextInputWith(false);

      BinaryMessenger mockMessenger = mock(BinaryMessenger.class);
      doAnswer(invocation -> onMessengerMessage(invocation))
          .when(mockMessenger)
          .send(any(String.class), any(ByteBuffer.class));
      doAnswer(invocation -> onMessengerMessage(invocation))
          .when(mockMessenger)
          .send(any(String.class), any(ByteBuffer.class), any(BinaryMessenger.BinaryReply.class));

      mockView = mock(KeyboardManager.ViewDelegate.class);
      doAnswer(invocation -> mockMessenger).when(mockView).getBinaryMessenger();
      doAnswer(invocation -> textInputResult)
          .when(mockView)
          .onTextInputKeyEvent(any(KeyEvent.class));
      doAnswer(
              invocation -> {
                KeyEvent event = invocation.getArgument(0);
                boolean handled = keyboardManager.handleEvent(event);
                assertEquals(handled, false);
                return null;
              })
          .when(mockView)
          .redispatch(any(KeyEvent.class));

      keyboardManager = new KeyboardManager(mockView);
    }

    public @Mock KeyboardManager.ViewDelegate mockView;
    public KeyboardManager keyboardManager;

    /** Set channel calls to respond immediately with the given response. */
    public void respondToChannelCallsWith(boolean handled) {
      channelHandler =
          (JSONObject data, Consumer<Boolean> reply) -> {
            if (reply != null) {
              reply.accept(handled);
            }
          };
    }

    /**
     * Record channel calls to the given storage.
     *
     * <p>They are not responded to until the stored callbacks are manually called.
     */
    public void recordChannelCallsTo(@NonNull ArrayList<CallRecord> storage) {
      channelHandler =
          (JSONObject data, Consumer<Boolean> reply) -> {
            storage.add(CallRecord.channelCall(data, reply));
          };
    }

    /** Set embedder calls to respond immediately with the given response. */
    public void respondToEmbedderCallsWith(boolean handled) {
      embedderHandler =
          (KeyData keyData, Consumer<Boolean> reply) -> {
            if (reply != null) {
              reply.accept(handled);
            }
          };
    }

    /**
     * Record embedder calls to the given storage.
     *
     * <p>They are not responded to until the stored callbacks are manually called.
     */
    public void recordEmbedderCallsTo(@NonNull ArrayList<CallRecord> storage) {
      embedderHandler =
          (KeyData keyData, Consumer<Boolean> reply) -> {
            storage.add(CallRecord.embedderCall(keyData, reply));
          };
    }

    /** Set text calls to respond with the given response. */
    public void respondToTextInputWith(boolean response) {
      textInputResult = response;
    }

    private ChannelCallHandler channelHandler;
    private EmbedderCallHandler embedderHandler;
    private Boolean textInputResult;

    private Object onMessengerMessage(@NonNull InvocationOnMock invocation) {
      final String channel = invocation.getArgument(0);
      final ByteBuffer buffer = invocation.getArgument(1);
      buffer.rewind();

      final BinaryMessenger.BinaryReply reply = invocation.getArgument(2);
      if (channel == "flutter/keyevent") {
        // Parse a channel call.
        final JSONObject jsonObject = (JSONObject) JSONMessageCodec.INSTANCE.decodeMessage(buffer);
        final Consumer<Boolean> jsonReply =
            reply == null ? null : handled -> reply.reply(buildJsonResponse(handled));
        channelHandler.accept(jsonObject, jsonReply);
      } else if (channel == "flutter/keydata") {
        // Parse an embedder call.
        final KeyData keyData = new KeyData(buffer);
        final Consumer<Boolean> booleanReply =
            reply == null ? null : handled -> reply.reply(buildBinaryResponse(handled));
        embedderHandler.accept(keyData, booleanReply);
      } else {
        assertTrue(false);
      }
      return null;
    }
  }

  /**
   * Assert that the channel call is an event that matches the given data.
   *
   * <p>For now this function only validates key code, but not scancode or characters.
   *
   * @param data the target data to be tested.
   * @param type the type of the data, usually "keydown" or "keyup".
   * @param keyCode the key code.
   */
  static void assertChannelEventEquals(
      @NonNull JSONObject message, @NonNull String type, @NonNull Integer keyCode) {
    try {
      assertEquals(type, message.get("type"));
      assertEquals("android", message.get("keymap"));
      assertEquals(keyCode, message.get("keyCode"));
    } catch (JSONException e) {
      assertNull(e);
    }
  }

  /** Assert that the embedder call is an event that matches the given data. */
  static void assertEmbedderEventEquals(
      @NonNull KeyData data,
      KeyData.Type type,
      long physicalKey,
      long logicalKey,
      String character,
      boolean synthesized) {
    assertEquals(type, data.type);
    assertEquals(physicalKey, data.physicalKey);
    assertEquals(logicalKey, data.logicalKey);
    assertEquals(character, data.character);
    assertEquals(synthesized, data.synthesized);
  }

  /**
   * Print each byte of the given buffer as a hex (such as "0a" for 0x0a), and return the
   * concatenated string.
   */
  static String printBufferBytes(@NonNull ByteBuffer buffer) {
    final String[] results = new String[buffer.capacity()];
    for (int byteIdx = 0; byteIdx < buffer.capacity(); byteIdx += 1) {
      results[byteIdx] = String.format("%02x", buffer.get(byteIdx));
    }
    return String.join("", results);
  }

  @Before
  public void setUp() {
    MockitoAnnotations.openMocks(this);
  }

  // Tests start

  @Test
  public void serializeAndDeserializeKeyData() {
    // Test data1: Non-empty character, synthesized.
    final KeyData data1 = new KeyData();
    data1.physicalKey = 0x0a;
    data1.logicalKey = 0x0b;
    data1.timestamp = 0x0c;
    data1.type = KeyData.Type.kRepeat;
    data1.character = "A";
    data1.synthesized = true;

    final ByteBuffer data1Buffer = data1.toBytes();

    assertEquals(
        ""
            + "0100000000000000"
            + "0c00000000000000"
            + "0200000000000000"
            + "0a00000000000000"
            + "0b00000000000000"
            + "0100000000000000"
            + "41",
        printBufferBytes(data1Buffer));
    // `position` is considered as the message size.
    assertEquals(49, data1Buffer.position());

    data1Buffer.rewind();
    final KeyData data1Loaded = new KeyData(data1Buffer);
    assertEquals(data1Loaded.timestamp, data1.timestamp);

    // Test data2: Empty character, not synthesized.
    final KeyData data2 = new KeyData();
    data2.physicalKey = 0xaaaabbbbccccl;
    data2.logicalKey = 0x666677778888l;
    data2.timestamp = 0x333344445555l;
    data2.type = KeyData.Type.kUp;
    data2.character = null;
    data2.synthesized = false;

    final ByteBuffer data2Buffer = data2.toBytes();

    assertEquals(
        ""
            + "0000000000000000"
            + "5555444433330000"
            + "0100000000000000"
            + "ccccbbbbaaaa0000"
            + "8888777766660000"
            + "0000000000000000",
        printBufferBytes(data2Buffer));

    data2Buffer.rewind();
    final KeyData data2Loaded = new KeyData(data2Buffer);
    assertEquals(data2Loaded.timestamp, data2.timestamp);
  }

  @Test
  public void respondsTrueWhenHandlingNewEvents() {
    final KeyboardTester tester = new KeyboardTester();
    final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    final ArrayList<CallRecord> calls = new ArrayList<>();

    tester.recordChannelCallsTo(calls);

    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertChannelEventEquals(calls.get(0).channelObject, "keydown", 65);

    // Don't send the key event to the text plugin if the only primary responder
    // hasn't responded.
    verify(tester.mockView, times(0)).onTextInputKeyEvent(any(KeyEvent.class));
    verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));
  }

  @Test
  public void channelReponderHandlesEvents() {
    final KeyboardTester tester = new KeyboardTester();
    final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    final ArrayList<CallRecord> calls = new ArrayList<>();

    tester.recordChannelCallsTo(calls);

    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertChannelEventEquals(calls.get(0).channelObject, "keydown", 65);

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
  public void embedderReponderHandlesEvents() {
    final KeyboardTester tester = new KeyboardTester();
    final KeyEvent keyEvent =
        new FakeKeyEvent(100, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_A, 0, 0, SCAN_KEY_A, 'a');
    final ArrayList<CallRecord> calls = new ArrayList<>();

    tester.recordEmbedderCallsTo(calls);

    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertEmbedderEventEquals(
        calls.get(0).embedderObject,
        KeyData.Type.kDown,
        KeyCodes.PHYSICAL_KEY_A,
        KeyCodes.LOGICAL_KEY_A,
        "a",
        false);

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
  public void textInputHandlesEventsIfNoRespondersDo() {
    final KeyboardTester tester = new KeyboardTester();
    final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    final ArrayList<CallRecord> calls = new ArrayList<>();

    tester.recordChannelCallsTo(calls);

    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertChannelEventEquals(calls.get(0).channelObject, "keydown", 65);

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
  }

  @Test
  public void redispatchEventsIfTextInputDoesntHandle() {
    final KeyboardTester tester = new KeyboardTester();
    final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    final ArrayList<CallRecord> calls = new ArrayList<>();

    tester.recordChannelCallsTo(calls);

    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertChannelEventEquals(calls.get(0).channelObject, "keydown", 65);

    // Don't send the key event to the text plugin if the only primary responder
    // hasn't responded.
    verify(tester.mockView, times(0)).onTextInputKeyEvent(any(KeyEvent.class));
    verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));

    // Neither the primary responders nor text input plugin handles the event.
    tester.respondToTextInputWith(false);
    calls.get(0).reply.accept(false);

    verify(tester.mockView, times(1)).onTextInputKeyEvent(keyEvent);
    verify(tester.mockView, times(1)).redispatch(keyEvent);
  }

  @Test
  public void redispatchedEventsAreCorrectlySkipped() {
    final KeyboardTester tester = new KeyboardTester();
    final ArrayList<CallRecord> calls = new ArrayList<>();

    tester.recordChannelCallsTo(calls);

    final KeyEvent keyEvent = new FakeKeyEvent(KeyEvent.ACTION_DOWN, 65);
    final boolean result = tester.keyboardManager.handleEvent(keyEvent);

    assertEquals(true, result);
    assertEquals(calls.size(), 1);
    assertChannelEventEquals(calls.get(0).channelObject, "keydown", 65);

    // Don't send the key event to the text plugin if the only primary responder
    // hasn't responded.
    verify(tester.mockView, times(0)).onTextInputKeyEvent(any(KeyEvent.class));
    verify(tester.mockView, times(0)).redispatch(any(KeyEvent.class));

    // Neither the primary responders nor text input plugin handles the event.
    tester.respondToTextInputWith(false);
    calls.get(0).reply.accept(false);

    verify(tester.mockView, times(1)).onTextInputKeyEvent(keyEvent);
    verify(tester.mockView, times(1)).redispatch(keyEvent);

    // It's redispatched to the keyboard manager, but no eventual key calls.
    assertEquals(calls.size(), 1);
  }

  @Test
  public void tapLowerA() {
    final KeyboardTester tester = new KeyboardTester();
    final ArrayList<CallRecord> calls = new ArrayList<>();

    tester.recordEmbedderCallsTo(calls);
    tester.respondToTextInputWith(true); // Suppress redispatching

    assertEquals(
        true,
        tester.keyboardManager.handleEvent(
            new FakeKeyEvent(100, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_A, 0, 0, SCAN_KEY_A, 'a')));
    assertEquals(calls.size(), 1);
    assertEmbedderEventEquals(
        calls.get(0).embedderObject,
        KeyData.Type.kDown,
        KeyCodes.PHYSICAL_KEY_A,
        KeyCodes.LOGICAL_KEY_A,
        "a",
        false);
    calls.clear();

    assertEquals(
        true,
        tester.keyboardManager.handleEvent(
            new FakeKeyEvent(100, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_A, 1, 0, SCAN_KEY_A, 'a')));
    assertEquals(calls.size(), 1);
    assertEmbedderEventEquals(
        calls.get(0).embedderObject,
        KeyData.Type.kRepeat,
        KeyCodes.PHYSICAL_KEY_A,
        KeyCodes.LOGICAL_KEY_A,
        "a",
        false);
    calls.clear();

    assertEquals(
        true,
        tester.keyboardManager.handleEvent(
            new FakeKeyEvent(100, KeyEvent.ACTION_UP, KeyEvent.KEYCODE_A, 0, 0, SCAN_KEY_A, 'a')));
    assertEquals(calls.size(), 1);
    assertEmbedderEventEquals(
        calls.get(0).embedderObject,
        KeyData.Type.kUp,
        KeyCodes.PHYSICAL_KEY_A,
        KeyCodes.LOGICAL_KEY_A,
        null,
        false);
    calls.clear();
  }

  @Test
  public void tapUpperA() {
    final KeyboardTester tester = new KeyboardTester();
    final ArrayList<CallRecord> calls = new ArrayList<>();

    tester.recordEmbedderCallsTo(calls);
    tester.respondToTextInputWith(true); // Suppress redispatching

    // ShiftLeft
    assertEquals(
        true,
        tester.keyboardManager.handleEvent(
            new FakeKeyEvent(100, KeyEvent.ACTION_DOWN, 0x3b, 0, 0x41, SCAN_SHIFT_LEFT, '\0')));
    assertEquals(calls.size(), 1);
    assertEmbedderEventEquals(
        calls.get(0).embedderObject,
        KeyData.Type.kDown,
        KeyCodes.PHYSICAL_SHIFT_LEFT,
        KeyCodes.LOGICAL_SHIFT_LEFT,
        null,
        false);
    calls.clear();

    assertEquals(
        true,
        tester.keyboardManager.handleEvent(
            new FakeKeyEvent(100, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_A, 0, 0x41, SCAN_KEY_A, 'A')));
    assertEquals(calls.size(), 1);
    assertEmbedderEventEquals(
        calls.get(0).embedderObject,
        KeyData.Type.kDown,
        KeyCodes.PHYSICAL_KEY_A,
        KeyCodes.LOGICAL_KEY_A,
        "A",
        false);
    calls.clear();

    assertEquals(
        true,
        tester.keyboardManager.handleEvent(
            new FakeKeyEvent(100, KeyEvent.ACTION_UP, KeyEvent.KEYCODE_A, 0, 0x41, SCAN_KEY_A, 'A')));
    assertEquals(calls.size(), 1);
    assertEmbedderEventEquals(
        calls.get(0).embedderObject,
        KeyData.Type.kUp,
        KeyCodes.PHYSICAL_KEY_A,
        KeyCodes.LOGICAL_KEY_A,
        null,
        false);
    calls.clear();

    // ShiftLeft
    assertEquals(
        true,
        tester.keyboardManager.handleEvent(
            new FakeKeyEvent(100, KeyEvent.ACTION_UP, 0x3b, 0, 0x41, SCAN_SHIFT_LEFT, '\0')));
    assertEquals(calls.size(), 1);
    assertEmbedderEventEquals(
        calls.get(0).embedderObject,
        KeyData.Type.kUp,
        KeyCodes.PHYSICAL_SHIFT_LEFT,
        KeyCodes.LOGICAL_SHIFT_LEFT,
        null,
        false);
    calls.clear();
  }
}
