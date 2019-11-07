package test.io.flutter.embedding.engine;

import android.content.Context;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatcher;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.loader.FlutterLoader;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.argThat;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

@Config(manifest=Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterEngineTest {
  @Test
  public void itDoesNotCrashIfGeneratedPluginRegistrantIsUnavailable() {
    FlutterJNI flutterJNI = mock(FlutterJNI.class);
    when(flutterJNI.isAttached()).thenReturn(true);

    FlutterEngine flutterEngine = new FlutterEngine(
        RuntimeEnvironment.application,
        mock(FlutterLoader.class),
        flutterJNI,
        new String[] {},
        true
    );
    // The fact that the above constructor executed without error means that
    // it dealt with a non-existent GeneratedPluginRegistrant.
  }

  @Test
  public void itAddsWriteServiceInfoVmFlagToEveryRun() {
    // Setup test.
    FlutterJNI flutterJNI = mock(FlutterJNI.class);
    when(flutterJNI.isAttached()).thenReturn(true);

    FlutterLoader flutterLoader = mock(FlutterLoader.class);

    // Execute behavior under test.
    FlutterEngine flutterEngine = new FlutterEngine(
        RuntimeEnvironment.application,
        flutterLoader,
        flutterJNI,
        new String[] {},
        true
    );

    // Verify results.
    verify(flutterLoader, times(1)).ensureInitializationComplete(
        any(Context.class),
        argThat(new ArrayContainsString("--write-service-info service_info.json"))
    );
  }

  private static class ArrayContainsString extends ArgumentMatcher<String[]> {
    private String item;

    ArrayContainsString(String item) {
      this.item = item;
    }

    @Override
    public boolean matches(Object argument) {
      if (argument instanceof String[]) {
        String[] array = (String[]) argument;
        for (String value : array) {
          if (value.equals(item)) {
            return true;
          }
        }
        return false;
      } else {
        return false;
      }
    }
  }

}
