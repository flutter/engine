// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.os.Build;
import android.view.InputDevice;
import android.view.KeyEvent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.JSONMessageCodec;
import java.util.HashMap;
import java.util.Map;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * Event message channel for key events to/from the Flutter framework.
 *
 * <p>Sends key up/down events to the framework, and receives asynchronous messages from the
 * framework about whether or not the key was handled.
 */
public class KeyEventChannel {
  private static final String TAG = "KeyEventChannel";

  /**
   * Sets the event response handler to be used to receive key event response messages from the
   * framework on this channel.
   */
  public void setEventResponseHandler(EventResponseHandler handler) {
    this.eventResponseHandler = handler;
  }

  private EventResponseHandler eventResponseHandler;

  /** A handler of incoming key handling messages. */
  public interface EventResponseHandler {

    /**
     * Called whenever the framework responds that a given key event was handled by the framework.
     *
     * @param event the event to be marked as being handled by the framework. Must not be null.
     */
    public void onKeyEventHandled(KeyEvent event);

    /**
     * Called whenever the framework responds that a given key event wasn't handled by the
     * framework.
     *
     * @param event the event to be marked as not being handled by the framework. Must not be null.
     */
    public void onKeyEventNotHandled(KeyEvent event);
  }

  /**
   * A constructor that creates a KeyEventChannel with the default message handler.
   *
   * @param binaryMessenger the binary messenger used to send messages on this channel.
   */
  public KeyEventChannel(@NonNull BinaryMessenger binaryMessenger) {
    this.channel =
        new BasicMessageChannel<>(binaryMessenger, "flutter/keyevent", JSONMessageCodec.INSTANCE);
  }

  /**
   * Creates a reply handler for the given key event.
   *
   * @param event the Android key event to create a reply for.
   */
  BasicMessageChannel.Reply<Object> createReplyHandler(KeyEvent event) {
    return message -> {
      if (eventResponseHandler == null) {
        return;
      }

      try {
        if (message == null) {
          eventResponseHandler.onKeyEventNotHandled(event);
          return;
        }
        final JSONObject annotatedEvent = (JSONObject) message;
        final boolean handled = annotatedEvent.getBoolean("handled");
        if (handled) {
          eventResponseHandler.onKeyEventHandled(event);
        } else {
          eventResponseHandler.onKeyEventNotHandled(event);
        }
      } catch (JSONException e) {
        Log.e(TAG, "Unable to unpack JSON message: " + e);
        eventResponseHandler.onKeyEventNotHandled(event);
      }
    };
  }

  @NonNull public final BasicMessageChannel<Object> channel;

  public void keyUp(@NonNull FlutterKeyEvent keyEvent) {
    Map<String, Object> message = new HashMap<>();
    message.put("type", "keyup");
    message.put("keymap", "android");
    encodeKeyEvent(keyEvent, message);

    channel.send(message, createReplyHandler(keyEvent.event));
  }

  public void keyDown(@NonNull FlutterKeyEvent keyEvent) {
    Map<String, Object> message = new HashMap<>();
    message.put("type", "keydown");
    message.put("keymap", "android");
    encodeKeyEvent(keyEvent, message);

    channel.send(message, createReplyHandler(keyEvent.event));
  }

  private void encodeKeyEvent(
      @NonNull FlutterKeyEvent event, @NonNull Map<String, Object> message) {
    message.put("flags", event.flags);
    message.put("plainCodePoint", event.plainCodePoint);
    message.put("codePoint", event.codePoint);
    message.put("keyCode", event.keyCode);
    message.put("scanCode", event.scanCode);
    message.put("metaState", event.metaState);
    if (event.complexCharacter != null) {
      message.put("character", event.complexCharacter.toString());
    }
    message.put("source", event.source);
    message.put("vendorId", event.vendorId);
    message.put("productId", event.productId);
    message.put("deviceId", event.deviceId);
    message.put("repeatCount", event.repeatCount);
  }

  /** A key event as defined by Flutter. */
  public static class FlutterKeyEvent {
    /**
     * The id for the device this event came from.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getDeviceId()">KeyEvent.getDeviceId()</a>
     */
    public final int deviceId;
    /**
     * The flags for this key event.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getFlags()">KeyEvent.getFlags()</a>
     */
    public final int flags;
    /**
     * The code point for the Unicode character produced by this event if no meta keys were pressed
     * (by passing 0 to {@code KeyEvent.getUnicodeChar(int)}).
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getUnicodeChar(int)">KeyEvent.getUnicodeChar(int)</a>
     */
    public final int plainCodePoint;
    /**
     * The code point for the Unicode character produced by this event, taking into account the meta
     * keys currently pressed.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getUnicodeChar()">KeyEvent.getUnicodeChar()</a>
     */
    public final int codePoint;
    /**
     * The Android key code for this event.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getKeyCode()">KeyEvent.getKeyCode()</a>
     */
    public final int keyCode;
    /**
     * The character produced by this event, including any combining characters pressed before it.
     */
    @Nullable public final Character complexCharacter;
    /**
     * The Android scan code for the key pressed.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getScanCode()">KeyEvent.getScanCode()</a>
     */
    public final int scanCode;
    /**
     * The meta key state for the Android key event.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getMetaState()">KeyEvent.getMetaState()</a>
     */
    public final int metaState;
    /**
     * The source of the key event.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getSource()">KeyEvent.getSource()</a>
     */
    public final int source;
    /**
     * The vendorId of the device that produced this key event.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/InputDevice?hl=en#getVendorId()">InputDevice.getVendorId()</a>
     */
    public final int vendorId;
    /**
     * The productId of the device that produced this key event.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/InputDevice?hl=en#getProductId()">InputDevice.getProductId()</a>
     */
    public final int productId;
    /**
     * The repeat count for this event.
     *
     * @see <a
     *     href="https://developer.android.com/reference/android/view/KeyEvent?hl=en#getRepeatCount()">KeyEvent.getRepeatCount()</a>
     */
    public final int repeatCount;
    /**
     * The Android key event that this Flutter key event was created from.
     *
     * <p>This event is used to identify pending events when results are received from the
     * framework.
     */
    public final KeyEvent event;

    public FlutterKeyEvent(@NonNull KeyEvent androidKeyEvent) {
      this(androidKeyEvent, null);
    }

    public FlutterKeyEvent(
        @NonNull KeyEvent androidKeyEvent, @Nullable Character complexCharacter) {
      this(
          androidKeyEvent.getDeviceId(),
          androidKeyEvent.getFlags(),
          androidKeyEvent.getUnicodeChar(0x0),
          androidKeyEvent.getUnicodeChar(),
          androidKeyEvent.getKeyCode(),
          complexCharacter,
          androidKeyEvent.getScanCode(),
          androidKeyEvent.getMetaState(),
          androidKeyEvent.getSource(),
          androidKeyEvent.getRepeatCount(),
          androidKeyEvent);
    }

    public FlutterKeyEvent(
        int deviceId,
        int flags,
        int plainCodePoint,
        int codePoint,
        int keyCode,
        @Nullable Character complexCharacter,
        int scanCode,
        int metaState,
        int source,
        int repeatCount,
        KeyEvent event) {
      this.deviceId = deviceId;
      this.flags = flags;
      this.plainCodePoint = plainCodePoint;
      this.codePoint = codePoint;
      this.keyCode = keyCode;
      this.complexCharacter = complexCharacter;
      this.scanCode = scanCode;
      this.metaState = metaState;
      this.source = source;
      this.repeatCount = repeatCount;
      this.event = event;
      InputDevice device = InputDevice.getDevice(deviceId);
      if (device != null) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
          this.vendorId = device.getVendorId();
          this.productId = device.getProductId();
        } else {
          this.vendorId = 0;
          this.productId = 0;
        }
      } else {
        this.vendorId = 0;
        this.productId = 0;
      }
    }
  }
}
