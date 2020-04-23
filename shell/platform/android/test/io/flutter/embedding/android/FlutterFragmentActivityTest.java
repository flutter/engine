package io.flutter.embedding.android;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.plugins.GeneratedPluginRegistrant;
import java.util.List;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.android.controller.ActivityController;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterFragmentActivityTest {
  @Before
  public void setUp() {
    GeneratedPluginRegistrant.clearRegisteredEngines();
  }

  @After
  public void tearDown() {
    GeneratedPluginRegistrant.clearRegisteredEngines();
  }

  @Test
  public void itDoesNotCrashWhenNoSplashDrawableIsGiven() {
    Intent intent = FlutterFragmentActivity.createDefaultIntent(RuntimeEnvironment.application);
    ActivityController<FlutterFragmentActivity> activityController =
        Robolectric.buildActivity(FlutterFragmentActivity.class, intent);
    FlutterFragmentActivity fragmentActivity = activityController.get();

    // We never supplied the resource key to robolectric so it doesn't exist.
    SplashScreen splashScreen = fragmentActivity.provideSplashScreen();
    // It should quietly return a null and not crash.
    assertNull(splashScreen);
  }
}
