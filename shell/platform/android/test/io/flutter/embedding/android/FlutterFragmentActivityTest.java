package io.flutter.embedding.android;

import static org.junit.Assert.assertNull;
import static org.robolectric.Shadows.shadowOf;

import android.content.Intent;
import io.flutter.plugins.GeneratedPluginRegistrant;
import io.flutter.embedding.android.FlutterFragmentActivity;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
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
  public void itDoesNotCrashWhenSplashScreenMetadataIsNotDefined() {
    Intent intent = FlutterFragmentActivity.createDefaultIntent(RuntimeEnvironment.application);
    ActivityController<FlutterFragmentActivity> activityController =
        Robolectric.buildActivity(FlutterFragmentActivity.class, intent);
    FlutterFragmentActivity fragmentActivity = activityController.get();

    // We never supplied the resource key to robolectric so it doesn't exist.
    SplashScreen splashScreen = fragmentActivity.provideSplashScreen();
    // It should quietly return a null and not crash.
    assertNull(splashScreen);
  }

  @Test
  public void itDoesNotCrashWhenSplashScreenDrawableDoesNotExist() {
    Intent intent = FlutterFragmentActivity.createDefaultIntent(RuntimeEnvironment.application);
    ActivityController<FlutterFragmentActivity> activityController =
        Robolectric.buildActivity(FlutterFragmentActivity.class, intent);
    FlutterFragmentActivity fragmentActivity = activityController.get();

    // We supply a metadata to the Flutter fragment activity but the value of the metadata
    // references a drawable ID that does not exist.
    ResolveInfo resolveInfo = new ResolveInfo();
    resolveInfo.activityInfo = new ActivityInfo();
    resolveInfo.activityInfo.name = fragmentActivity.getComponentName().getClassName();
    resolveInfo.activityInfo.applicationInfo = new ApplicationInfo();
    resolveInfo.activityInfo.applicationInfo.packageName =
        RuntimeEnvironment.application.getPackageName();
    resolveInfo.activityInfo.metaData = new Bundle();
    // 1234 is an arbitrary value that is an invalid drawable ID.
    resolveInfo.activityInfo.metaData.putInt(
        FlutterActivityLaunchConfigs.SPLASH_SCREEN_META_DATA_KEY, 1234);
    shadowOf(RuntimeEnvironment.application.getPackageManager())
        .addResolveInfoForIntent(intent, resolveInfo);
    shadowOf(RuntimeEnvironment.application.getPackageManager())
        .addOrUpdateActivity(resolveInfo.activityInfo);
    shadowOf(RuntimeEnvironment.application.getPackageManager())
        .addActivityIfNotPresent(fragmentActivity.getComponentName());

    // Trying to build a splash screen off of this invalid data should not crash.
    SplashScreen splashScreen = fragmentActivity.provideSplashScreen();
    // It should quietly return a null.
    assertNull(splashScreen);
  }
}
