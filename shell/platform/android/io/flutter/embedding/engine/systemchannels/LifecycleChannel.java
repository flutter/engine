// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.util.*;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.StringCodec;

/**
 * TODO(mattcarroll): fill in javadoc for LifecycleChannel.
 */
public class LifecycleChannel {

  public enum AppLifecycleState { INACTIVE, RESUMED, PAUSED }

  @NonNull
  public final BasicMessageChannel<String> channel;

  private final LifecycleQueryMessageHandler messageHandler;

  private AppLifecycleState state = AppLifecycleState.INACTIVE;

  public LifecycleChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new BasicMessageChannel<>(dartExecutor, "flutter/lifecycle", StringCodec.INSTANCE);
    messageHandler = new LifecycleQueryMessageHandler(this);
    // this.channel.setMessageHandler(new LifecycleQueryMessageHandler(this));
    Log.e("flutter", "Created LifecycleQueryMessageHandler");
  }

  public AppLifecycleState getCurrentState() {
    Log.e("flutter", "Got state externally");
    return state;
  }

  public void appIsInactive() {
    Log.e("flutter", "Inactive");
    state = AppLifecycleState.INACTIVE;
    channel.send(LifecycleStateToString(state));
  }

  public void appIsResumed() {
    Log.e("flutter", "Resumed");
    state = AppLifecycleState.RESUMED;
    channel.send(LifecycleStateToString(state));
  }

  public void appIsPaused() {
    Log.e("flutter", "Paused");
    state = AppLifecycleState.PAUSED;
    channel.send(LifecycleStateToString(state));
  }

  public static String LifecycleStateToString(AppLifecycleState state) {
    switch (state) {
      case INACTIVE: return "AppLifecycleState.inactive";
      case RESUMED: return "AppLifecycleState.resumed";
      case PAUSED: return "AppLifecycleState.paused";
    }
    return null;
  }

  public class LifecycleQueryMessageHandler implements BasicMessageChannel.MessageHandler<String> {
      private final LifecycleChannel channel;
                     
      LifecycleQueryMessageHandler (LifecycleChannel channel) {
        this.channel = channel;
        this.channel.channel.setMessageHandler(this);
      }

      /**
       * Replies to all messages on this channel with the current lifecycle state.
       */
      @Override
      public void onMessage(String message, BasicMessageChannel.Reply<String> reply) {
          Log.e("flutter", "Recieved Message... Handling");
          channel.channel.send(LifecycleChannel.LifecycleStateToString(channel.getCurrentState()));
          reply.reply("yoyoyo");
          // reply.reply(LifecycleChannel.LifecycleStateToString(channel.getCurrentState()));
      }
  }
}
