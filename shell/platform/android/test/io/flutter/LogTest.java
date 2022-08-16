package io.flutter;

import static org.junit.Assert.assertTrue;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(AndroidJUnit4.class)
public class LogTest {
  @Test
  public void canGetStackTrace() {
    assertTrue(Log.getStackTraceString(new Exception()).contains("canGetStackTrace"));
  }
}
