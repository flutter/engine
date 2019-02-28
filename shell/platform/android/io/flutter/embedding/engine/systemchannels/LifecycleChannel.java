// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;

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

  /**
   * Cache the current state so that any queries for it may access it.
   */
  private AppLifecycleState state = AppLifecycleState.INACTIVE;

  public LifecycleChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new BasicMessageChannel<>(dartExecutor, "flutter/lifecycle", StringCodec.INSTANCE);
    this.channel.setMessageHandler(new LifecycleQueryMessageHandler(this));
  }

  public AppLifecycleState getCurrentState() {
    return state;
  }

  public void sendCurrentState() {
    channel.send(lifecycleStateToString(state));
  }

  public void appIsInactive() {
    state = AppLifecycleState.INACTIVE;
    sendCurrentState();
  }

  public void appIsResumed() {
    state = AppLifecycleState.RESUMED;
    sendCurrentState();
  }

  public void appIsPaused() {
    state = AppLifecycleState.PAUSED;
    sendCurrentState();
  }

  public static String lifecycleStateToString(AppLifecycleState state) {
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
    }

    /**
     * Replies to all messages on this channel with the current lifecycle state. Also sends the
     * the current lifecycle state through the event channels.
     */
    @Override
    public void onMessage(String message, BasicMessageChannel.Reply<String> reply) {
      channel.sendCurrentState();
      reply.reply(LifecycleChannel.lifecycleStateToString(channel.getCurrentState()));
    }
  }
}
