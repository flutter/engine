package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;

import java.util.HashMap;
import java.util.Map;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.JSONMessageCodec;
import io.flutter.plugin.common.StringCodec;

public class LifecycleChannel {

  public final BasicMessageChannel<String> channel;

  public LifecycleChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new BasicMessageChannel<>(dartExecutor, "flutter/lifecycle", StringCodec.INSTANCE);
  }

  public void appIsInactive() {
    channel.send("AppLifecycleState.inactive");
  }

  public void appIsResumed() {
    channel.send("AppLifecycleState.resumed");
  }

  public void appIsPaused() {
    channel.send("AppLifecycleState.paused");
  }

}
