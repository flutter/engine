package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import java.util.HashMap;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.StandardMessageCodec;

/**
 * System channel that sends accessibility requests and events from Flutter to Android.
 * <p>
 * See {@link AccessibilityMessageHandler}, which lists all accessibility requests and
 * events that might be sent from Flutter to the Android platform.
 */
public class AccessibilityChannel {
  @NonNull
  public BasicMessageChannel<Object> channel;
  @Nullable
  private AccessibilityMessageHandler handler;

  private final BasicMessageChannel.MessageHandler<Object> parsingMessageHandler = new BasicMessageChannel.MessageHandler<Object>() {
    @Override
    public void onMessage(Object message, BasicMessageChannel.Reply<Object> reply) {
      // If there is no handler to respond to this message then we don't need to
      // parse it. Return.
      if (handler == null) {
        return;
      }

      @SuppressWarnings("unchecked")
      final HashMap<String, Object> annotatedEvent = (HashMap<String, Object>) message;
      final String type = (String) annotatedEvent.get("type");
      @SuppressWarnings("unchecked")
      final HashMap<String, Object> data = (HashMap<String, Object>) annotatedEvent.get("data");

      switch (type) {
        case "announce":
          String announceMessage = (String) data.get("message");
          handler.announce(announceMessage == null ? "" : announceMessage);
          break;
        case "tap": {
          Integer nodeId = (Integer) annotatedEvent.get("nodeId");
          if (nodeId != null) {
            handler.onTap(nodeId);
          }
          break;
        }
        case "longPress": {
          Integer nodeId = (Integer) annotatedEvent.get("nodeId");
          if (nodeId != null) {
            handler.onLongPress(nodeId);
          }
          break;
        }
        case "tooltip": {
          String tooltipMessage = (String) data.get("message");
          handler.onTooltip(tooltipMessage == null ? "" : tooltipMessage);
          break;
        }
      }
    }
  };

  public AccessibilityChannel(@NonNull DartExecutor dartExecutor) {
    channel = new BasicMessageChannel<>(dartExecutor, "flutter/accessibility", StandardMessageCodec.INSTANCE);
    channel.setMessageHandler(parsingMessageHandler);
  }

  /**
   * Sets the {@link AccessibilityMessageHandler} which receives all events and requests
   * that are parsed from the underlying accessibility channel.
   */
  public void setAccessibilityMessageHandler(@Nullable AccessibilityMessageHandler handler) {
    this.handler = handler;
  }

  /**
   * Overrides the standard parsing logic for the accessibility channel with the given
   * {@code messageHandler}.
   *
   * Calling this method disconnects the standard channel message handler and as a result
   * no methods will be invoked on a given {@link AccessibilityMessageHandler} until
   * {@link #restoreDefaultMethodHandler()} is invoked.
   */
  public void overrideDefaultMethodHandler(@NonNull BasicMessageChannel.MessageHandler<Object> messageHandler) {
    channel.setMessageHandler(messageHandler);
  }

  /**
   * Replaces an overriding {@link io.flutter.plugin.common.BasicMessageChannel.MessageHandler}
   * with the standard handler that forwards calls to {@link AccessibilityMessageHandler}.
   *
   * This method is the inverse of {@link #overrideDefaultMethodHandler(BasicMessageChannel.MessageHandler)}.
   */
  public void restoreDefaultMethodHandler() {
    channel.setMessageHandler(parsingMessageHandler);
  }

  public interface AccessibilityMessageHandler {
    /**
     * The Dart application would like the given {@code message} to be announced.
     */
    void announce(@NonNull String message);

    /**
     * The user has tapped on the artifact with the given {@code nodeId}.
     */
    void onTap(int nodeId);

    /**
     * The user has long pressed on the artifact with the given {@code nodeId}.
     */
    void onLongPress(int nodeId);

    /**
     * The user has opened a popup window, menu, dialog, etc.
     */
    void onTooltip(@NonNull String message);
  }
}