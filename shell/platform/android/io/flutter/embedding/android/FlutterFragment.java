// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterShellArgs;
import io.flutter.embedding.engine.renderer.OnFirstFrameRenderedListener;
import io.flutter.plugin.platform.PlatformPlugin;
import io.flutter.view.FlutterMain;
import io.flutter.view.FlutterRunArguments;

import static android.content.ComponentCallbacks2.TRIM_MEMORY_RUNNING_LOW;

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
 *  - {@link Activity#onTrimMemory(int)}
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
 * {@link FlutterView}. Using a {@link FlutterView} requires forwarding some calls from an
 * {@code Activity}, as well as forwarding lifecycle calls from an {@code Activity} or a
 * {@code Fragment}.
 *
 * @see FlutterActivity, which displays a fullscreen Flutter app without any additional work
 * @see FlutterView, which renders the UI for a {@link FlutterEngine}
 */
@SuppressWarnings("WeakerAccess")
@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1)
public class FlutterFragment extends Fragment {
  private static final String TAG = "FlutterFragment";

  private static final String ARG_IS_SPLASH_SCREEN_DESIRED = "show_splash_screen";
  private static final String ARG_INITIAL_ROUTE = "initial_route";
  private static final String ARG_APP_BUNDLE_PATH = "app_bundle_path";
  private static final String ARG_DART_ENTRYPOINT = "dart_entrypoint";
  private static final String ARG_FLUTTER_INITIALIZATION_ARGS = "initialization_args";

  private static final WindowManager.LayoutParams MATCH_PARENT =
      new WindowManager.LayoutParams(WindowManager.LayoutParams.MATCH_PARENT, WindowManager.LayoutParams.MATCH_PARENT);

  /**
   * Factory method that creates a new {@link FlutterFragment} with a default configuration.
   *  - no splash screen
   *  - initial route of "/"
   *  - default app bundle location
   *  - default Dart entrypoint of "main"
   *  - no special engine arguments
   *
   * @return new {@link FlutterFragment}
   */
  public static FlutterFragment newInstance() {
    return newInstance(
        false,
        null,
        null,
        null,
        null
    );
  }

  /**
   * Factory method that creates a new {@link FlutterFragment} with the given configuration.
   *
   * @param isSplashScreenDesired should a splash screen be shown until the 1st Flutter frame is rendered?
   * @param initialRoute the first route that a Flutter app will render in this {@link FlutterFragment}, defaults to "/"
   * @param appBundlePath the path to the app bundle which contains the Dart app to execute
   * @param dartEntrypoint the name of the initial Dart method to invoke, defaults to "main"
   * @param flutterShellArgs any special configuration arguments for the Flutter engine
   *
   * @return a new {@link FlutterFragment}
   */
  public static FlutterFragment newInstance(boolean isSplashScreenDesired,
                                            @Nullable String initialRoute,
                                            @Nullable String appBundlePath,
                                            @Nullable String dartEntrypoint,
                                            @Nullable FlutterShellArgs flutterShellArgs) {
    FlutterFragment frag = new FlutterFragment();

    Bundle args = createArgsBundle(
        isSplashScreenDesired,
        initialRoute,
        appBundlePath,
        dartEntrypoint,
        flutterShellArgs
    );
    frag.setArguments(args);

    return frag;
  }

  /**
   * Creates a {@link Bundle} or arguments that can be used to configure a {@link FlutterFragment}.
   * This method is exposed so that developers can create subclasses of {@link FlutterFragment}.
   * Subclasses should declare static factories that use this method to create arguments that will
   * be understood by the base class, and then the subclass can add any additional arguments it
   * wants to this {@link Bundle}. Example:
   *
   * <pre>{@code
   * public static MyFlutterFragment newInstance(String myNewArg) {
   *   // Create an instance of our subclass Fragment.
   *   MyFlutterFragment myFrag = new MyFlutterFragment();
   *
   *   // Create the Bundle or args that FlutterFragment understands.
   *   Bundle args = FlutterFragment.createArgsBundle(...);
   *
   *   // Add our new args to the bundle.
   *   args.putString(ARG_MY_NEW_ARG, myNewArg);
   *
   *   // Give the args to our subclass Fragment.
   *   myFrag.setArguments(args);
   *
   *   // Return the newly created subclass Fragment.
   *   return myFrag;
   * }
   * }</pre>
   *
   * @param isSplashScreenDesired should a splash screen be shown until the 1st Flutter frame is rendered?
   * @param initialRoute the first route that a Flutter app will render in this {@link FlutterFragment}, defaults to "/"
   * @param appBundlePath the path to the app bundle which contains the Dart app to execute
   * @param dartEntrypoint the name of the initial Dart method to invoke, defaults to "main"
   * @param flutterShellArgs any special configuration arguments for the Flutter engine
   *
   * @return Bundle of arguments that configure a {@link FlutterFragment}
   */
  public static Bundle createArgsBundle(boolean isSplashScreenDesired,
                                        @Nullable String initialRoute,
                                        @Nullable String appBundlePath,
                                        @Nullable String dartEntrypoint,
                                        @Nullable FlutterShellArgs flutterShellArgs) {
    Bundle args = new Bundle();
    args.putBoolean(ARG_IS_SPLASH_SCREEN_DESIRED, isSplashScreenDesired);
    args.putString(ARG_INITIAL_ROUTE, initialRoute);
    args.putString(ARG_APP_BUNDLE_PATH, appBundlePath);
    args.putString(ARG_DART_ENTRYPOINT, dartEntrypoint);
    if (null != flutterShellArgs) {
      args.putStringArray(ARG_FLUTTER_INITIALIZATION_ARGS, flutterShellArgs.toArray());
    }
    return args;
  }

  private FlutterEngine flutterEngine;
  private FrameLayout container;
  private FlutterView flutterView;
  private View launchView;

  private PlatformPlugin platformPlugin;

  public FlutterFragment() {
    // Ensure that we at least have an empty Bundle of arguments so that we don't
    // need to continually check for null arguments before grabbing one.
    setArguments(new Bundle());
  }

  @Override
  public void onAttach(Activity activity) {
    super.onAttach(activity);

    // TODO(mattcarroll): I think the build system is linking the wrong Fragment API. It says that
    //                    getContext() cannot be found...
    initializeFlutter(activity);

    // When "retain instance" is true, the FlutterEngine will survive configuration
    // changes. Therefore, we create a new one only if one does not already exist.
    if (flutterEngine == null) {
      createFlutterEngine();
    }

    // Regardless of whether or not a FlutterEngine already existed, the PlatformPlugin
    // is bound to a specific Activity. Therefore, it needs to be created and configured
    // every time this Fragment attaches to a new Activity.
    connectFlutterEngineToAndroidPlatform();
  }

  // TODO(mattcarroll): does it really make sense to run this for every Fragment? Note: the
  //                    implementation of the "ensure" method includes a reference to the default
  //                    app bundle path. How does that jive with "doInitialFlutterViewRun"?
  private void initializeFlutter(@NonNull Context context) {
    String[] flutterShellArgsArray = getArguments().getStringArray(ARG_FLUTTER_INITIALIZATION_ARGS);
    FlutterShellArgs flutterShellArgs = new FlutterShellArgs(
        flutterShellArgsArray != null ? flutterShellArgsArray : new String[] {}
    );

    FlutterMain.ensureInitializationComplete(context.getApplicationContext(), flutterShellArgs);
  }

  /**
   * Creates a new FlutterEngine instance.
   *
   * Subclasses can instantiate their own {@link FlutterEngine} by overriding
   * {@link #onCreateFlutterEngine(Context)}.
   *
   * Subclasses can alter the {@link FlutterEngine} after creation by overriding
   * {@link #onFlutterEngineCreated(FlutterEngine)}.
   */
  private void createFlutterEngine() {
    // Create a FlutterEngine to back our FlutterView.
    flutterEngine = onCreateFlutterEngine(getActivity());

    // Allow subclasses to customize FlutterEngine as desired.
    onFlutterEngineCreated(flutterEngine);
  }

  /**
   * Hook for subclasses to customize the creation of the {@code FlutterEngine}.
   *
   * By default, this method returns a standard {@link FlutterEngine} without any modification.
   */
  @NonNull
  protected FlutterEngine onCreateFlutterEngine(@NonNull Context context) {
    Log.d(TAG, "onCreateFlutterEngine()");
    return new FlutterEngine(context, getResources());
  }

  /**
   * Hook for subclasses to customize the {@link FlutterEngine} owned by this {@link FlutterFragment}
   * after the {@link FlutterEngine} has been instantiated.
   */
  protected void onFlutterEngineCreated(@NonNull FlutterEngine flutterEngine) {
    // no-op
  }

  /**
   * Creates a {@link PlatformPlugin} that receives requests from the app running in the
   * {@link FlutterEngine} and carries out the necessary Android behavior.
   *
   * The created plugin is connected to the {@link FlutterEngine} via its
   * {@link io.flutter.embedding.engine.systemchannels.PlatformChannel}.
   *
   * Some examples of behavior carried about by the platform plugin include:
   *  - clipboard
   *  - haptic feedback
   *  - playing system sounds
   *  - etc.
   */
  private void connectFlutterEngineToAndroidPlatform() {
    platformPlugin = new PlatformPlugin(getActivity());
    flutterEngine.getSystemChannels().platform.setMethodCallHandler(platformPlugin);
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup parent, Bundle savedInstanceState) {
    Log.e(TAG, "onCreateView()");
    View layout = createLayout();

    flutterEngine.getPluginRegistry().attach(flutterView, getActivity());
    flutterView.attachToFlutterEngine(flutterEngine);
    doInitialFlutterViewRun();

    return layout;
  }

  /**
   * Creates a {@code FrameLayout} that takes all available space, then creates a {@code FlutterView}
   * within the FrameLayout container.
   */
  private View createLayout() {
    // Create our FlutterView for rendering and user interaction.
    flutterView = createFlutterView(getActivity());

    // Allow subclasses to customize FlutterView as desired.
    onFlutterViewCreated(flutterView);

    // Add FlutterView to the View hierarchy.
    container = new FrameLayout(getContextCompat());
    container.setLayoutParams(MATCH_PARENT);
    container.addView(flutterView, MATCH_PARENT);

    // Create and add a launch view, if desired.
    launchView = createLaunchView();
    if (launchView != null) {
      Log.d(TAG, "Showing launch view.");
      container.addView(launchView, MATCH_PARENT);
    }

    return container;
  }

  /**
   * Hook for subclasses to customize the creation of the {@code FlutterView}.
   */
  @NonNull
  protected FlutterView createFlutterView(@NonNull Context context) {
    Log.d(TAG, "createFlutterView()");
    return new FlutterView(context, null);
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
   *
   * Subclasses may override this method to return a launch view of choice.
   */
  @SuppressLint("NewApi")
  @Nullable
  protected View createLaunchView() {
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

    // TODO(mattcarroll): move the launch screen support into FlutterView
    flutterEngine.getRenderer().addOnFirstFrameRenderedListener(new OnFirstFrameRenderedListener() {
      @Override
      public void onFirstFrameRendered() {
        Log.d(TAG, "onFirstFrameRendered(). Animating out the launch view");
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

        flutterEngine.getRenderer().removeOnFirstFrameRenderedListener(this);
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

  @Override
  public void onStart() {
    super.onStart();
    Log.d(TAG, "onStart()");
    flutterEngine.getSystemChannels().lifecycle.appIsInactive();
  }

  public void onPostResume() {
    Log.d(TAG, "onPostResume()");
    // TODO(mattcarroll): what does 'updateAccessibilityFeatures' do? why is it called here?
    flutterView.updateAccessibilityFeatures();
    // TODO(mattcarroll): why does the platform plugin care about "post resume"?
    platformPlugin.onPostResume();
    // TODO(mattcarroll): why does the lifecycle wait for "post resume" to say we're resumed?
    flutterEngine.getSystemChannels().lifecycle.appIsResumed();
  }

  @Override
  public void onPause() {
    super.onPause();
    Log.d(TAG, "onPause()");
    flutterEngine.getSystemChannels().lifecycle.appIsInactive();
  }

  @Override
  public void onStop() {
    super.onStop();
    Log.d(TAG, "onStop()");
    flutterEngine.getSystemChannels().lifecycle.appIsPaused();
  }

  @Override
  public void onDestroyView() {
    super.onDestroyView();
    Log.d(TAG, "onDestroyView()");
    flutterView.detachFromFlutterEngine();

    // TODO(mattcarroll): what does 'detach' refer to?  The Activity? The UI? JNI?....
    flutterEngine.getPluginRegistry().detach();
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    Log.d(TAG, "onDestroy()");

    // TODO(mattcarroll): re-evaluate how Flutter plugins interact with FlutterView and FlutterNativeView
    final boolean detach = flutterEngine.getPluginRegistry().onViewDestroy(flutterEngine);
    if (detach || retainFlutterIsolateAfterFragmentDestruction()) {
      // Detach, but do not destroy the FlutterView if a plugin expressed interest in its
      // FlutterNativeView.
      flutterEngine.detachFromJni();
    } else {
      flutterEngine.destroy();
    }
  }

  @Override
  public void onDetach() {
    super.onDetach();
    Log.d(TAG, "onDetach: Detaching platform channel.");
    flutterEngine.getSystemChannels().platform.setMethodCallHandler(null);
    platformPlugin = null;
  }

  /**
   * The hardware back button was pressed.
   *
   * See {@link Activity#onBackPressed()}
   */
  public void onBackPressed() {
    Log.d(TAG, "onBackPressed()");
    flutterEngine.getSystemChannels().navigation.popRoute();
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
    flutterEngine.getPluginRegistry().onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

  /**
   * A new Intent was received by the {@link Activity} that currently owns this {@link Fragment}.
   *
   * See {@link Activity#onNewIntent(Intent)}
   *
   * @param intent new Intent
   */
  public void onNewIntent(@NonNull Intent intent) {
    flutterEngine.getPluginRegistry().onNewIntent(intent);
  }

  /**
   * A result has been returned after an invocation of {@link Fragment#startActivityForResult(Intent, int)}.
   *
   * @param requestCode request code sent with {@link Fragment#startActivityForResult(Intent, int)}
   * @param resultCode code representing the result of the {@code Activity} that was launched
   * @param data any corresponding return data, held within an {@code Intent}
   */
  @Override
  public void onActivityResult(int requestCode, int resultCode, Intent data) {
    flutterEngine.getPluginRegistry().onActivityResult(requestCode, resultCode, data);
  }

  /**
   * The {@link Activity} that owns this {@link Fragment} is about to go to the background
   * as the result of a user's choice/action, i.e., not as the result of an OS decision.
   *
   * See {@link Activity#onUserLeaveHint()}
   */
  public void onUserLeaveHint() {
    flutterEngine.getPluginRegistry().onUserLeaveHint();
  }

  /**
   * Callback invoked when memory is low.
   *
   * This implementation forwards a memory pressure warning to the running Flutter app.
   *
   * @param level level
   */
  public void onTrimMemory(int level) {
    // Use a trim level delivered while the application is running so the
    // framework has a chance to react to the notification.
    if (level == TRIM_MEMORY_RUNNING_LOW) {
      flutterEngine.getSystemChannels().system.sendMemoryPressureWarning();
    }
  }

  /**
   * Callback invoked when memory is low.
   *
   * This implementation forwards a memory pressure warning to the running Flutter app.
   */
  @Override
  public void onLowMemory() {
    super.onLowMemory();
    flutterEngine.getSystemChannels().system.sendMemoryPressureWarning();
  }

  /**
   * The {@link FlutterEngine} that backs the Flutter content presented by this {@code Fragment}.
   *
   * @return the {@link FlutterEngine} held by this {@code Fragment}
   */
  @Nullable
  public FlutterEngine getFlutterEngine() {
    return flutterEngine;
  }

  /**
   * Enables transparency on this {@link FlutterFragment}'s {@link FlutterView}. If part of the
   * Flutter app does not paint a background, the {@link FlutterView} will show through to whatever
   * is behind this {@link FlutterFragment}.
   *
   * @see FlutterFragment#disableTransparentBackground()
   */
  public void enableTransparentBackground() {
    flutterView.enableTransparentBackground();
  }

  /**
   * Disables transparency on this {@link FlutterFragment}'s {@link FlutterView}. As a result, the
   * entire {@link FlutterView} will be painted with some color.
   *
   * @see FlutterFragment#enableTransparentBackground()
   */
  public void disableTransparentBackground() {
    flutterView.disableTransparentBackground();
  }

  private boolean isSplashScreenDesired() {
    return getArguments().getBoolean(ARG_IS_SPLASH_SCREEN_DESIRED, false);
  }

  @Nullable
  private String getInitialRoute() {
    return getArguments().getString(ARG_INITIAL_ROUTE);
  }

  @NonNull
  private String getAppBundlePath() {
    return getArguments().getString(ARG_APP_BUNDLE_PATH, FlutterMain.findAppBundlePath(getContextCompat()));
  }

  @NonNull
  private String getDartEntrypoint() {
    return getArguments().getString(ARG_DART_ENTRYPOINT, "main");
  }

  /**
   * Should the Flutter isolate that is connected to this {@code FlutterFragment}
   * be retained after this {@code FlutterFragment} is destroyed?
   *
   * Defaults to false. This method can be overridden in subclasses to retain the
   * Flutter isolate.
   *
   * Isolates in Dart/Flutter are similar to processes in other languages. Any data
   * held within the Flutter isolate within this {@code FlutterFragment} will be lost
   * if the isolate is destroyed. This loss of data is fine for most UI related Flutter
   * behavior. However, if this {@code FlutterFragment}'s isolate contains application
   * data that is required outside of this {@code FlutterFragment} then consider retaining
   * the isolate for later use, or consider a data marshalling strategy to move long-lived
   * data out of this isolate.
   *
   * See https://docs.flutter.io/flutter/dart-isolate/Isolate-class.html
   *
   * @return true if this FlutterFragment's Flutter isolate should be retained after destruction, false otherwise
   */
  // TODO(mattcarroll): how exactly does this retain the engine? is there a static reference somewhere?
  protected boolean retainFlutterIsolateAfterFragmentDestruction() {
    return false;
  }

  /**
   * Starts running Dart within the FlutterView for the first time.
   *
   * Reloading/restarting Dart within a given FlutterView is not supported.
   */
  private void doInitialFlutterViewRun() {
//    if (BuildConfig.DEBUG && flutterView.getFlutterNativeView().isApplicationRunning()) {
    if (flutterEngine.getDartExecutor().isApplicationRunning()) {
      return;
//      throw new RuntimeException("Tried to initialize Dart execution in Flutter engine that is already running.");
    }

    if (getInitialRoute() != null) {
      flutterEngine.getSystemChannels().navigation.setInitialRoute(getInitialRoute());
    }

    // TODO(mattcarroll): are FlutterRunArguments and FlutterShellArgs the same thing? consolidate if they are
    FlutterRunArguments args = new FlutterRunArguments();
    args.bundlePath = getAppBundlePath();
    args.entrypoint = getDartEntrypoint();
    args.defaultPath = null;
    flutterEngine.getDartExecutor().runFromBundle(args);

    // TODO(mattcarroll): why do we need to resetAccessibilityTree in this method? Can we call that from within FlutterView somewhere?
    flutterView.resetAccessibilityTree();
  }

  @NonNull
  private Context getContextCompat() {
    return getActivity();
    // TODO(mattcarroll): bring back when target SDK is rev'd
//    return Build.VERSION.SDK_INT >= 23
//      ? getContext()
//      : getActivity();
  }
}
