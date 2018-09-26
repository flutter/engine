// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.FragmentManager;
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
 * {@code Activity} which displays a {@link io.flutter.view.FlutterView} that takes up all
 * available space.
 *
 * {@link FlutterActivity} is the simplest and most direct way to integrate Flutter within an
 * Android app.
 * TODO(mattcarroll): how exactly does FlutterActivity need to be leveraged to display Flutter content?
 *
 * If Flutter is needed in a location that cannot use an {@code Activity>}, consider using
 * a {@link FlutterFragment}. Using a {@link FlutterFragment} requires forwarding some calls from
 * an {@code Activity} to the {@link FlutterFragment}.
 *
 * If Flutter is needed in a location that can only use a {@code View}, consider using a
 * {@link io.flutter.view.FlutterView}. Using a {@link io.flutter.view.FlutterView} requires
 * forwarding some calls from an {@code Activity}, as well as forwarding lifecycle calls from
 * an {@code Activity} or a {@code Fragment}.
 *
 * @see FlutterFragment
 *
 * @see io.flutter.view.FlutterView
 */
@SuppressLint("Registered")
@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1)
public class FlutterActivity extends Activity {
  private static final String TAG = "FlutterActivity";

  private static final String EXTRA_SHOW_SPLASH_SCREEN = "show_launch_screen";
  // TODO: where did this package path come from? is this what it should be?
  private static final String SPLASH_SCREEN_META_DATA_KEY = "io.flutter.app.android.SplashScreenUntilFirstFrame";
  private static final String EXTRA_INITIAL_ROUTE = "route";
  // TODO: where did this package path come from? is this what it should be?
  private static final String INITIAL_ROUTE_META_DATA_KEY = "io.flutter.app.android.InitialRoute";

  private static final String TAG_FLUTTER_FRAGMENT = "flutter_fragment";
  private static final int FRAGMENT_CONTAINER_ID = 609893468; // random number
  private FlutterFragment flutterFragment;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    configureStatusBarColor();
    setContentView(createFragmentContainer());
    ensureFlutterFragmentCreated();
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
   * a reference to that {@code FlutterFragment} is retained in {@code flutterFragment}.
   */
  private void ensureFlutterFragmentCreated() {
    FragmentManager fragmentManager = getFragmentManager();
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
   * the {@link FlutterFragment} displayed by this {@link Activity}.
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
   * @return the {@link FlutterFragment} to be displayed in this {@link FlutterActivity}
   */
  @NonNull
  protected FlutterFragment createFlutterFragment() {
    return FlutterFragment.newInstance(
        isSplashScreenDesired(),
        getInitialRoute(),
        getAppBundlePath(),
        null, // TODO(mattcarroll): introduce Intent based entrypoint selection
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

  public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
    flutterFragment.onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

  @Override
  public void onUserLeaveHint() {
    flutterFragment.onUserLeaveHint();
  }

  @SuppressWarnings("unused")
  @Nullable
  protected FlutterEngine getFlutterEngine() {
    return flutterFragment.getFlutterEngine();
  }

  /**
   * The path to the bundle that contains this Flutter app's resources, e.g., Dart code snapshots.
   *
   * @return file path to Flutter's app bundle
   */
  @NonNull
  private String getAppBundlePath() {
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
   * TODO(mattcarroll): move all splash behavior to FlutterView
   */
  private boolean isSplashScreenDesired() {
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
   * @return initial route to be rendered by Flutter
   */
  @Nullable
  private String getInitialRoute() {
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
