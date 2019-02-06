// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.view.KeyEvent;

import java.util.HashMap;
import java.util.Map;

import android.view.KeyCharacterMap;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.JSONMessageCodec;

/**
 * TODO(mattcarroll): fill in javadoc for KeyEventChannel.
 */
public class KeyEventChannel {

  @NonNull
  public final BasicMessageChannel<Object> channel;

  private int mCombiningCharacter;

  public KeyEventChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new BasicMessageChannel<>(dartExecutor, "flutter/keyevent", JSONMessageCodec.INSTANCE);
  }

  public void keyUp(@NonNull KeyEvent keyEvent) {
    Map<String, Object> message = new HashMap<>();
    message.put("type", "keyup");
    message.put("keymap", "android");
    encodeKeyEvent(keyEvent, message);

    channel.send(message);
  }

  public void keyDown(@NonNull KeyEvent keyEvent) {
    Map<String, Object> message = new HashMap<>();
    message.put("type", "keydown");
    message.put("keymap", "android");
    encodeKeyEvent(keyEvent, message);

    channel.send(message);
  }

  private void encodeKeyEvent(@NonNull KeyEvent event, @NonNull Map<String, Object> message) {
    message.put("flags", event.getFlags());
    int codePoint = event.getUnicodeChar();
    message.put("plainCodePoint", event.getUnicodeChar(0x0));
    message.put("codePoint", codePoint);
    message.put("keyCode", event.getKeyCode());
    message.put("scanCode", event.getScanCode());
    message.put("metaState", event.getMetaState());
    if (codePoint == 0) {
      return;
    }
    if ((codePoint & KeyCharacterMap.COMBINING_ACCENT) != 0) {
      // If a combining character was entered before, combine this one with that one.
      int plainCodePoint = codePoint & KeyCharacterMap.COMBINING_ACCENT_MASK;
      if (mCombiningCharacter != 0) {
        mCombiningCharacter = KeyCharacterMap.getDeadChar(mCombiningCharacter, plainCodePoint);
      } else {
        mCombiningCharacter = plainCodePoint;
      }
    } else {
      if (mCombiningCharacter != 0) {
        int combinedChar = KeyCharacterMap.getDeadChar(mCombiningCharacter, codePoint);
        if (combinedChar > 0) {
          codePoint = combinedChar;
        }
        mCombiningCharacter = 0;
      }
      message.put("character", Character.toString((char) codePoint));
    }
  }
}
