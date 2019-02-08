package io.flutter.embedding.engine.android;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;

import io.flutter.embedding.engine.systemchannels.KeyEventChannel;

public class AndroidKeyProcessor {
  private final KeyEventChannel keyEventChannel;
  private int baseCharacter;

  public AndroidKeyProcessor(@NonNull KeyEventChannel keyEventChannel) {
    this.keyEventChannel = keyEventChannel;
  }

  public void onKeyUp(@NonNull KeyEvent keyEvent) {
    Character complexCharacter = applyKeyToBaseCharacter(keyEvent.getUnicodeChar());
    keyEventChannel.keyUp(
        new KeyEventChannel.FlutterKeyEvent(keyEvent, complexCharacter)
    );
  }

  public void onKeyDown(@NonNull KeyEvent keyEvent) {
    Character complexCharacter = applyKeyToBaseCharacter(keyEvent.getUnicodeChar());
    keyEventChannel.keyDown(
        new KeyEventChannel.FlutterKeyEvent(keyEvent, complexCharacter)
    );
  }

  /**
   * Applies the given unicode character in {@code codePoint} to a previously entered
   * unicode character and returns the combination of these characters if a combination
   * exists.
   *
   * If the given unicode character is the first to be pressed in a session, or if
   * the new unicode character cannot be combined with the previous unicode character,
   * null is returned.
   *
   * This method mutates {@link #baseCharacter} over time to combine characters.
   */
  @Nullable
  private Character applyKeyToBaseCharacter(int codePoint) {
    if (codePoint == 0) {
      return null;
    }

    if ((codePoint & KeyCharacterMap.COMBINING_ACCENT) != 0) {
      // If a combining character was entered before, combine this one with that one.
      int plainCodePoint = codePoint & KeyCharacterMap.COMBINING_ACCENT_MASK;
      if (baseCharacter != 0) {
        baseCharacter = KeyCharacterMap.getDeadChar(baseCharacter, plainCodePoint);
      } else {
        baseCharacter = plainCodePoint;
      }
    } else {
      if (baseCharacter != 0) {
        int combinedChar = KeyCharacterMap.getDeadChar(baseCharacter, codePoint);
        if (combinedChar > 0) {
          codePoint = combinedChar;
        }
        baseCharacter = 0;
      }
      return (char) codePoint;
    }

    return null;
  }
}
