// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterShellArgs;
import io.flutter.plugin.platform.PlatformPlugin;
import io.flutter.view.FlutterMain;

/**
 * {@code Activity} which displays a {@link FlutterFragment} that takes up all available space.
 *
 * {@link FlutterActivity} is the simplest and most direct way to integrate Flutter within an
 * Android app.
 *
 * By default, {@link FlutterActivity} configures itself to do the following:
 *  - no splash screen
 *  - asks {@link FlutterMain#findAppBundlePath(Context)} for the path to the Dart app bundle
 *  - uses a default Dart entrypoint of "main"
 *  - uses a default initial route of "/" within the Flutter app
 *
 * The display of a splash screen, app bundle path, Dart entrypoint, and initial route can each be
 * controlled by overriding their respective methods in a subclass:
 *  - {@link #isSplashScreenDesired()}
 *  - {@link #getAppBundlePath()}
 *  - {@link #getDartEntrypoint()}
 *  - {@link #getInitialRoute()}
 *
 * If Flutter is needed in a location that cannot use an {@code Activity}, consider using
 * a {@link FlutterFragment}. Using a {@link FlutterFragment} requires forwarding some calls from
 * an {@code Activity} to the {@link FlutterFragment}.
 *
 * If Flutter is needed in a location that can only use a {@code View}, consider using a
 * {@link FlutterView}. Using a {@link FlutterView} requires forwarding some calls from an
 * {@code Activity}, as well as forwarding lifecycle calls from an {@code Activity} or a
 * {@code Fragment}.
 *
 * @see FlutterFragment, which presents a Flutter app within a {@code Fragment}
 * @see FlutterView, which renders the UI for a {@link FlutterEngine}
 */
@SuppressLint("Registered")
@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1)
public class FlutterActivity extends FragmentActivity {
  private static final String TAG = "FlutterActivity";

  // Meta-data arguments
  // TODO: where did this package path come from? is this what it should be?
  private static final String SPLASH_SCREEN_META_DATA_KEY = "io.flutter.app.android.SplashScreenUntilFirstFrame";
  // TODO: where did this package path come from? is this what it should be?
  private static final String INITIAL_ROUTE_META_DATA_KEY = "io.flutter.app.android.InitialRoute";

  // Intent extra arguments
  private static final String EXTRA_SHOW_SPLASH_SCREEN = "show_launch_screen";
  private static final String EXTRA_DART_ENTRYPOINT = "dart_entrypoint";
  private static final String EXTRA_INITIAL_ROUTE = "route";

  // FlutterFragment management
  private static final String TAG_FLUTTER_FRAGMENT = "flutter_fragment";
  private static final int FRAGMENT_CONTAINER_ID = 609893468; // random number
  private FlutterFragment flutterFragment;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(createFragmentContainer());
    ensureFlutterFragmentCreated();
    configureStatusBarColor();
  }

  /**
   * Sets the color of this {@code Activity}'s status bar to a translucent black, i.e., a dark
   * overlay.
   *
   * Beginning with Android Lollipop, the status bar at the top of the screen can have its color
   * set at runtime.
   */
  @TargetApi(Build.VERSION_CODES.LOLLIPOP)
  private void configureStatusBarColor() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      Window window = getWindow();
      window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
      window.setStatusBarColor(0x40000000);
      window.getDecorView().setSystemUiVisibility(PlatformPlugin.DEFAULT_SYSTEM_UI);
    }
  }

  /**
   * If no {@code FlutterFragment} exists in this {@code FlutterActivity}, then a {@code FlutterFragment}
   * is created and added. If a {@code FlutterFragment} does exist in this {@code FlutterActivity}, then
   * a reference to that {@code FlutterFragment} is retained in {@code #flutterFragment}.
   */
  private void ensureFlutterFragmentCreated() {
    FragmentManager fragmentManager = getSupportFragmentManager();
    flutterFragment = (FlutterFragment) fragmentManager.findFragmentByTag(TAG_FLUTTER_FRAGMENT);

    if (flutterFragment == null) {
      // No FlutterFragment exists yet. This must be the initial Activity creation. We will create
      // and add a new FlutterFragment to this Activity.
      flutterFragment = createFlutterFragment();
      fragmentManager
          .beginTransaction()
          .add(FRAGMENT_CONTAINER_ID, flutterFragment, TAG_FLUTTER_FRAGMENT)
          .commit();
    }
  }

  /**
   * Creates a {@link FrameLayout} with an ID of {@link #FRAGMENT_CONTAINER_ID} that will contain
   * the {@link FlutterFragment} displayed by this {@link FlutterActivity}.
   *
   * @return the FrameLayout container
   */
  @NonNull
  private View createFragmentContainer() {
    FrameLayout container = new FrameLayout(this);
    container.setId(FRAGMENT_CONTAINER_ID);
    container.setLayoutParams(new ViewGroup.LayoutParams(
        ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.MATCH_PARENT
    ));
    return container;
  }

  /**
   * Factory method to create the instance of the {@link FlutterFragment} that this
   * {@link FlutterActivity} displays.
   *
   * Subclasses may override this method to return a specialization of {@link FlutterFragment}.
   *
   * @return the {@link FlutterFragment} to be displayed in this {@link FlutterActivity}
   */
  @NonNull
  protected FlutterFragment createFlutterFragment() {
    return FlutterFragment.newInstance(
        isSplashScreenDesired(),
        getInitialRoute(),
        getAppBundlePath(),
        getDartEntrypoint(),
        FlutterShellArgs.fromIntent(getIntent())
    );
  }

  @Override
  public void onPostResume() {
    super.onPostResume();
    flutterFragment.onPostResume();
  }

  @Override
  protected void onNewIntent(Intent intent) {
    // Forward Intents to our FlutterFragment in case it cares.
    flutterFragment.onNewIntent(intent);
  }

  @Override
  public void onBackPressed() {
    flutterFragment.onBackPressed();
  }

  // TODO(mattcarroll): there should be an @Override here but the build system is saying it's wrong
  public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
    flutterFragment.onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

  @Override
  public void onUserLeaveHint() {
    flutterFragment.onUserLeaveHint();
  }

  @Override
  public void onTrimMemory(int level) {
    super.onTrimMemory(level);
    flutterFragment.onTrimMemory(level);
  }

  @SuppressWarnings("unused")
  @Nullable
  protected FlutterEngine getFlutterEngine() {
    return flutterFragment.getFlutterEngine();
  }

  /**
   * The path to the bundle that contains this Flutter app's resources, e.g., Dart code snapshots.
   *
   * When this {@link FlutterActivity} is run by Flutter tooling and a data String is included
   * in the launching {@code Intent}, that data String is interpreted as an app bundle path.
   *
   * By default, the app bundle path is obtained from {@link FlutterMain#findAppBundlePath(Context)}.
   *
   * Subclasses may override this method to return a custom app bundle path.
   *
   * @return file path to Flutter's app bundle
   */
  @NonNull
  protected String getAppBundlePath() {
    // If this Activity was launched from tooling, and the incoming Intent contains
    // a custom app bundle path, return that path.
    if (isDebuggable() && Intent.ACTION_RUN.equals(getIntent().getAction())) {
      String appBundlePath = getIntent().getDataString();
      if (appBundlePath != null) {
        return appBundlePath;
      }
    }

    // Return the default app bundle path.
    return FlutterMain.findAppBundlePath(getApplicationContext());
  }

  @Nullable
  protected String getDartEntrypoint() {
    return getIntent().getStringExtra(EXTRA_DART_ENTRYPOINT);
  }

  /**
   * Should the {@code Activity}'s {@code windowBackground} be used as a splash screen
   * until the first frame of Flutter is rendered?
   *
   * This preference can be controlled with 2 methods:
   *  * Pass a boolean as {@link #EXTRA_SHOW_SPLASH_SCREEN} with the launching {@code Intent}, or
   *  * Set a {@code <meta-data>} called {@link #SPLASH_SCREEN_META_DATA_KEY} for this
   *    {@code Activity} in the Android manifest.
   *
   * If both preferences are set, the {@code Intent} preference takes precedence.
   *
   * The reason that a {@code <meta-data>} preference is supported is because this {@code Activity}
   * might be the very first {@code Activity} launched, which means the developer won't have
   * control over the incoming {@code Intent}.
   *
   * Subclasses may override this method to directly control whether or not a splash screen is
   * displayed.
   *
   * TODO(mattcarroll): move all splash behavior to FlutterView
   */
  protected boolean isSplashScreenDesired() {
    if (getIntent().hasExtra(EXTRA_SHOW_SPLASH_SCREEN)) {
      return getIntent().getBooleanExtra(EXTRA_SHOW_SPLASH_SCREEN, false);
    }

    try {
      @SuppressLint("WrongConstant")
      ActivityInfo activityInfo = getPackageManager().getActivityInfo(
          getComponentName(),
          PackageManager.GET_META_DATA|PackageManager.GET_ACTIVITIES
      );
      Bundle metadata = activityInfo.metaData;
      return metadata != null && metadata.getBoolean(SPLASH_SCREEN_META_DATA_KEY);
    } catch (PackageManager.NameNotFoundException e) {
      return false;
    }
  }

  /**
   * The initial route that a Flutter app will render upon loading and executing its Dart code.
   *
   * This preference can be controlled with 2 methods:
   *  * Pass a boolean as {@link #EXTRA_INITIAL_ROUTE} with the launching {@code Intent}, or
   *  * Set a {@code <meta-data>} called {@link #INITIAL_ROUTE_META_DATA_KEY} for this
   *    {@code Activity} in the Android manifest.
   *
   * If both preferences are set, the {@code Intent} preference takes precedence.
   *
   * The reason that a {@code <meta-data>} preference is supported is because this {@code Activity}
   * might be the very first {@code Activity} launched, which means the developer won't have
   * control over the incoming {@code Intent}.
   *
   * Subclasses may override this method to directly control the initial route.
   *
   * @return initial route to be rendered by Flutter
   */
  @Nullable
  protected String getInitialRoute() {
    if (getIntent().hasExtra(EXTRA_INITIAL_ROUTE)) {
      return getIntent().getStringExtra(EXTRA_INITIAL_ROUTE);
    }

    try {
      @SuppressLint("WrongConstant")
      ActivityInfo activityInfo = getPackageManager().getActivityInfo(
          getComponentName(),
          PackageManager.GET_META_DATA|PackageManager.GET_ACTIVITIES
      );
      Bundle metadata = activityInfo.metaData;
      return metadata != null ? metadata.getString(INITIAL_ROUTE_META_DATA_KEY) : null;
    } catch (PackageManager.NameNotFoundException e) {
      return null;
    }
  }

  /**
   * Is Flutter running in "debug mode"?
   *
   * Debug mode allows Flutter to operate with hot reload and hot restart. Release mode does not.
   *
   * @return true if Flutter is running in "debug mode", false otherwise
   */
  private boolean isDebuggable() {
    return (getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0;
  }
}
