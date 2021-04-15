package io.flutter.embedding.android;

import android.view.KeyEvent;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel;

/**
 * A light wrapper around a {@link KeyEventChannel} that turns it into a {@link PrimaryResponder}.
 */
class KeyChannelResponder implements KeyboardManager.PrimaryResponder {
  private static final String TAG = "KeyChannelResponder";

  @NonNull private final KeyEventChannel keyEventChannel;
  private final AndroidKeyProcessor keyProcessor = new AndroidKeyProcessor();

  KeyChannelResponder(@NonNull KeyEventChannel keyEventChannel) {
    this.keyEventChannel = keyEventChannel;
  }

  @Override
  public void handleEvent(
      @NonNull KeyEvent keyEvent, @NonNull OnKeyEventHandledCallback onKeyEventHandledCallback) {
    final int action = keyEvent.getAction();
    if (action != KeyEvent.ACTION_DOWN && action != KeyEvent.ACTION_UP) {
      // There is theoretically a KeyEvent.ACTION_MULTIPLE, but theoretically
      // that isn't sent by Android anymore, so this is just for protection in
      // case the theory is wrong.
      onKeyEventHandledCallback.onKeyEventHandled(false);
      return;
    }

    final Character complexCharacter =
        keyProcessor.applyCombiningCharacterToBaseCharacter(keyEvent.getUnicodeChar());
    KeyEventChannel.FlutterKeyEvent flutterEvent =
        new KeyEventChannel.FlutterKeyEvent(keyEvent, complexCharacter);

    final boolean isKeyUp = action != KeyEvent.ACTION_DOWN;
    keyEventChannel.sendFlutterKeyEvent(
        flutterEvent,
        isKeyUp,
        (isEventHandled) -> onKeyEventHandledCallback.onKeyEventHandled(isEventHandled));
    return;
  }
}
