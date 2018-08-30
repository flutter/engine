// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Log;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;
import io.flutter.app.FlutterActivity;
import io.flutter.plugin.common.PluginRegistry;
import io.flutter.view.FlutterNativeView;
import io.flutter.view.FlutterView;

/**
 * {@code Fragment} which displays a {@link FlutterView} that takes up all available space.
 *
 * Using a {@code FlutterFragment} requires forwarding a number of calls from an {@code Activity} to
 * ensure that the internal Flutter app behaves as expected:
 *  - {@link Activity#onPostResume()}
 *  - {@link Activity#onBackPressed()}
 *  - {@link Activity#onRequestPermissionsResult(int, String[], int[])} ()}
 *  - {@link Activity#onNewIntent(Intent)} ()}
 *  - {@link Activity#onUserLeaveHint()}
 *
 * Additionally, when starting an {@code Activity} for a result from this {@code Fragment}, be sure
 * to invoke {@link Fragment#startActivityForResult(Intent, int)} rather than
 * {@link Activity#startActivityForResult(Intent, int)}. If the {@code Activity} version of the
 * method is invoked then this {@code Fragment} will never receive its
 * {@link Fragment#onActivityResult(int, int, Intent)} callback.
 *
 * If convenient, consider using a {@link FlutterActivity} instead of a {@code FlutterFragment} to
 * avoid the work of forwarding calls.
 *
 * If Flutter is needed in a location that can only use a {@code View}, consider using a
 * {@link io.flutter.view.FlutterView}. Using a {@link io.flutter.view.FlutterView} requires
 * forwarding some calls from an {@code Activity}, as well as forwarding lifecycle calls from
 * an {@code Activity} or a {@code Fragment}.
 *
 * @see FlutterActivity
 *
 * @see io.flutter.view.FlutterView
 */
@SuppressWarnings("WeakerAccess")
@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1)
public class FlutterFragment extends Fragment implements PluginRegistry {
  private static final String TAG = "FlutterFragment";

  private static final String ARG_IS_SPLASH_SCREEN_DESIRED = "show_splash_screen";
  private static final String ARG_INITIAL_ROUTE = "initial_route";
  private static final String ARG_APP_BUNDLE_PATH = "app_bundle_path";

  private static final WindowManager.LayoutParams MATCH_PARENT =
      new WindowManager.LayoutParams(WindowManager.LayoutParams.MATCH_PARENT, WindowManager.LayoutParams.MATCH_PARENT);

  public static FlutterFragment newInstance(boolean isSplashScreenDesired,
                                            @Nullable String initialRoute,
                                            @NonNull String appBundlePath) {
    FlutterFragment frag = new FlutterFragment();

    Bundle args = new Bundle();
    args.putBoolean(ARG_IS_SPLASH_SCREEN_DESIRED, isSplashScreenDesired);
    args.putString(ARG_INITIAL_ROUTE, initialRoute);
    args.putString(ARG_APP_BUNDLE_PATH, appBundlePath);
    frag.setArguments(args);

    return frag;
  }

  private FrameLayout container;
  private FlutterView flutterView;
  private View launchView;

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup parent, Bundle savedInstanceState) {
    createLayout();
    // TODO: Should we start running the FlutterView here or when attached? It was being done here
    //       in the Activity, but maybe that was a problem to begin with?
    doInitialFlutterViewRun();

    return container;
  }

  @Override
  public void onStart() {
    super.onStart();
    flutterView.onStart();
  }

  @Override
  public void onResume() {
    super.onResume();
    // TODO: should flutterView have an onResume() method?
  }

  public void onPostResume() {
    flutterView.onPostResume();
  }

  @Override
  public void onPause() {
    super.onPause();
    flutterView.onPause();
  }

  @Override
  public void onStop() {
    super.onStop();
    flutterView.onStop();
  }

  @Override
  public void onDestroy() {
    super.onDestroy();

    // TODO(mattcarroll): re-evaluate how Flutter plugins interact with FlutterView and FlutterNativeView
    final boolean detach = flutterView.getPluginRegistry().onViewDestroy(
      flutterView.getFlutterNativeView()
    );
    if (detach || retainFlutterNativeView()) {
      // Detach, but do not destroy the FlutterView if a plugin expressed interest in its
      // FlutterNativeView.
      flutterView.detach();
    } else {
      flutterView.destroy();
    }
  }

  /**
   * The hardware back button was pressed.
   *
   * See {@link Activity#onBackPressed()}
   */
  public void onBackPressed() {
    flutterView.popRoute();
  }

  /**
   * The result of a permission request has been received.
   *
   * See {@link Activity#onRequestPermissionsResult(int, String[], int[])}
   *
   * @param requestCode identifier passed with the initial permission request
   * @param permissions permissions that were requested
   * @param grantResults permission grants or denials
   */
  public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
    flutterView.getPluginRegistry().onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

  /**
   * A new Intent was received by the {@link Activity} that currently owns this {@link Fragment}.
   *
   * See {@link Activity#onNewIntent(Intent)}
   *
   * @param intent new Intent
   */
  public void onNewIntent(@NonNull Intent intent) {
    flutterView.getPluginRegistry().onNewIntent(intent);
  }

  @Override
  public void onActivityResult(int requestCode, int resultCode, Intent data) {
    flutterView.getPluginRegistry().onActivityResult(requestCode, resultCode, data);
  }

  /**
   * The {@link Activity} that owns this {@link Fragment} is about to go to the background
   * as the result of a user's choice/action, i.e., not as the result of an OS decision.
   *
   * See {@link Activity#onUserLeaveHint()}
   */
  public void onUserLeaveHint() {
    flutterView.getPluginRegistry().onUserLeaveHint();
  }

  @Override
  public void onTrimMemory(int level) {
    super.onTrimMemory(level);

    // Use a trim level delivered while the application is running so the
    // framework has a chance to react to the notification.
    if (level == TRIM_MEMORY_RUNNING_LOW) {
      flutterView.onMemoryPressure();
    }
  }

  @Override
  public void onLowMemory() {
    super.onLowMemory();
    flutterView.onMemoryPressure();
  }

  /**
   * Creates a {@code FrameLayout} that takes all available space, then creates a {@code FlutterView}
   * within the FrameLayout container.
   */
  private void createLayout() {
    container = new FrameLayout(getContextCompat());
    container.setLayoutParams(MATCH_PARENT);

    flutterView = createFlutterView(getActivity());
    onFlutterViewCreated(flutterView);
    container.addView(flutterView, MATCH_PARENT);

    launchView = createLaunchView();
    if (launchView != null) {
      container.addView(launchView, MATCH_PARENT);
    }
  }

  /**
   * Hook for subclasses to customize the creation of the {@code FlutterView}.
   *
   * TODO: get rid of dependency on Activity
   */
  @NonNull
  protected FlutterView createFlutterView(@NonNull Activity activity) {
    FlutterNativeView nativeView = createFlutterNativeView(activity);
    return new FlutterView(activity, null, nativeView);
  }

  /**
   * Hook for subclasses to customize the creation of the {@code FlutterNativeView}.
   *
   * This method is only invoked from the default implementation of {@link #createFlutterView(Activity)}.
   * If {@link #createFlutterView(Activity)} is overridden, then this method will not be invoked unless
   * it is invoked directly from the subclass.
   *
   * By default, this method returns a standard {@link FlutterNativeView} without any modification.
   */
  @NonNull
  protected FlutterNativeView createFlutterNativeView(@NonNull Context context) {
    return new FlutterNativeView(context);
  }

  /**
   * Hook for subclasses to customize aspects of the {@code FlutterView} that are not creation
   * dependent, e.g., {@code FlutterView}'s initial route.
   */
  protected void onFlutterViewCreated(@SuppressWarnings("unused") @NonNull FlutterView flutterView) {
    // no-op
  }

  /**
   * Creates a {@link View} containing the same {@link Drawable} as the one set as the
   * {@code windowBackground} of the parent activity for use as a launch splash view.
   *
   * Shows and then automatically animates out the launch view when Flutter renders its first frame.
   *
   * {@code FlutterView} must exist before this method is called.
   *
   * Returns null if no {@code windowBackground} is set for the activity.
   */
  @SuppressLint("NewApi")
  @Nullable
  private View createLaunchView() {
    if (!isSplashScreenDesired()) {
      return null;
    }

    final Drawable launchScreenDrawable = getSplashScreenDrawableFromActivityTheme();
    if (launchScreenDrawable == null) {
      return null;
    }

    final View view = new View(getContextCompat());
    view.setLayoutParams(MATCH_PARENT);
    // TODO(mattcarroll): this API version error exists in the original code. Why is it there?
    view.setBackground(launchScreenDrawable);

    flutterView.addFirstFrameListener(new FlutterView.FirstFrameListener() {
      @Override
      public void onFirstFrame() {
        launchView.animate()
            .alpha(0f)
            // Use Android's default animation duration.
            .setListener(new AnimatorListenerAdapter() {
              @Override
              public void onAnimationEnd(Animator animation) {
                // Views added to an Activity's addContentView is always added to its
                // root FrameLayout.
                ((ViewGroup) launchView.getParent()).removeView(launchView);
                launchView = null;
              }
            });

        flutterView.removeFirstFrameListener(this);
      }
    });

    // Resets the activity theme from the one containing the launch screen in the window
    // background to a blank one since the launch screen is now in a view in front of the
    // FlutterView.
    //
    // We can make this configurable if users want it.
    // TODO: get rid of Activity references.
    getActivity().setTheme(android.R.style.Theme_Black_NoTitleBar);

    return view;
  }

  private boolean isSplashScreenDesired() {
    return getArguments().getBoolean(ARG_IS_SPLASH_SCREEN_DESIRED, false);
  }

  /**
   * Extracts a {@link Drawable} from the parent activity's {@code windowBackground}.
   *
   * {@code android:windowBackground} is specifically reused instead of other attributes
   * because the Android framework can display it fast enough when launching the app as opposed
   * to anything defined in the Activity subclass.
   *
   * TODO(mattcarroll): speed aside, should developers be given the opportunity to use different Drawables?
   *
   * Returns null if no {@code windowBackground} is set for the activity.
   */
  @SuppressWarnings("deprecation")
  @Nullable
  private Drawable getSplashScreenDrawableFromActivityTheme() {
    TypedValue typedValue = new TypedValue();
    if (!getContextCompat().getTheme().resolveAttribute(
        android.R.attr.windowBackground,
        typedValue,
        true)) {
      return null;
    }
    if (typedValue.resourceId == 0) {
      return null;
    }
    try {
      return getResources().getDrawable(typedValue.resourceId);
    } catch (Resources.NotFoundException e) {
      Log.e(TAG, "Referenced launch screen windowBackground resource does not exist");
      return null;
    }
  }

  /**
   * Returns the Flutter view used by this {@code Fragment}; will be null before
   * {@link #onCreateView(LayoutInflater, ViewGroup, Bundle)} is invoked. Will be
   * non-null after {@link #onCreateView(LayoutInflater, ViewGroup, Bundle)} is invoked, up
   * until {@link #onDestroyView()} is invoked.
   */
  @SuppressWarnings("unused")
  @Nullable
  public FlutterView getFlutterView() {
    return flutterView;
  }

  /**
   * Starts running Dart within the FlutterView for the first time.
   *
   * The difference between the first run and subsequent reloads is that when FlutterView is
   * run for the first time, it attempts to reuse an existing Dart isolate.
   */
  private void doInitialFlutterViewRun() {
    runFlutterView(getInitialRoute(), getAppBundlePath(), true);
  }

  /**
   * Reloads Dart code with that at the given {@code appBundlePath}, sets Flutter's initial route
   * to {@code initialRoute}, and then reloads the Dart VM.
   *
   * TODO(mattcarroll): is the above description accurate?
   *
   * @param initialRoute initial Flutter route to display after reloading Dart code
   * @param appBundlePath Android file path to Flutter's app bundle
   */
  public void hotReload(@Nullable String initialRoute, @NonNull String appBundlePath) {
    runFlutterView(initialRoute, appBundlePath, false);
  }

  /**
   * Instructs the FlutterView to start running with the given {@code appBundlePath}, and to start
   * Flutter at the given {@code initialRoute}.
   *
   * @param initialRoute initial Flutter route to display after reloading Dart code
   * @param appBundlePath Android file path to Flutter's app bundle
   * @param reuseIsolate whether or not to reuse an existing Dart isolate
   */
  private void runFlutterView(@Nullable String initialRoute, @NonNull String appBundlePath, boolean reuseIsolate) {
    if (initialRoute != null) {
      flutterView.setInitialRoute(initialRoute);
    }
    // TODO: what does it really mean for this condition to be false? is that ok? should we log?
    if (!flutterView.getFlutterNativeView().isApplicationRunning()) {
      flutterView.runFromBundle(appBundlePath, null, "main", reuseIsolate);
    }
  }

  @Nullable
  private String getInitialRoute() {
    return getArguments().getString(ARG_INITIAL_ROUTE);
  }

  @NonNull
  private String getAppBundlePath() {
    // Guaranteed non-null by Fragment factory method which declares the incoming
    // parameter @NonNull.
    //noinspection ConstantConditions
    return getArguments().getString(ARG_APP_BUNDLE_PATH);
  }

  @Override
  public final boolean hasPlugin(@NonNull String key) {
    return flutterView.getPluginRegistry().hasPlugin(key);
  }

  @Override
  @Nullable
  @SuppressWarnings("unchecked")
  public <T> T valuePublishedByPlugin(@NonNull String pluginKey) {
    return (T) flutterView.getPluginRegistry().valuePublishedByPlugin(pluginKey);
  }

  @Override
  @NonNull
  public PluginRegistry.Registrar registrarFor(@NonNull String pluginKey) {
    return flutterView.getPluginRegistry().registrarFor(pluginKey);
  }

  // TODO: what is the significance of this method? should it be a subclass hook?
  private boolean retainFlutterNativeView() {
    return false;
  }

  @NonNull
  private Context getContextCompat() {
    return Build.VERSION.SDK_INT >= 23
      ? getContext()
      : getActivity();
  }
}
