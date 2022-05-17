// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import androidx.annotation.NonNull;
import io.flutter.plugin.common.BinaryMessenger;
import java.util.HashMap;

/**
 * A {@link KeyboardManager.Responder} of {@link KeyboardManager} that handles events by sending
 * processed information in {@link KeyData}.
 *
 * <p>This class corresponds to the HardwareKeyboard API in the framework.
 */
public class KeyEmbedderResponder implements KeyboardManager.Responder {
  private static final String TAG = "KeyEmbedderResponder";

  private static KeyData.Type getEventType(KeyEvent event) {
    final boolean isRepeatEvent = event.getRepeatCount() > 0;
    switch (event.getAction()) {
      case KeyEvent.ACTION_DOWN:
        return isRepeatEvent ? KeyData.Type.kRepeat : KeyData.Type.kDown;
      case KeyEvent.ACTION_UP:
        return KeyData.Type.kUp;
      default:
        throw new AssertionError("Unexpected event type");
    }
  }

  private final HashMap<Long, Long> pressingRecords = new HashMap<>();
  private BinaryMessenger messenger;

  public KeyEmbedderResponder(BinaryMessenger messenger) {
    this.messenger = messenger;
    // CapsLock
    //    this.pressingGoals.add(new PressingGoal(true, 0x0000070039L, 0x0100000104L,
    // KeyEvent.META_CAPS_LOCK_ON));
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
    if (newCharacterCodePoint == 0) {
      combiningCharacter = 0;
      return 0;
    }
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
    final Long byMapping = KeyboardMap.scanCodeToPhysical.get((long) event.getScanCode());
    if (byMapping != null) {
      return byMapping;
    }
    return KeyboardMap.kAndroidPlane + event.getScanCode();
  }

  private Long getLogicalKey(@NonNull KeyEvent event) {
    final Long byMapping = KeyboardMap.keyCodeToLogical.get((long) event.getKeyCode());
    if (byMapping != null) {
      return byMapping;
    }
    return KeyboardMap.kAndroidPlane + event.getKeyCode();
  }

  void updatePressingState(Long physicalKey, Long logicalKey) {
    if (logicalKey != null) {
      final Long previousValue = pressingRecords.put(physicalKey, logicalKey);
      if (previousValue != null) throw new AssertionError("The key was not empty");
    } else {
      final Long previousValue = pressingRecords.remove(physicalKey);
      if (previousValue == null) throw new AssertionError("The key was empty");
    }
  }

  void synchronizePressingKey(
      KeyboardMap.PressingGoal goal, boolean truePressed, long eventPhysicalKey, KeyEvent event) {
    // During an incoming event, there might be a synthesized Flutter event for each key of each
    // pressing goal, followed by an eventual main Flutter event.
    //
    //    NowState ---------------->  PreEventState --------------> TrueState
    //              Synchronization                      Event
    //
    // The goal of the synchronization algorithm is to derive a pre-event state that can satisfy the
    // true state (`truePressed`) after the event, and that requires as few synthesized events based
    // on the current state (`nowStates`) as possible.
    final boolean[] nowStates = new boolean[goal.keys.length];
    final Boolean[] preEventStates = new Boolean[goal.keys.length];
    boolean postEventAnyPressed = false;
    // 1. Find the current states of all keys.
    // 2. Derive the pre-event state of the event key (if applicable.)
    for (int keyIdx = 0; keyIdx < goal.keys.length; keyIdx += 1) {
      nowStates[keyIdx] = pressingRecords.containsKey(goal.keys[keyIdx].physicalKey);
      if (goal.keys[keyIdx].physicalKey == eventPhysicalKey) {
        switch (getEventType(event)) {
          case kDown:
            preEventStates[keyIdx] = false;
            postEventAnyPressed = true;
            if (!truePressed) {
              throw new AssertionError(
                  String.format(
                      "Unexpected metaState 0 for key 0x%x during an ACTION_down event.",
                      eventPhysicalKey));
            }
            break;
          case kUp:
            // Incoming event is an up. Although the previous state should be pressed, don't
            // synthesize a down event even if it's not. The later code will handle such cases by
            // skipping abrupt up events. Obviously don't synthesize up events either.
            preEventStates[keyIdx] = nowStates[keyIdx];
            break;
          case kRepeat:
            // Incoming event is repeat. The previous state can be either pressed or released.
            // Don't synthesize a down event here, or there will be a down event and a repeat event,
            // both of which sending printable characters. Obviously don't synthesize up events
            // either.
            if (!truePressed) {
              throw new AssertionError(
                  String.format(
                      "Unexpected metaState 0 for key 0x%x during an ACTION_down repeat event.",
                      eventPhysicalKey));
            }
            preEventStates[keyIdx] = nowStates[keyIdx];
            postEventAnyPressed = true;
            break;
        }
      } else {
        postEventAnyPressed = postEventAnyPressed || nowStates[keyIdx];
      }
    }

    // Fill the rest of the pre-event states to match the true state.
    if (truePressed) {
      // It is required that at least one key is pressed.
      for (int keyIdx = 0; keyIdx < goal.keys.length; keyIdx += 1) {
        if (preEventStates[keyIdx] != null) {
          continue;
        }
        if (postEventAnyPressed) {
          preEventStates[keyIdx] = nowStates[keyIdx];
          postEventAnyPressed = postEventAnyPressed || nowStates[keyIdx];
        } else {
          preEventStates[keyIdx] = true;
          postEventAnyPressed = true;
        }
      }
      if (!postEventAnyPressed) {
        preEventStates[0] = true;
      }
    } else {
      for (int keyIdx = 0; keyIdx < goal.keys.length; keyIdx += 1) {
        if (preEventStates[keyIdx] != null) {
          continue;
        }
        preEventStates[keyIdx] = false;
      }
    }

    // Dispatch synthesized events for state differences.
    for (int keyIdx = 0; keyIdx < goal.keys.length; keyIdx += 1) {
      if (nowStates[keyIdx] != preEventStates[keyIdx]) {
        final KeyboardMap.KeyPair key = goal.keys[keyIdx];
        synthesizeEvent(
            preEventStates[keyIdx], key.logicalKey, key.physicalKey, event.getEventTime());
      }
    }
  }

  // Return: if any events has been sent
  private boolean handleEventImpl(
      @NonNull KeyEvent event, @NonNull OnKeyEventHandledCallback onKeyEventHandledCallback) {
    System.out.printf(
        "KeyEvent [0x%x] scan 0x%x key 0x%x uni 0x%x meta 0x%x\n",
        event.getAction(),
        event.getScanCode(),
        event.getKeyCode(),
        event.getUnicodeChar(),
        event.getMetaState());
    final Long physicalKey = getPhysicalKey(event);
    final Long logicalKey = getLogicalKey(event);

    for (final KeyboardMap.PressingGoal goal : KeyboardMap.pressingGoals) {
      // System.out.printf(
      //     "Meta 0x%x mask 0x%x result %d\n",
      //     event.getMetaState(), goal.mask, (event.getMetaState() & goal.mask) != 0 ? 1 : 0);
      synchronizePressingKey(goal, (event.getMetaState() & goal.mask) != 0, physicalKey, event);
    }

    boolean isDownEvent;
    switch (event.getAction()) {
      case KeyEvent.ACTION_DOWN:
        isDownEvent = true;
        break;
      case KeyEvent.ACTION_UP:
        isDownEvent = false;
        break;
      default:
        return false;
    }

    KeyData.Type type;
    String character = null;
    final Long lastLogicalRecord = pressingRecords.get(physicalKey);
    if (isDownEvent) {
      if (lastLogicalRecord == null) {
        type = KeyData.Type.kDown;
      } else {
        // A key has been pressed that has the exact physical key as a currently
        // pressed one.
        if (event.getRepeatCount() > 0) {
          type = KeyData.Type.kRepeat;
        } else {
          synthesizeEvent(false, lastLogicalRecord, physicalKey, event.getEventTime());
          type = KeyData.Type.kDown;
        }
      }
      final char complexChar = applyCombiningCharacterToBaseCharacter(event.getUnicodeChar());
      if (complexChar != 0) {
        character = "" + complexChar;
      }
    } else { // isDownEvent is false
      if (lastLogicalRecord == null) {
        // Ignore abrupt up events.
        return false;
      } else {
        type = KeyData.Type.kUp;
      }
    }

    if (type != KeyData.Type.kRepeat) {
      updatePressingState(physicalKey, isDownEvent ? logicalKey : null);
    }

    final KeyData output = new KeyData();
    output.timestamp = event.getEventTime();
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
    updatePressingState(physicalKey, isDown ? logicalKey : null);
    sendKeyEvent(output, null);
  }

  private void sendKeyEvent(KeyData data, OnKeyEventHandledCallback onKeyEventHandledCallback) {
    final BinaryMessenger.BinaryReply handleMessageReply =
        onKeyEventHandledCallback == null
            ? null
            : message -> {
              Boolean handled = false;
              message.rewind();
              if (message.capacity() != 0) {
                handled = message.get() != 0;
              }
              onKeyEventHandledCallback.onKeyEventHandled(handled);
            };

    messenger.send(KeyData.CHANNEL, data.toBytes(), handleMessageReply);
  }

  @Override
  public void handleEvent(
      @NonNull KeyEvent event, @NonNull OnKeyEventHandledCallback onKeyEventHandledCallback) {
    final boolean sentAny = handleEventImpl(event, onKeyEventHandledCallback);
    if (!sentAny) {
      synthesizeEvent(true, 0l, 0l, 0l);
      onKeyEventHandledCallback.onKeyEventHandled(true);
    }
  }
}
