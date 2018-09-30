package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodChannel;

public class NavigationChannel {

  public final MethodChannel channel;

  public NavigationChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new MethodChannel(dartExecutor, "flutter/navigation", JSONMethodCodec.INSTANCE);
  }

  public void setInitialRoute(String initialRoute) {
    channel.invokeMethod("setInitialRoute", initialRoute);
  }

  public void pushRoute(String route) {
    channel.invokeMethod("pushRoute", route);
  }

  public void popRoute() {
    channel.invokeMethod("popRoute", null);
  }

  public void setMethodCallHandler(MethodChannel.MethodCallHandler handler) {
    channel.setMethodCallHandler(handler);
  }

}
