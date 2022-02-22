package io.flutter.util;

import android.view.KeyEvent;
import java.util.HashMap;
import java.util.Map;

// In the test environment, keyEvent.getUnicodeChar throws an exception. This
// class works around the exception by hardcoding the returned value.
public class FakeKeyEvent extends KeyEvent {

  private Map<Integer, Integer> keyCodeMap = new HashMap<>();

  public FakeKeyEvent(int action, int keyCode) {
    super(action, keyCode);
    keyCodeMap.put(KEYCODE_1, (int) '1');
    keyCodeMap.put(KEYCODE_2, (int) '2');
    keyCodeMap.put(KEYCODE_3, (int) '3');
    keyCodeMap.put(KEYCODE_4, (int) '4');
    keyCodeMap.put(KEYCODE_5, (int) '5');
    keyCodeMap.put(KEYCODE_6, (int) '6');
    keyCodeMap.put(KEYCODE_7, (int) '7');
    keyCodeMap.put(KEYCODE_8, (int) '8');
    keyCodeMap.put(KEYCODE_9, (int) '9');
    keyCodeMap.put(KEYCODE_0, (int) '0');
  }

  public final int getUnicodeChar() {
    int code = getKeyCode();
    if (code == KeyEvent.KEYCODE_BACK) {
      return 0;
    }
    if (keyCodeMap.containsKey(code)) {
      return keyCodeMap.get(code);
    }
    return 1;
  }
}
