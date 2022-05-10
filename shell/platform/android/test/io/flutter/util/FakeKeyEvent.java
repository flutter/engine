package io.flutter.util;

import android.view.KeyEvent;

// In the test environment, keyEvent.getUnicodeChar throws an exception. This
// class works around the exception by hardcoding the returned value.
public class FakeKeyEvent extends KeyEvent {
  public FakeKeyEvent(int action, int keyCode) {
    super(action, keyCode);
  }

  public FakeKeyEvent(
      long eventTime,
      int action,
      int code,
      int repeat,
      int metaState,
      int scancode,
      char character) {
    super(
        /*downTime*/ eventTime,
        eventTime,
        action,
        code,
        repeat,
        metaState, /*deviceId*/
        0,
        scancode);
    this.character = character;
  }

  private char character = 0;

  public final int getUnicodeChar() {
    if (getKeyCode() == KeyEvent.KEYCODE_BACK) {
      return 0;
    }
    return character;
  }
}
