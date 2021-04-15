package io.flutter.embedding.android;

import static junit.framework.TestCase.assertEquals;

import android.annotation.TargetApi;
import android.view.KeyCharacterMap;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
@TargetApi(28)
public class AndroidKeyProcessorTest {
  AndroidKeyProcessor keyProcessor;
  private static final int DEAD_KEY = '`' | KeyCharacterMap.COMBINING_ACCENT;

  @Before
  public void setUp() {
    keyProcessor = new AndroidKeyProcessor();
  }

  @Test
  public void basicCombingCharactersTest() {
    assertEquals(0, (int) keyProcessor.applyCombiningCharacterToBaseCharacter(0));
    assertEquals('A', (int) keyProcessor.applyCombiningCharacterToBaseCharacter('A'));
    assertEquals('B', (int) keyProcessor.applyCombiningCharacterToBaseCharacter('B'));
    assertEquals('B', (int) keyProcessor.applyCombiningCharacterToBaseCharacter('B'));
    assertEquals(0, (int) keyProcessor.applyCombiningCharacterToBaseCharacter(0));
    assertEquals(0, (int) keyProcessor.applyCombiningCharacterToBaseCharacter(0));

    assertEquals('`', (int) keyProcessor.applyCombiningCharacterToBaseCharacter(DEAD_KEY));
    assertEquals('`', (int) keyProcessor.applyCombiningCharacterToBaseCharacter(DEAD_KEY));
    assertEquals('À', (int) keyProcessor.applyCombiningCharacterToBaseCharacter('A'));

    assertEquals('`', (int) keyProcessor.applyCombiningCharacterToBaseCharacter(DEAD_KEY));
    assertEquals(0, (int) keyProcessor.applyCombiningCharacterToBaseCharacter(0));
    // The 0 input should remove the combining state.
    assertEquals('A', (int) keyProcessor.applyCombiningCharacterToBaseCharacter('A'));

    assertEquals(0, (int) keyProcessor.applyCombiningCharacterToBaseCharacter(0));
    assertEquals('`', (int) keyProcessor.applyCombiningCharacterToBaseCharacter(DEAD_KEY));
    assertEquals('À', (int) keyProcessor.applyCombiningCharacterToBaseCharacter('A'));
  }
}
