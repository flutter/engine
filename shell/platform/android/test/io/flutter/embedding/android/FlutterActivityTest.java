package io.flutter.embedding.android;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;
import static org.robolectric.Shadows.shadowOf;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.embedding.android.FlutterActivityLaunchConfigs.BackgroundMode;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.loader.FlutterLoader;
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
public class FlutterActivityTest {
  @Before
  public void setUp() {
    GeneratedPluginRegistrant.clearRegisteredEngines();
  }

  @After
  public void tearDown() {
    GeneratedPluginRegistrant.clearRegisteredEngines();
  }

  @Test
  public void itCreatesDefaultIntentWithExpectedDefaults() {
    Intent intent = FlutterActivity.createDefaultIntent(RuntimeEnvironment.application);
    ActivityController<FlutterActivity> activityController =
        Robolectric.buildActivity(FlutterActivity.class, intent);
    FlutterActivity flutterActivity = activityController.get();
    flutterActivity.setDelegate(new FlutterActivityAndFragmentDelegate(flutterActivity));

    assertEquals("main", flutterActivity.getDartEntrypointFunctionName());
    assertEquals("/", flutterActivity.getInitialRoute());
    assertArrayEquals(new String[] {}, flutterActivity.getFlutterShellArgs().toArray());
    assertTrue(flutterActivity.shouldAttachEngineToActivity());
    assertNull(flutterActivity.getCachedEngineId());
    assertTrue(flutterActivity.shouldDestroyEngineWithHost());
    assertEquals(BackgroundMode.opaque, flutterActivity.getBackgroundMode());
    assertEquals(RenderMode.surface, flutterActivity.getRenderMode());
    assertEquals(TransparencyMode.opaque, flutterActivity.getTransparencyMode());
  }

  @Test
  public void itDestroysNewEngineWhenIntentIsMissingParameter() {
    // All clients should use the static members of FlutterActivity to construct an
    // Intent. Missing extras is an error. However, Flutter has number of tests that
    // don't seem to use the static members of FlutterActivity to construct the
    // launching Intent, so this test explicitly verifies that even illegal Intents
    // result in the automatic destruction of a non-cached FlutterEngine, which prevents
    // the breakage of memory usage benchmark tests.
    Intent intent = new Intent(RuntimeEnvironment.application, FlutterActivity.class);
    ActivityController<FlutterActivity> activityController =
        Robolectric.buildActivity(FlutterActivity.class, intent);
    FlutterActivity flutterActivity = activityController.get();
    flutterActivity.setDelegate(new FlutterActivityAndFragmentDelegate(flutterActivity));

    assertTrue(flutterActivity.shouldDestroyEngineWithHost());
  }

  @Test
  public void itDoesNotDestroyFlutterEngineWhenProvidedByHost() {
    Intent intent =
        new Intent(RuntimeEnvironment.application, FlutterActivityWithProvidedEngine.class);
    ActivityController<FlutterActivityWithProvidedEngine> activityController =
        Robolectric.buildActivity(FlutterActivityWithProvidedEngine.class, intent);
    activityController.create();
    FlutterActivityWithProvidedEngine flutterActivity = activityController.get();

    assertFalse(flutterActivity.shouldDestroyEngineWithHost());
  }

  @Test
  public void itCreatesNewEngineIntentWithRequestedSettings() {
    Intent intent =
        FlutterActivity.withNewEngine()
            .initialRoute("/custom/route")
            .backgroundMode(BackgroundMode.transparent)
            .build(RuntimeEnvironment.application);
    ActivityController<FlutterActivity> activityController =
        Robolectric.buildActivity(FlutterActivity.class, intent);
    FlutterActivity flutterActivity = activityController.get();
    flutterActivity.setDelegate(new FlutterActivityAndFragmentDelegate(flutterActivity));

    assertEquals("/custom/route", flutterActivity.getInitialRoute());
    assertArrayEquals(new String[] {}, flutterActivity.getFlutterShellArgs().toArray());
    assertTrue(flutterActivity.shouldAttachEngineToActivity());
    assertNull(flutterActivity.getCachedEngineId());
    assertTrue(flutterActivity.shouldDestroyEngineWithHost());
    assertEquals(BackgroundMode.transparent, flutterActivity.getBackgroundMode());
    assertEquals(RenderMode.texture, flutterActivity.getRenderMode());
    assertEquals(TransparencyMode.transparent, flutterActivity.getTransparencyMode());
  }

  @Test
  public void itCreatesCachedEngineIntentThatDoesNotDestroyTheEngine() {
    Intent intent =
        FlutterActivity.withCachedEngine("my_cached_engine")
            .destroyEngineWithActivity(false)
            .build(RuntimeEnvironment.application);
    ActivityController<FlutterActivity> activityController =
        Robolectric.buildActivity(FlutterActivity.class, intent);
    FlutterActivity flutterActivity = activityController.get();

    assertArrayEquals(new String[] {}, flutterActivity.getFlutterShellArgs().toArray());
    assertTrue(flutterActivity.shouldAttachEngineToActivity());
    assertEquals("my_cached_engine", flutterActivity.getCachedEngineId());
    assertFalse(flutterActivity.shouldDestroyEngineWithHost());
  }

  @Test
  public void itCreatesCachedEngineIntentThatDestroysTheEngine() {
    Intent intent =
        FlutterActivity.withCachedEngine("my_cached_engine")
            .destroyEngineWithActivity(true)
            .build(RuntimeEnvironment.application);
    ActivityController<FlutterActivity> activityController =
        Robolectric.buildActivity(FlutterActivity.class, intent);
    FlutterActivity flutterActivity = activityController.get();

    assertArrayEquals(new String[] {}, flutterActivity.getFlutterShellArgs().toArray());
    assertTrue(flutterActivity.shouldAttachEngineToActivity());
    assertEquals("my_cached_engine", flutterActivity.getCachedEngineId());
    assertTrue(flutterActivity.shouldDestroyEngineWithHost());
  }

  @Test
  public void itRegistersPluginsAtConfigurationTime() {
    FlutterActivity activity =
        Robolectric.buildActivity(FlutterActivityWithProvidedEngine.class).get();
    activity.onCreate(null);

    assertTrue(GeneratedPluginRegistrant.getRegisteredEngines().isEmpty());
    activity.configureFlutterEngine(activity.getFlutterEngine());

    List<FlutterEngine> registeredEngines = GeneratedPluginRegistrant.getRegisteredEngines();
    assertEquals(1, registeredEngines.size());
    assertEquals(activity.getFlutterEngine(), registeredEngines.get(0));
  }

  @Test
  public void itDoesNotCrashWhenSplashScreenMetadataIsNotDefined() {
    Intent intent = FlutterActivity.createDefaultIntent(RuntimeEnvironment.application);
    ActivityController<FlutterActivity> activityController =
        Robolectric.buildActivity(FlutterActivity.class, intent);
    FlutterActivity flutterActivity = activityController.get();

    // We never supplied the metadata to the robolectric activity info so it doesn't exist.
    SplashScreen splashScreen = flutterActivity.provideSplashScreen();
    // It should quietly return a null and not crash.
    assertNull(splashScreen);
  }

  @Test
  public void itDoesNotCrashWhenSplashScreenDrawableDoesNotExist() {
    Intent intent = FlutterActivity.createDefaultIntent(RuntimeEnvironment.application);
    ActivityController<FlutterActivity> activityController =
        Robolectric.buildActivity(FlutterActivity.class, intent);
    FlutterActivity flutterActivity = activityController.get();

    ResolveInfo resolveInfo = new ResolveInfo();
    resolveInfo.activityInfo = new ActivityInfo();
    resolveInfo.activityInfo.name = "FlutterActivity";
    resolveInfo.activityInfo.applicationInfo = new ApplicationInfo();
    resolveInfo.activityInfo.applicationInfo.packageName = "io.flutter.test";
    resolveInfo.activityInfo.metaData = new Bundle();
    resolveInfo.activityInfo.metaData.putInt(FlutterActivityLaunchConfigs.NORMAL_THEME_META_DATA_KEY, 1234);
    shadowOf(RuntimeEnvironment.application.getPackageManager())
        .addResolveInfoForIntent(intent, resolveInfo);

    System.out.println("++++++ classname " + flutterActivity.getClass().toString());
    System.out.println("++++++ component name is " + flutterActivity.getComponentName());
    System.out.println("++++++ package manager is " + RuntimeEnvironment.application.getPackageManager().getClass());
    ActivityInfo activityInfo =
    shadowOf(RuntimeEnvironment.application.getPackageManager()).getActivityInfo(flutterActivity.getComponentName(), PackageManager.GET_META_DATA);
    System.out.println("++++++ activityinfo is " + activityInfo);

    // We never supplied the metadata to the robolectric activity info so it doesn't exist.
    SplashScreen splashScreen = flutterActivity.provideSplashScreen();
    // It should quietly return a null and not crash.
    assertNull(splashScreen);
  }

  static class FlutterActivityWithProvidedEngine extends FlutterActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
      super.delegate = new FlutterActivityAndFragmentDelegate(this);
      super.delegate.setupFlutterEngine();
    }

    @Nullable
    @Override
    public FlutterEngine provideFlutterEngine(@NonNull Context context) {
      FlutterJNI flutterJNI = mock(FlutterJNI.class);
      when(flutterJNI.isAttached()).thenReturn(true);

      return new FlutterEngine(
          context, mock(FlutterLoader.class), flutterJNI, new String[] {}, false);
    }
  }
}
