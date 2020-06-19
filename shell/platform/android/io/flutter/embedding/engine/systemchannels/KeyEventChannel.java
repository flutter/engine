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
     * @param id the event id of the event to be marked as being handled by the framework. Must not
     *     be null.
     */
    public void onKeyEventHandled(long id);

    /**
     * Called whenever the framework responds that a given key event wasn't handled by the
     * framework.
     *
     * @param id the event id of the event to be marked as not being handled by the framework. Must
     *     not be null.
     */
    public void onKeyEventNotHandled(long id);
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
   * Creates a reply handler for this an event with the given eventId.
   *
   * @param eventId the event ID to create a reply for.
   */
  BasicMessageChannel.Reply<Object> createReplyHandler(long eventId) {
    return message -> {
      if (eventResponseHandler == null) {
        return;
      }

      try {
        if (message == null) {
          eventResponseHandler.onKeyEventNotHandled(eventId);
          return;
        }
        final JSONObject annotatedEvent = (JSONObject) message;
        final boolean handled = annotatedEvent.getBoolean("handled");
        if (handled) {
          eventResponseHandler.onKeyEventHandled(eventId);
        } else {
          eventResponseHandler.onKeyEventNotHandled(eventId);
        }
      } catch (JSONException e) {
        Log.e(TAG, "Unable to unpack JSON message: " + e);
        eventResponseHandler.onKeyEventNotHandled(eventId);
      }
    };
  }

  @NonNull public final BasicMessageChannel<Object> channel;

  public void keyUp(@NonNull FlutterKeyEvent keyEvent) {
    Map<String, Object> message = new HashMap<>();
    message.put("type", "keyup");
    message.put("keymap", "android");
    encodeKeyEvent(keyEvent, message);

    channel.send(message, createReplyHandler(keyEvent.eventId));
  }

  public void keyDown(@NonNull FlutterKeyEvent keyEvent) {
    Map<String, Object> message = new HashMap<>();
    message.put("type", "keydown");
    message.put("keymap", "android");
    encodeKeyEvent(keyEvent, message);

    channel.send(message, createReplyHandler(keyEvent.eventId));
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
    message.put("eventId", event.eventId);
  }

  /**
   * A key event as defined by Flutter that includes an id for the specific event to be used when
   * responding to the event.
   */
  public static class FlutterKeyEvent {
    public final int deviceId;
    public final int flags;
    public final int plainCodePoint;
    public final int codePoint;
    public final int keyCode;
    @Nullable public final Character complexCharacter;
    public final int scanCode;
    public final int metaState;
    public final int source;
    public final int vendorId;
    public final int productId;
    public final int repeatCount;
    public final long eventId;

    public FlutterKeyEvent(@NonNull KeyEvent androidKeyEvent, long eventId) {
      this(androidKeyEvent, null, eventId);
    }

    public FlutterKeyEvent(
        @NonNull KeyEvent androidKeyEvent, @Nullable Character complexCharacter, long eventId) {
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
          eventId);
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
        long eventId) {
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
      this.eventId = eventId;
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
