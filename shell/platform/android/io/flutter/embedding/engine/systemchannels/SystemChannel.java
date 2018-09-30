package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;

import java.util.HashMap;
import java.util.Map;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.JSONMessageCodec;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodChannel;

public class SystemChannel {

  public final BasicMessageChannel<Object> channel;

  public SystemChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new BasicMessageChannel<>(dartExecutor, "flutter/system", JSONMessageCodec.INSTANCE);
  }

  public void sendMemoryPressureWarning() {
    Map<String, Object> message = new HashMap<>(1);
    message.put("type", "memoryPressure");
    channel.send(message);
  }

}
