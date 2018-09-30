package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;

import java.util.Arrays;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodChannel;

public class LocalizationChannel {

  public final MethodChannel channel;

  public LocalizationChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new MethodChannel(dartExecutor, "flutter/localization", JSONMethodCodec.INSTANCE);
  }

  public void setLocale(String language, String country) {
    channel.invokeMethod("setLocale", Arrays.asList(language, country));
  }

  public void setMethodCallHandler(MethodChannel.MethodCallHandler handler) {
    channel.setMethodCallHandler(handler);
  }

}
