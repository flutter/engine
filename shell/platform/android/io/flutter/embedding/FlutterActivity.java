// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

import io.flutter.plugin.platform.PlatformPlugin;
import io.flutter.view.FlutterMain;

/**
 * {@code Activity} which displays a {@link io.flutter.view.FlutterView} that takes up all
 * available space.
 *
 * {@link FlutterActivity} is the simplest and most direct way to integrate Flutter within an
 * Android app.
 * TODO: how exactly does FlutterActivity need to be leveraged to display Flutter content?
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
public class FlutterActivity extends Activity {
  private static final String TAG = "FlutterActivity";
  private static final String TAG_FLUTTER_FRAGMENT = "flutter_fragment";
  private static final int CONTAINER_ID = 609893468; // random number
  private static final String SPLASH_SCREEN_META_DATA_KEY = "io.flutter.app.android.SplashScreenUntilFirstFrame";

  private FlutterFragment flutterFragment;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    FlutterShellArgs args = FlutterShellArgs.fromIntent(getIntent());
    // TODO(mattcarroll): Change FlutterMain to accept FlutterShellArgs and move additional constants in
    //       FlutterMain over to FlutterShellArgs.
    FlutterMain.ensureInitializationComplete(getApplicationContext(), args.toArray());

    configureStatusBarColor();
    createFlutterFragmentContainer();
    createFlutterFragment(savedInstanceState == null);
  }

  @Override
  public void onPostResume() {
    super.onPostResume();
    flutterFragment.onPostResume();
  }

  @Override
  public void onBackPressed() {
    flutterFragment.onBackPressed();
  }

  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
    flutterFragment.onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

  @Override
  protected void onNewIntent(Intent intent) {
    boolean isRunAction = Intent.ACTION_RUN.equals(intent.getAction());
    if (isDebuggable() && isRunAction) {
      // We're in debug mode, and processing an ACTION_RUN, so we will hot reload Flutter.
      String route = intent.getStringExtra("route");
      String appBundlePath = intent.getDataString();
      if (appBundlePath == null) {
        // Fall back to the installation path if no bundle path was specified.
        appBundlePath = FlutterMain.findAppBundlePath(getApplicationContext());
      }
      flutterFragment.hotReload(route, appBundlePath);
    } else {
      // We're not in debug mode, or we're processing an action other than ACTION_RUN. Therefore,
      // we will not execute a hot reload on Flutter.  But, we will forward this event to the
      // Fragment so that the Intent can be delivered to any interested plugins.
      flutterFragment.onNewIntent(intent);
    }
  }

  @Override
  public void onUserLeaveHint() {
    flutterFragment.onUserLeaveHint();
  }

  /**
   * Sets up a {@code FrameLayout} that takes up all available space in the {@code Activity}. This
   * {@code FrameLayout} will hold the {@code FlutterFragment}.
   */
  private void createFlutterFragmentContainer() {
    FrameLayout container = new FrameLayout(this);
    container.setId(CONTAINER_ID);
    container.setLayoutParams(new ViewGroup.LayoutParams(
        ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.MATCH_PARENT
    ));
    setContentView(container);
  }

  /**
   * If {@code isInitialConfiguration} is true, creates a new {@link FlutterFragment} and places it
   * within this {@code Activity}'s layout. Otherwise, this method simply grabs a reference to the
   * existing {@code FlutterFragment}.
   *
   * @param isInitialConfiguration true if a {@code FlutterFragment} has not yet been created, false otherwise
   */
  private void createFlutterFragment(boolean isInitialConfiguration) {
    if (isInitialConfiguration) {
      flutterFragment = FlutterFragment.newInstance(
          isSplashScreenDesired(),
          getInitialRoute(),
          getAppBundlePath()
      );
      getFragmentManager()
          .beginTransaction()
          .add(CONTAINER_ID, flutterFragment, TAG_FLUTTER_FRAGMENT)
          .commit();
    } else {
      flutterFragment = (FlutterFragment) getFragmentManager().findFragmentByTag(TAG_FLUTTER_FRAGMENT);
    }
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
   * Should the {@code Activity}'s {@code windowBackground} be used as a launch screen
   * until the first frame of Flutter is rendered?
   *
   * This preference is controlled by setting a {@code <meta-data>} tag in the Android
   * manifest for this {@code Activity}.
   */
  private Boolean isSplashScreenDesired() {
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
   * The initial route is set by launching this {@code Activity} with a {@code String} extra called
   * "route".
   *
   * @return initial route to be rendered by Flutter
   */
  private String getInitialRoute() {
    return getIntent().getStringExtra("route");
  }

  /**
   * The path to the bundle that contains this Flutter app's resources, e.g., Dart code snapshots.
   *
   * @return file path to Flutter's app bundle
   */
  private String getAppBundlePath() {
    // If our launching Intent has ACTION_RUN then it indicates that this Activity
    // was launched from tooling. Therefore, our launching Intent may also include
    // a custom app bundle path within its data string.
    if (Intent.ACTION_RUN.equals(getIntent().getAction())) {
      String appBundlePath = getIntent().getDataString();
      if (appBundlePath != null) {
        return appBundlePath;
      }
    }

    // Return the default app bundle path.
    return FlutterMain.findAppBundlePath(getApplicationContext());
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
