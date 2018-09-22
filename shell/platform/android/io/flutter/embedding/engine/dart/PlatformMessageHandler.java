package io.flutter.embedding.engine.dart;

public interface PlatformMessageHandler {
  void handlePlatformMessage(final String channel, byte[] message, final int replyId);

  void handlePlatformMessageResponse(int replyId, byte[] reply);
}
