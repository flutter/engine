// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import static android.view.WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS;
import static android.view.WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.os.Build;
import android.view.View;
import android.view.Window;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import androidx.activity.OnBackPressedCallback;
import androidx.fragment.app.FragmentActivity;
import androidx.test.core.app.ApplicationProvider;
import io.flutter.embedding.engine.systemchannels.PlatformChannel;
import io.flutter.embedding.engine.systemchannels.PlatformChannel.Brightness;
import io.flutter.embedding.engine.systemchannels.PlatformChannel.ClipboardContentFormat;
import io.flutter.embedding.engine.systemchannels.PlatformChannel.SystemChromeStyle;
import io.flutter.plugin.platform.PlatformPlugin.PlatformPluginDelegate;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.android.controller.ActivityController;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.ShadowLooper;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class PlatformPluginTest {
  private final Context ctx = ApplicationProvider.getApplicationContext();

  private View fakeDecorView = mock(View.class);
  private Window fakeWindow = mock(Window.class);
  private Activity fakeActivity = mock(Activity.class);
  private PlatformChannel mockPlatformChannel = mock(PlatformChannel.class);
  private PlatformPluginDelegate mockPlatformPluginDelegate = mock(PlatformPluginDelegate.class);
  private PlatformPlugin testPlatformPlugin = new PlatformPlugin(fakeActivity, mockPlatformChannel);
  private PlatformPlugin testPlatformPluginWithDelegate =
      new PlatformPlugin(fakeActivity, mockPlatformChannel, mockPlatformPluginDelegate);

  private ClipboardManager clipboardManager;
  private ClipboardContentFormat clipboardFormat;

  @Before
  public void setUp() {
    when(fakeWindow.getDecorView()).thenReturn(fakeDecorView);
    when(fakeActivity.getWindow()).thenReturn(fakeWindow);
  }

  public void setUpForTextClipboardTests() {
    clipboardManager = spy(ctx.getSystemService(ClipboardManager.class));
    when(fakeActivity.getSystemService(Context.CLIPBOARD_SERVICE)).thenReturn(clipboardManager);
    clipboardFormat = ClipboardContentFormat.PLAIN_TEXT;
  }

  @Config(sdk = Build.VERSION_CODES.KITKAT)
  @Test
  public void itIgnoresNewHapticEventsOnOldAndroidPlatforms() {
    // HEAVY_IMPACT haptic response is only available on "M" (23) and later.
    testPlatformPlugin.vibrateHapticFeedback(PlatformChannel.HapticFeedbackType.HEAVY_IMPACT);

    // SELECTION_CLICK haptic response is only available on "LOLLIPOP" (21) and later.
    testPlatformPlugin.vibrateHapticFeedback(PlatformChannel.HapticFeedbackType.SELECTION_CLICK);
  }

  @Test
  public void platformPlugin_getClipboardDataIsNonNullWhenPlainTextCopied() throws IOException {
    setUpForTextClipboardTests();
    ClipData clip = ClipData.newPlainText("label", "Text");

    assertNull(testPlatformPlugin.mPlatformMessageHandler.getClipboardData(clipboardFormat));
    clipboardManager.setPrimaryClip(clip);
    assertNotNull(testPlatformPlugin.mPlatformMessageHandler.getClipboardData(clipboardFormat));
  }

  @Test
  public void platformPlugin_getClipboardDataIsNonNullWhenContentUriWithTextProvided()
      throws IOException {
    setUpForTextClipboardTests();
    ContentResolver contentResolver = mock(ContentResolver.class);
    ClipData.Item mockItem = mock(ClipData.Item.class);
    Uri mockUri = mock(Uri.class);

    when(fakeActivity.getContentResolver()).thenReturn(contentResolver);
    when(mockUri.getScheme()).thenReturn("content");
    when(mockItem.getText()).thenReturn(null);
    when(mockItem.getUri()).thenReturn(mockUri);
    when(contentResolver.openTypedAssetFileDescriptor(any(Uri.class), any(), any()))
        .thenReturn(mock(AssetFileDescriptor.class));
    when(mockItem.coerceToText(fakeActivity)).thenReturn("something non-null");

    ClipData clip = new ClipData("label", new String[0], mockItem);

    assertNull(testPlatformPlugin.mPlatformMessageHandler.getClipboardData(clipboardFormat));
    clipboardManager.setPrimaryClip(clip);
    assertNotNull(testPlatformPlugin.mPlatformMessageHandler.getClipboardData(clipboardFormat));
  }

  @Test
  public void platformPlugin_getClipboardDataIsNullWhenContentUriProvidedContainsNoText()
      throws IOException {
    setUpForTextClipboardTests();
    ContentResolver contentResolver = ctx.getContentResolver();

    when(fakeActivity.getContentResolver()).thenReturn(contentResolver);

    Uri uri = Uri.parse("content://media/external_primary/images/media/");
    ClipData clip = ClipData.newUri(contentResolver, "URI", uri);

    clipboardManager.setPrimaryClip(clip);
    assertNull(testPlatformPlugin.mPlatformMessageHandler.getClipboardData(clipboardFormat));
  }

  @Test
  public void platformPlugin_getClipboardDataIsNullWhenNonContentUriProvided() throws IOException {
    setUpForTextClipboardTests();
    ContentResolver contentResolver = ctx.getContentResolver();

    when(fakeActivity.getContentResolver()).thenReturn(contentResolver);

    Uri uri = Uri.parse("file:///");
    ClipData clip = ClipData.newUri(contentResolver, "URI", uri);

    clipboardManager.setPrimaryClip(clip);
    assertNull(testPlatformPlugin.mPlatformMessageHandler.getClipboardData(clipboardFormat));
  }

  @Test
  public void platformPlugin_getClipboardDataIsNullWhenItemHasNoTextNorUri() throws IOException {
    setUpForTextClipboardTests();
    ClipData.Item mockItem = mock(ClipData.Item.class);

    when(mockItem.getText()).thenReturn(null);
    when(mockItem.getUri()).thenReturn(null);

    ClipData clip = new ClipData("label", new String[0], mockItem);

    clipboardManager.setPrimaryClip(clip);
    assertNull(testPlatformPlugin.mPlatformMessageHandler.getClipboardData(clipboardFormat));
  }

  @SuppressWarnings("deprecation")
  // ClipboardManager.getText
  @Config(sdk = Build.VERSION_CODES.P)
  @Test
  public void platformPlugin_hasStrings() {
    setUpForTextClipboardTests();

    // Plain text
    ClipData clip = ClipData.newPlainText("label", "Text");
    clipboardManager.setPrimaryClip(clip);
    assertTrue(testPlatformPlugin.mPlatformMessageHandler.clipboardHasStrings());

    // Empty plain text
    clip = ClipData.newPlainText("", "");
    clipboardManager.setPrimaryClip(clip);
    // Without actually accessing clipboard data (preferred behavior), it is not possible to
    // distinguish between empty and non-empty string contents.
    assertTrue(testPlatformPlugin.mPlatformMessageHandler.clipboardHasStrings());

    // HTML text
    clip = ClipData.newHtmlText("motto", "Don't be evil", "<b>Don't</b> be evil");
    clipboardManager.setPrimaryClip(clip);
    assertTrue(testPlatformPlugin.mPlatformMessageHandler.clipboardHasStrings());

    // Text MIME type
    clip = new ClipData("label", new String[] {"text/something"}, new ClipData.Item("content"));
    clipboardManager.setPrimaryClip(clip);
    assertTrue(testPlatformPlugin.mPlatformMessageHandler.clipboardHasStrings());

    // Other MIME type
    clip =
        new ClipData(
            "label", new String[] {"application/octet-stream"}, new ClipData.Item("content"));
    clipboardManager.setPrimaryClip(clip);
    assertFalse(testPlatformPlugin.mPlatformMessageHandler.clipboardHasStrings());

    if (Build.VERSION.SDK_INT >= 28) {
      // Empty clipboard
      clipboardManager.clearPrimaryClip();
      assertFalse(testPlatformPlugin.mPlatformMessageHandler.clipboardHasStrings());
    }

    // Verify that the clipboard contents are never accessed.
    verify(clipboardManager, never()).getPrimaryClip();
    verify(clipboardManager, never()).getText();
  }

  @Config(sdk = Build.VERSION_CODES.Q)
  @Test
  public void setNavigationBarDividerColor() {
    if (Build.VERSION.SDK_INT >= 28) {
      // Default style test
      SystemChromeStyle style =
          new SystemChromeStyle(
              0XFF000000, // statusBarColor
              Brightness.LIGHT, // statusBarIconBrightness
              true, // systemStatusBarContrastEnforced
              0XFFC70039, // systemNavigationBarColor
              Brightness.LIGHT, // systemNavigationBarIconBrightness
              0XFF006DB3, // systemNavigationBarDividerColor
              true); // systemNavigationBarContrastEnforced

      testPlatformPlugin.mPlatformMessageHandler.setSystemUiOverlayStyle(style);

      verify(fakeWindow).setStatusBarColor(0xFF000000);
      verify(fakeWindow).setNavigationBarColor(0XFFC70039);
      verify(fakeWindow).setNavigationBarDividerColor(0XFF006DB3);
      verify(fakeWindow).setStatusBarContrastEnforced(true);
      verify(fakeWindow).setNavigationBarContrastEnforced(true);

      // Regression test for https://github.com/flutter/flutter/issues/88431
      // A null brightness should not affect changing color settings.
      style =
          new SystemChromeStyle(
              0XFF006DB3, // statusBarColor
              null, // statusBarIconBrightness
              false, // systemStatusBarContrastEnforced
              0XFF000000, // systemNavigationBarColor
              null, // systemNavigationBarIconBrightness
              0XFF006DB3, // systemNavigationBarDividerColor
              false); // systemNavigationBarContrastEnforced

      testPlatformPlugin.mPlatformMessageHandler.setSystemUiOverlayStyle(style);

      verify(fakeWindow).setStatusBarColor(0XFF006DB3);
      verify(fakeWindow).setNavigationBarColor(0XFF000000);
      verify(fakeWindow, times(2)).setNavigationBarDividerColor(0XFF006DB3);
      verify(fakeWindow).setStatusBarContrastEnforced(false);
      verify(fakeWindow).setNavigationBarContrastEnforced(false);

      // Null contrasts values should be allowed.
      style =
          new SystemChromeStyle(
              0XFF006DB3, // statusBarColor
              null, // statusBarIconBrightness
              null, // systemStatusBarContrastEnforced
              0XFF000000, // systemNavigationBarColor
              null, // systemNavigationBarIconBrightness
              0XFF006DB3, // systemNavigationBarDividerColor
              null); // systemNavigationBarContrastEnforced

      testPlatformPlugin.mPlatformMessageHandler.setSystemUiOverlayStyle(style);

      verify(fakeWindow, times(2)).setStatusBarColor(0XFF006DB3);
      verify(fakeWindow, times(2)).setNavigationBarColor(0XFF000000);
      verify(fakeWindow, times(3)).setNavigationBarDividerColor(0XFF006DB3);
      // Count is 1 each from earlier calls
      verify(fakeWindow, times(1)).setStatusBarContrastEnforced(true);
      verify(fakeWindow, times(1)).setNavigationBarContrastEnforced(true);
      verify(fakeWindow, times(1)).setStatusBarContrastEnforced(false);
      verify(fakeWindow, times(1)).setNavigationBarContrastEnforced(false);
    }
  }

  @Config(sdk = Build.VERSION_CODES.R)
  @Test
  public void setNavigationBarIconBrightness() {
    if (Build.VERSION.SDK_INT >= 30) {
      WindowInsetsController fakeWindowInsetsController = mock(WindowInsetsController.class);
      when(fakeWindow.getInsetsController()).thenReturn(fakeWindowInsetsController);

      SystemChromeStyle style =
          new SystemChromeStyle(
              null, // statusBarColor
              null, // statusBarIconBrightness
              null, // systemStatusBarContrastEnforced
              null, // systemNavigationBarColor
              Brightness.LIGHT, // systemNavigationBarIconBrightness
              null, // systemNavigationBarDividerColor
              null); // systemNavigationBarContrastEnforced

      testPlatformPlugin.mPlatformMessageHandler.setSystemUiOverlayStyle(style);

      verify(fakeWindowInsetsController)
          .setSystemBarsAppearance(0, APPEARANCE_LIGHT_NAVIGATION_BARS);

      style =
          new SystemChromeStyle(
              null, // statusBarColor
              null, // statusBarIconBrightness
              null, // systemStatusBarContrastEnforced
              null, // systemNavigationBarColor
              Brightness.DARK, // systemNavigationBarIconBrightness
              null, // systemNavigationBarDividerColor
              null); // systemNavigationBarContrastEnforced

      testPlatformPlugin.mPlatformMessageHandler.setSystemUiOverlayStyle(style);

      verify(fakeWindowInsetsController)
          .setSystemBarsAppearance(
              APPEARANCE_LIGHT_NAVIGATION_BARS, APPEARANCE_LIGHT_NAVIGATION_BARS);
    }
  }

  @Config(sdk = Build.VERSION_CODES.R)
  @Test
  public void setStatusBarIconBrightness() {
    if (Build.VERSION.SDK_INT >= 30) {
      WindowInsetsController fakeWindowInsetsController = mock(WindowInsetsController.class);
      when(fakeWindow.getInsetsController()).thenReturn(fakeWindowInsetsController);

      SystemChromeStyle style =
          new SystemChromeStyle(
              null, // statusBarColor
              Brightness.LIGHT, // statusBarIconBrightness
              null, // systemStatusBarContrastEnforced
              null, // systemNavigationBarColor
              null, // systemNavigationBarIconBrightness
              null, // systemNavigationBarDividerColor
              null); // systemNavigationBarContrastEnforced

      testPlatformPlugin.mPlatformMessageHandler.setSystemUiOverlayStyle(style);

      verify(fakeWindowInsetsController).setSystemBarsAppearance(0, APPEARANCE_LIGHT_STATUS_BARS);

      style =
          new SystemChromeStyle(
              null, // statusBarColor
              Brightness.DARK, // statusBarIconBrightness
              null, // systemStatusBarContrastEnforced
              null, // systemNavigationBarColor
              null, // systemNavigationBarIconBrightness
              null, // systemNavigationBarDividerColor
              null); // systemNavigationBarContrastEnforced

      testPlatformPlugin.mPlatformMessageHandler.setSystemUiOverlayStyle(style);

      verify(fakeWindowInsetsController)
          .setSystemBarsAppearance(APPEARANCE_LIGHT_STATUS_BARS, APPEARANCE_LIGHT_STATUS_BARS);
    }
  }

  @SuppressWarnings("deprecation")
  // SYSTEM_UI_FLAG_*, setSystemUiVisibility
  @Config(sdk = Build.VERSION_CODES.Q)
  @Test
  public void setSystemUiMode() {
    if (Build.VERSION.SDK_INT >= 28) {
      testPlatformPlugin.mPlatformMessageHandler.showSystemUiMode(
          PlatformChannel.SystemUiMode.LEAN_BACK);
      verify(fakeDecorView)
          .setSystemUiVisibility(
              View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                  | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                  | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_FULLSCREEN);

      testPlatformPlugin.mPlatformMessageHandler.showSystemUiMode(
          PlatformChannel.SystemUiMode.IMMERSIVE);
      verify(fakeDecorView)
          .setSystemUiVisibility(
              View.SYSTEM_UI_FLAG_IMMERSIVE
                  | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                  | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                  | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_FULLSCREEN);

      testPlatformPlugin.mPlatformMessageHandler.showSystemUiMode(
          PlatformChannel.SystemUiMode.IMMERSIVE_STICKY);
      verify(fakeDecorView)
          .setSystemUiVisibility(
              View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                  | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                  | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                  | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    if (Build.VERSION.SDK_INT >= 29) {
      testPlatformPlugin.mPlatformMessageHandler.showSystemUiMode(
          PlatformChannel.SystemUiMode.EDGE_TO_EDGE);
      verify(fakeDecorView)
          .setSystemUiVisibility(
              View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                  | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
    }
  }

  @SuppressWarnings("deprecation")
  // SYSTEM_UI_FLAG_FULLSCREEN
  @Test
  public void setSystemUiModeListener_overlaysAreHidden() { // HANDLE DIFF
    ActivityController<Activity> controller = Robolectric.buildActivity(Activity.class);
    controller.setup();
    Activity fakeActivity = controller.get();

    PlatformPlugin platformPlugin = new PlatformPlugin(fakeActivity, mockPlatformChannel);

    // Subscribe to system UI visibility events.
    platformPlugin.mPlatformMessageHandler.setSystemUiChangeListener();

    // Simulate system UI changed to full screen.
    fakeActivity
        .getWindow()
        .getDecorView()
        .dispatchSystemUiVisibilityChanged(View.SYSTEM_UI_FLAG_FULLSCREEN);

    // No events should have been sent to the platform channel yet. They are scheduled for
    // the next frame.
    verify(mockPlatformChannel, never()).systemChromeChanged(anyBoolean());

    // Simulate the next frame.
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    // Now the platform channel should receive the event.
    verify(mockPlatformChannel).systemChromeChanged(false);
  }

  @SuppressWarnings("deprecation")
  // dispatchSystemUiVisibilityChanged
  @Test
  public void setSystemUiModeListener_overlaysAreVisible() {
    ActivityController<Activity> controller = Robolectric.buildActivity(Activity.class);
    controller.setup();
    Activity fakeActivity = controller.get();
    PlatformPlugin platformPlugin = new PlatformPlugin(fakeActivity, mockPlatformChannel);

    // Subscribe to system Ui visibility events.
    platformPlugin.mPlatformMessageHandler.setSystemUiChangeListener();

    // Simulate system UI changed to *not* full screen.
    fakeActivity.getWindow().getDecorView().dispatchSystemUiVisibilityChanged(0);

    // No events should have been sent to the platform channel yet. They are scheduled for
    // the next frame.
    verify(mockPlatformChannel, never()).systemChromeChanged(anyBoolean());

    // Simulate the next frame.
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    // Now the platform channel should receive the event.
    verify(mockPlatformChannel).systemChromeChanged(true);
  }

  @SuppressWarnings("deprecation")
  // SYSTEM_UI_FLAG_*, setSystemUiVisibility
  @Config(sdk = Build.VERSION_CODES.P)
  @Test
  public void doNotEnableEdgeToEdgeOnOlderSdk() {
    testPlatformPlugin.mPlatformMessageHandler.showSystemUiMode(
        PlatformChannel.SystemUiMode.EDGE_TO_EDGE);
    verify(fakeDecorView, never())
        .setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
  }

  @SuppressWarnings("deprecation")
  // FLAG_TRANSLUCENT_STATUS, FLAG_TRANSLUCENT_NAVIGATION
  @Config(sdk = Build.VERSION_CODES.Q)
  @Test
  public void verifyWindowFlagsSetToStyleOverlays() {
    SystemChromeStyle style =
        new SystemChromeStyle(
            0XFF000000, Brightness.LIGHT, true, 0XFFC70039, Brightness.LIGHT, 0XFF006DB3, true);

    testPlatformPlugin.mPlatformMessageHandler.setSystemUiOverlayStyle(style);
    verify(fakeWindow).addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
    verify(fakeWindow)
        .clearFlags(
            WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS
                | WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
  }

  @Test
  public void setFrameworkHandlesBackFlutterActivity() {
    testPlatformPluginWithDelegate.mPlatformMessageHandler.setFrameworkHandlesBack(true);

    verify(mockPlatformPluginDelegate, times(1)).setFrameworkHandlesBack(true);
  }

  @Test
  public void popSystemNavigatorFlutterActivity() {
    when(mockPlatformPluginDelegate.popSystemNavigator()).thenReturn(false);

    testPlatformPluginWithDelegate.mPlatformMessageHandler.popSystemNavigator();

    verify(mockPlatformPluginDelegate, times(1)).popSystemNavigator();
    verify(fakeActivity, times(1)).finish();
  }

  @Test
  public void doesNotDoAnythingByDefaultIfPopSystemNavigatorOverridden() {
    when(mockPlatformPluginDelegate.popSystemNavigator()).thenReturn(true);

    testPlatformPluginWithDelegate.mPlatformMessageHandler.popSystemNavigator();

    verify(mockPlatformPluginDelegate, times(1)).popSystemNavigator();
    // No longer perform the default action when overridden.
    verify(fakeActivity, never()).finish();
  }

  @SuppressWarnings("deprecation")
  // Robolectric.setupActivity.
  // TODO(reidbaker): https://github.com/flutter/flutter/issues/133151
  @Test
  public void popSystemNavigatorFlutterFragment() {
    // Migrate to ActivityScenario by following https://github.com/robolectric/robolectric/pull/4736
    FragmentActivity activity = spy(Robolectric.setupActivity(FragmentActivity.class));
    final AtomicBoolean onBackPressedCalled = new AtomicBoolean(false);
    OnBackPressedCallback backCallback =
        new OnBackPressedCallback(true) {
          @Override
          public void handleOnBackPressed() {
            onBackPressedCalled.set(true);
          }
        };
    activity.getOnBackPressedDispatcher().addCallback(backCallback);

    when(mockPlatformPluginDelegate.popSystemNavigator()).thenReturn(false);
    PlatformPlugin platformPlugin =
        new PlatformPlugin(activity, mockPlatformChannel, mockPlatformPluginDelegate);

    platformPlugin.mPlatformMessageHandler.popSystemNavigator();

    verify(activity, never()).finish();
    verify(mockPlatformPluginDelegate, times(1)).popSystemNavigator();
    assertTrue(onBackPressedCalled.get());
  }

  @SuppressWarnings("deprecation")
  // Robolectric.setupActivity.
  // TODO(reidbaker): https://github.com/flutter/flutter/issues/133151
  @Test
  public void doesNotDoAnythingByDefaultIfFragmentPopSystemNavigatorOverridden() {
    FragmentActivity activity = spy(Robolectric.setupActivity(FragmentActivity.class));
    when(mockPlatformPluginDelegate.popSystemNavigator()).thenReturn(true);

    testPlatformPluginWithDelegate.mPlatformMessageHandler.popSystemNavigator();

    verify(mockPlatformPluginDelegate, times(1)).popSystemNavigator();
    // No longer perform the default action when overridden.
    verify(activity, never()).finish();
  }

  @Test
  public void setRequestedOrientationFlutterFragment() {
    FragmentActivity mockFragmentActivity = mock(FragmentActivity.class);
    when(mockPlatformPluginDelegate.popSystemNavigator()).thenReturn(false);
    PlatformPlugin platformPlugin =
        new PlatformPlugin(mockFragmentActivity, mockPlatformChannel, mockPlatformPluginDelegate);

    platformPlugin.mPlatformMessageHandler.setPreferredOrientations(0);

    verify(mockFragmentActivity, times(1)).setRequestedOrientation(0);
  }

  @Test
  public void performsDefaultBehaviorWhenNoDelegateProvided() {
    testPlatformPlugin.mPlatformMessageHandler.popSystemNavigator();

    verify(fakeActivity, times(1)).finish();
  }
}
