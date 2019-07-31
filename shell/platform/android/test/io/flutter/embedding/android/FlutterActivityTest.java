package io.flutter.embedding.android;

import android.content.Intent;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.android.controller.ActivityController;
import org.robolectric.annotation.Config;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@Config(manifest=Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterActivityTest {
  @Test
  public void itCreatesDefaultIntentWithExpectedDefaults() {
    Intent intent = FlutterActivity.createDefaultIntent(RuntimeEnvironment.application);
    ActivityController<FlutterActivity> activityController = Robolectric.buildActivity(FlutterActivity.class, intent);

    assertEquals("main", activityController.get().getDartEntrypointFunctionName());
  }

  @Test
  public void itCreatesNewEngineIntentWithRequestedSettings() {
    assertTrue(false);
  }

  @Test
  public void itCreatesCachedEngineIntentThatDoesNotDestroyTheEngine() {
    assertTrue(false);
  }

  @Test
  public void itCreatesCachedEngineIntentThatDestroysTheEngine() {
    assertTrue(false);
  }
}
