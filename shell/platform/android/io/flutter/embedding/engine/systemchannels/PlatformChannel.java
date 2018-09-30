package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodChannel;

public class PlatformChannel {

  public final MethodChannel channel;

  public PlatformChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new MethodChannel(dartExecutor, "flutter/platform", JSONMethodCodec.INSTANCE);
  }

  public void setMethodCallHandler(MethodChannel.MethodCallHandler handler) {
    channel.setMethodCallHandler(handler);
  }

}
