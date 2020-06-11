// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import java.util.Map;
import java.util.HashMap;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel;
import io.flutter.plugin.editing.TextInputPlugin;

public class AndroidKeyProcessor {
  private static final String TAG = "AndroidKeyProcessor";

  @NonNull private final KeyEventChannel keyEventChannel;
  @NonNull private final TextInputPlugin textInputPlugin;
  @NonNull private final Context context;
  private int combiningCharacter;

  private Map<Long, KeyEvent> pendingEvents = new HashMap<Long, KeyEvent>();
  private boolean dispatchingKeyEvent = false;

  public AndroidKeyProcessor(@NonNull Context context, @NonNull KeyEventChannel keyEventChannel, @NonNull TextInputPlugin textInputPlugin) {
    this.keyEventChannel = keyEventChannel;
    this.textInputPlugin = textInputPlugin;
    this.context = context;
    this.keyEventChannel.setKeyProcessor(this);
  }

  public boolean onKeyUp(@NonNull KeyEvent keyEvent) {
    if (dispatchingKeyEvent) {
      // Don't handle it if it is from our own delayed event synthesis.
      return false;
    }

    Character complexCharacter = applyCombiningCharacterToBaseCharacter(keyEvent.getUnicodeChar());
    keyEventChannel.keyUp(new KeyEventChannel.FlutterKeyEvent(keyEvent, complexCharacter));
    pendingEvents.put(keyEvent.getEventTime(), keyEvent);
    return true;
  }

  public boolean onKeyDown(@NonNull KeyEvent keyEvent) {
    if (dispatchingKeyEvent) {
      // Don't handle it if it is from our own delayed event synthesis.
      return false;
    }

    // If the textInputPlugin is still valid and accepting text, then we'll try
    // and send the key event to it, assuming that if the event can be sent, that
    // it has been handled.
    if (textInputPlugin.getLastInputConnection() != null
        && textInputPlugin.getInputMethodManager().isAcceptingText()) {
      if (textInputPlugin.getLastInputConnection().sendKeyEvent(keyEvent)) {
        return true;
      }
    }

    Character complexCharacter = applyCombiningCharacterToBaseCharacter(keyEvent.getUnicodeChar());
    keyEventChannel.keyDown(new KeyEventChannel.FlutterKeyEvent(keyEvent, complexCharacter));
    pendingEvents.put(keyEvent.getEventTime(), keyEvent);
    return true;
  }

  public void onKeyEventHandled(@NonNull long timestamp) {
    if (!pendingEvents.containsKey(timestamp)) {
      Log.e(TAG, "Key with timestamp " + timestamp + " not found in pending key events list");
      return;
    }
    Log.e(TAG, "Removing handled key with timestamp " + timestamp + " from pending key events list");
    // Since this event was already reported to Android as handled, we just
    // remove it from the map of pending events.
    pendingEvents.remove(timestamp);
  }

  public void onKeyEventNotHandled(@NonNull long timestamp) {
    if (!pendingEvents.containsKey(timestamp)) {
      Log.e(TAG, "Key with timestamp " + timestamp + " not found in pending key events list");
      return;
    }
    Log.e(TAG, "Removing unhandled key with timestamp " + timestamp + " from pending key events list");
    // Since this event was NOT handled by the framework we now synthesize a
    // new, identical, key event to pass along.
    KeyEvent pendingEvent = pendingEvents.remove(timestamp);
    Activity activity = getActivity(context);
    if (activity != null) {
      // Turn on dispatchingKeyEvent so that we don't dispatch to ourselves and
      // send it to the framework again.
      dispatchingKeyEvent = true;
      activity.dispatchKeyEvent(pendingEvent);
      dispatchingKeyEvent = false;
    }
  }

  private Activity getActivity(Context context) {
    if (context instanceof Activity) {
      return (Activity) context;
    }
    if (context instanceof ContextWrapper) {
      // Recurse up chain of base contexts until we find an Activity.
      return getActivity(((ContextWrapper) context).getBaseContext());
    }
    return null;
  }

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
  @Nullable
  private Character applyCombiningCharacterToBaseCharacter(int newCharacterCodePoint) {
    if (newCharacterCodePoint == 0) {
      return null;
    }

    Character complexCharacter = (char) newCharacterCodePoint;
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
      // The new character is a regular character. Apply combiningCharacter to it, if it exists.
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
}
