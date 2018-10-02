package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import java.util.HashMap;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.StandardMessageCodec;

public class AccessibilityChannel {
  public BasicMessageChannel<Object> channel;
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
          handler.announce((String) data.get("message"));
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
          handler.tooltip((String) data.get("message"));
          break;
        }
      }
    }
  };

  AccessibilityChannel(@NonNull DartExecutor dartExecutor) {
    channel = new BasicMessageChannel<>(dartExecutor, "flutter/accessibility", StandardMessageCodec.INSTANCE);
    channel.setMessageHandler(parsingMessageHandler);
  }

  public void release() {
    handler = null;
  }

  public void setAccessibilityMessageHandler(@Nullable AccessibilityMessageHandler handler) {
    this.handler = handler;
  }

  // TODO(mattcarroll): javadoc
  public void overrideDefaultMethodHandler(@NonNull BasicMessageChannel.MessageHandler<Object> messageHandler) {
    channel.setMessageHandler(messageHandler);
  }

  // TODO(mattcarroll): javadoc
  public void restoreDefaultMethodHandler() {
    channel.setMessageHandler(parsingMessageHandler);
  }

  public interface AccessibilityMessageHandler {
    void announce(@NonNull String message);

    void onTap(int nodeId);

    void onLongPress(int nodeId);

    void tooltip(@NonNull String message);

    void updateLiveRegion(int nodeId);
  }
}
