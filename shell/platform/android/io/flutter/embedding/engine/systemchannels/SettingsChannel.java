package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.text.format.DateFormat;

import java.util.HashMap;
import java.util.Map;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.JSONMessageCodec;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodChannel;

public class SettingsChannel {

  public final BasicMessageChannel<Object> channel;

  public SettingsChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new BasicMessageChannel<>(dartExecutor, "flutter/settings", JSONMessageCodec.INSTANCE);
  }

  public MessageBuilder startMessage() {
    return new MessageBuilder(channel);
  }

  public static class MessageBuilder {
    private final BasicMessageChannel<Object> channel;
    private Map<String, Object> message = new HashMap<>();

    MessageBuilder(@NonNull BasicMessageChannel<Object> channel) {
      this.channel = channel;
    }

    public MessageBuilder setTextScaleFactor(float textScaleFactor) {
      message.put("textScaleFactor", textScaleFactor);
      return this;
    }

    public MessageBuilder setUse24HourFormat(boolean use24HourFormat) {
      message.put("alwaysUse24HourFormat", use24HourFormat);
      return this;
    }

    public void send() {
      channel.send(message);
    }
  }
}
