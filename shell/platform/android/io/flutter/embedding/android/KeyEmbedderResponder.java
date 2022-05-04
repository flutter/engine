// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import androidx.annotation.NonNull;
import java.util.HashMap;
import java.util.function.BiConsumer;
import java.util.function.Consumer;
import io.flutter.plugin.common.BinaryMessenger;

/**
 * A {@link KeyboardManager.Responder} of {@link KeyboardManager} that handles events by sending processed
 * information in {@link KeyData}.
 *
 * <p>This class corresponds to the HardwareKeyboard API in the framework.
 */
public class KeyEmbedderResponder implements KeyboardManager.Responder {
  private static final String TAG = "KeyEmbedderResponder";

  // TODO(dkwingsmt): Doc
  public static final String CHANNEL = "flutter/keydata";

  // TODO(dkwingsmt): put the constants to key map.
  private static final Long kValueMask = 0x000ffffffffl;
  private static final Long kUnicodePlane = 0x00000000000l;
  private static final Long kAndroidPlane = 0x01100000000l;

  private final HashMap<Long, Long> pressingRecords = new HashMap<Long, Long>();
  private BinaryMessenger messenger;

  public KeyEmbedderResponder(BinaryMessenger messenger) {
    this.messenger = messenger;
  }

  private int combiningCharacter;
  // TODO(dkwingsmt): Deduplicate this function from KeyChannelResponder.java.
  /**
   * Applies the given Unicode character in {@code newCharacterCodePoint} to a previously entered
   * Unicode combining character and returns the combination of these characters if a combination
   * exists.
   *
   * <p>This method mutates {@link #combiningCharacter} over time to combine characters.
   *
   * <p>One of the following things happens in this method:
   *
   * <ul>
   *   <li>If no previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint}
   *       is not a combining character, then {@code newCharacterCodePoint} is returned.
   *   <li>If no previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint}
   *       is a combining character, then {@code newCharacterCodePoint} is saved as the {@link
   *       #combiningCharacter} and null is returned.
   *   <li>If a previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint} is
   *       also a combining character, then the {@code newCharacterCodePoint} is combined with the
   *       existing {@link #combiningCharacter} and null is returned.
   *   <li>If a previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint} is
   *       not a combining character, then the {@link #combiningCharacter} is applied to the regular
   *       {@code newCharacterCodePoint} and the resulting complex character is returned. The {@link
   *       #combiningCharacter} is cleared.
   * </ul>
   *
   * <p>The following reference explains the concept of a "combining character":
   * https://en.wikipedia.org/wiki/Combining_character
   */
  Character applyCombiningCharacterToBaseCharacter(int newCharacterCodePoint) {
    char complexCharacter = (char) newCharacterCodePoint;
    boolean isNewCodePointACombiningCharacter =
        (newCharacterCodePoint & KeyCharacterMap.COMBINING_ACCENT) != 0;
    if (isNewCodePointACombiningCharacter) {
      // If a combining character was entered before, combine this one with that one.
      int plainCodePoint = newCharacterCodePoint & KeyCharacterMap.COMBINING_ACCENT_MASK;
      if (combiningCharacter != 0) {
        combiningCharacter = KeyCharacterMap.getDeadChar(combiningCharacter, plainCodePoint);
      } else {
        combiningCharacter = plainCodePoint;
      }
    } else {
      // The new character is a regular character. Apply combiningCharacter to it, if
      // it exists.
      if (combiningCharacter != 0) {
        int combinedChar = KeyCharacterMap.getDeadChar(combiningCharacter, newCharacterCodePoint);
        if (combinedChar > 0) {
          complexCharacter = (char) combinedChar;
        }
        combiningCharacter = 0;
      }
    }

    return complexCharacter;
  }

  private Long getPhysicalKey(@NonNull KeyEvent event) {
    final Long byMapping = KeyboardMap.scanCodeToPhysical.get(event.getScanCode());
    if (byMapping != null) {
      return byMapping;
    }
    // TODO(dkwingsmt): Logic for D-pad
    return kAndroidPlane + event.getScanCode();
  }

  private Long getLogicalKey(@NonNull KeyEvent event) {
    // TODO(dkwingsmt): Better logic.
    final Long byMapping = KeyboardMap.keyCodeToLogical.get(event.getKeyCode());
    if (byMapping != null) {
      return byMapping;
    }
    return kAndroidPlane + event.getKeyCode();
  }

  void updatePressingState(Long physicalKey, Long logicalKey) {
    if (logicalKey != 0) {
      final Long previousValue = pressingRecords.put(physicalKey, logicalKey);
      if (previousValue != null)
        throw new AssertionError("The key was not empty");
    } else {
      final Long previousValue = pressingRecords.remove(physicalKey);
      if (previousValue == null)
        throw new AssertionError("The key was empty");
    }
  }

  // Return: if any events has been sent
  private boolean handleEventImpl(
      @NonNull KeyEvent event, @NonNull OnKeyEventHandledCallback onKeyEventHandledCallback) {
    final Long physicalKey = getPhysicalKey(event);
    final Long logicalKey = getLogicalKey(event);
    final long timestamp = event.getEventTime();
    final Long lastLogicalRecord = pressingRecords.get(physicalKey);
//    final boolean isRepeat = event.getRepeatCount() > 0;

    boolean isDown = false;
    switch (event.getAction()) {
      case KeyEvent.ACTION_DOWN:
        isDown = true;
        break;
      case KeyEvent.ACTION_UP:
        isDown = false;
        break;
      case KeyEvent.ACTION_MULTIPLE:
        // This action type has been deprecated in API level 29, and we don't know how to process it.
        return false;
    }

    Character character = 0;

    // TODO(dkwingsmt): repeat logic
    KeyData.Type type;
    if (isDown) {
      if (lastLogicalRecord == null) {
        type = KeyData.Type.kDown;
      } else {
        // A key has been pressed that has the exact physical key as a currently
        // pressed one. This can happen during repeated events.
        type = KeyData.Type.kRepeat;
      }
      character = applyCombiningCharacterToBaseCharacter(event.getUnicodeChar());
    } else {  // is_down_event false
      if (lastLogicalRecord == null) {
        // The physical key has been released before. It might indicate a missed
        // event due to loss of focus, or multiple keyboards pressed keys with the
        // same physical key. Ignore the up event.
        return false;
      } else {
        type = KeyData.Type.kUp;
      }
    }

    if (type != KeyData.Type.kRepeat) {
      updatePressingState(physicalKey, isDown ? logicalKey : 0);
    }

    final KeyData output = new KeyData();
    output.timestamp = timestamp;
    output.type = type;
    output.logicalKey = logicalKey;
    output.physicalKey = physicalKey;
    output.character = character;
    output.synthesized = false;

    sendKeyEvent(output, onKeyEventHandledCallback);
    return true;
  }

  private void synthesizeEvent(boolean isDown, Long logicalKey, Long physicalKey, long timestamp) {
    final KeyData output = new KeyData();
    output.timestamp = timestamp;
    output.type = isDown ? KeyData.Type.kDown : KeyData.Type.kUp;
    output.logicalKey = logicalKey;
    output.physicalKey = physicalKey;
    output.character = null;
    output.synthesized = true;
    sendKeyEvent(output, null);
  }

  private void sendKeyEvent(KeyData data, OnKeyEventHandledCallback onKeyEventHandledCallback) {
    final BinaryMessenger.BinaryReply handleMessageReply = onKeyEventHandledCallback == null ? null : message -> {
      Boolean handled = false;
      if (message.capacity() == 0) {
        handled = message.get() != 0;
      }
      onKeyEventHandledCallback.onKeyEventHandled(handled);
    };

    messenger.send(CHANNEL, data.toBytes(), handleMessageReply);
  }

  @Override
  public void handleEvent(
      @NonNull KeyEvent event, @NonNull OnKeyEventHandledCallback onKeyEventHandledCallback) {
    final boolean sentAny = handleEventImpl(event, onKeyEventHandledCallback);
    if (!sentAny) {
      onKeyEventHandledCallback.onKeyEventHandled(true);
      synthesizeEvent(true, 0l, 0l, 0l);
    }
  }
}
