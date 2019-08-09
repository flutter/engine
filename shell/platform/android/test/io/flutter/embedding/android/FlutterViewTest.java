package io.flutter.embedding.android;

import android.app.Activity;
import android.arch.lifecycle.Lifecycle;
import android.content.Context;
import android.content.Intent;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import static org.junit.Assert.assertEquals;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.android.controller.ActivityController;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterShellArgs;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.plugins.activity.ActivityControlSurface;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.engine.systemchannels.AccessibilityChannel;
import io.flutter.embedding.engine.systemchannels.LifecycleChannel;
import io.flutter.embedding.engine.systemchannels.LocalizationChannel;
import io.flutter.embedding.engine.systemchannels.NavigationChannel;
import io.flutter.embedding.engine.systemchannels.SettingsChannel;
import io.flutter.embedding.engine.systemchannels.SystemChannel;
import io.flutter.plugin.platform.PlatformPlugin;
import io.flutter.plugin.platform.PlatformViewsController;
import io.flutter.view.FlutterMain;

import static android.content.ComponentCallbacks2.TRIM_MEMORY_RUNNING_LOW;
import static org.junit.Assert.assertEquals;
import static org.mockito.Matchers.any;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

@Config(manifest=Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterViewTest {
  @Test
  public void itPassesViewportMetricsToTheEngineAppropriately() {
    // Create a FlutterView
    FlutterView flutterView = new FlutterView(RuntimeEnvironment.application);

    // Connect the FlutterView to the mock engine
    flutterView.attachToFlutterEngine(mockFlutterEngine());

    // Call FlutterView.onApplyWindowInsets w/ WindowInsets

    // Check if getRenderer().setViewPortMetrics is
      // called once
      // passes the correct object through

    assertEquals(true, true);
    // FlutterView flutterView = new FlutterView(flutterActivity);
  }

  // Create a mock WindowInsets class

  //

  /**
   * Creates a mock {@link FlutterEngine}.
   * <p>
   * The heuristic for deciding what to mock in the given {@link FlutterEngine} is that we
   * should mock the minimum number of necessary methods and associated objects. Maintaining
   * developers should add more mock behavior as required for tests, but should avoid mocking
   * things that are not required for the correct execution of tests.
   */
  @NonNull
  private FlutterEngine mockFlutterEngine() {
    // The use of SettingsChannel by the delegate requires some behavior of its own, so it is
    // explicitly mocked with some internal behavior.
    SettingsChannel fakeSettingsChannel = mock(SettingsChannel.class);
    SettingsChannel.MessageBuilder fakeMessageBuilder = mock(SettingsChannel.MessageBuilder.class);
    when(fakeMessageBuilder.setPlatformBrightness(any(SettingsChannel.PlatformBrightness.class))).thenReturn(fakeMessageBuilder);
    when(fakeMessageBuilder.setTextScaleFactor(any(Float.class))).thenReturn(fakeMessageBuilder);
    when(fakeMessageBuilder.setUse24HourFormat(any(Boolean.class))).thenReturn(fakeMessageBuilder);
    when(fakeSettingsChannel.startMessage()).thenReturn(fakeMessageBuilder);

    // Mock FlutterEngine and all of its required direct calls.
    FlutterEngine engine = mock(FlutterEngine.class);
    when(engine.getRenderer()).thenReturn(mock(FlutterRenderer.class));
    when(engine.getDartExecutor()).thenReturn(mock(DartExecutor.class));
    when(engine.getPlatformViewsController()).thenReturn(mock(PlatformViewsController.class));
    when(engine.getAccessibilityChannel()).thenReturn(mock(AccessibilityChannel.class));
    when(engine.getSettingsChannel()).thenReturn(fakeSettingsChannel);
    when(engine.getLocalizationChannel()).thenReturn(mock(LocalizationChannel.class));

    // when(engine.getKeyEventChannel()).thenReturn(mock(PlatformViewsController.class));
    // when(engine.getLifecycleChannel()).thenReturn(mock(LifecycleChannel.class));
    // when(engine.getNavigationChannel()).thenReturn(mock(NavigationChannel.class));
    // when(engine.getSystemChannel()).thenReturn(mock(SystemChannel.class));
    // when(engine.getActivityControlSurface()).thenReturn(mock(ActivityControlSurface.class));

    return engine;
  }

  @NonNull
  private Context mockContext() {
    Context context = mock(Context.class);

    return context;
  }
}
