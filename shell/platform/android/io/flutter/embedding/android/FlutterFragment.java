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

import java.util.HashMap;
import java.util.Map;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterShellArgs;
import io.flutter.embedding.engine.renderer.OnFirstFrameRenderedListener;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.JSONMessageCodec;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StringCodec;
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

  private MethodChannel flutterPlatformChannel;
  private PlatformPlugin platformPlugin;
  private BasicMessageChannel<String> mFlutterLifecycleChannel;
  private BasicMessageChannel<Object> mFlutterSystemChannel;
  private MethodChannel mFlutterNavigationChannel;

  public FlutterFragment() {
    // Ensure that we at least have an empty Bundle of arguments so that we don't
    // need to continually check for null arguments before grabbing one.
    setArguments(new Bundle());
  }

  @Override
  public void onAttach(Activity activity) {
    super.onAttach(activity);

    platformPlugin = new PlatformPlugin(activity);
    if (flutterEngine != null) {
      flutterPlatformChannel = new MethodChannel(flutterEngine.getDartExecutor(), "flutter/platform", JSONMethodCodec.INSTANCE);
      flutterPlatformChannel.setMethodCallHandler(platformPlugin);
    }
  }

  @Override
  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // TODO(mattcarroll): I think the build system is linking the wrong Fragment API. It says that
    //                    getContext() cannot be found...
    initializeFlutter(getActivity());

    ensureFlutterEngineCreated();
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
   * Creates a new FlutterEngine instance if one does not already exist.
   *
   * In addition to creating a new FlutterEngine, this method creates a PlatformPlugin that is
   * registered with the
   */
  private void ensureFlutterEngineCreated() {
    // Create a FlutterEngine to back our FlutterView. It may already exist if this Fragment
    // has been set to "retain instance".
    if (flutterEngine == null) {
      flutterEngine = createFlutterEngine(getActivity());

      // Allow subclasses to customize FlutterEngine as desired.
      onFlutterEngineCreated(flutterEngine);

      // Create and register desired platform channels to communicate between Dart/Java.
      registerPlaformChannels();

      flutterPlatformChannel = new MethodChannel(flutterEngine.getDartExecutor(), "flutter/platform", JSONMethodCodec.INSTANCE);
      flutterPlatformChannel.setMethodCallHandler(platformPlugin);
    }
  }

  // TODO(mattcarroll): create infrastructure to take in channels. Right now the use of local variables
  //                    prevents this method from being configured.
  protected void registerPlaformChannels() {
    mFlutterLifecycleChannel = new BasicMessageChannel<>(flutterEngine.getDartExecutor(), "flutter/lifecycle", StringCodec.INSTANCE);
    mFlutterSystemChannel = new BasicMessageChannel<>(flutterEngine.getDartExecutor(), "flutter/system", JSONMessageCodec.INSTANCE);
    mFlutterNavigationChannel = new MethodChannel(flutterEngine.getDartExecutor(), "flutter/navigation", JSONMethodCodec.INSTANCE);
  }

  /**
   * Hook for subclasses to customize the creation of the {@code FlutterEngine}.
   *
   * This method is only invoked from the default implementation of {@link #createFlutterView(Context)}.
   * If {@link #createFlutterView(Context)} is overridden, then this method will not be invoked unless
   * it is invoked directly from the subclass.
   *
   * By default, this method returns a standard {@link FlutterEngine} without any modification.
   */
  @NonNull
  protected FlutterEngine createFlutterEngine(@NonNull Context context) {
    Log.d(TAG, "createFlutterEngine()");
    return new FlutterEngine(context, getResources(), false);
  }

  /**
   * Hook for subclasses to customize the {@link FlutterEngine} owned by this {@link FlutterFragment}
   * after the {@link FlutterEngine} has been instantiated.
   */
  protected void onFlutterEngineCreated(@NonNull FlutterEngine flutterEngine) {
    // no-op
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup parent, Bundle savedInstanceState) {
    Log.e(TAG, "onCreateView()");
    createLayout();

    Log.d(TAG, "onCreateView: Attaching plugin registry to Activity");
    flutterEngine.getPluginRegistry().attach(flutterView, getActivity());

    // TODO: Should we start running the FlutterView here or when attached to window? It was being done here
    //       in the Activity, but maybe that was a problem to begin with?
    flutterView.attachToFlutterRenderer(flutterEngine.getRenderer(), flutterEngine.getDartExecutor());
    doInitialFlutterViewRun();

    return container;
  }

  /**
   * Creates a {@code FrameLayout} that takes all available space, then creates a {@code FlutterView}
   * within the FrameLayout container.
   */
  private void createLayout() {
    container = new FrameLayout(getContextCompat());
    container.setLayoutParams(MATCH_PARENT);

    // Create our FlutterView for rendering and user interaction.
    flutterView = createFlutterView(getActivity());

    // Allow subclasses to customize FlutterView as desired.
    onFlutterViewCreated(flutterView);

    // Add FlutterView to the View hierarchy.
    container.addView(flutterView, MATCH_PARENT);

    launchView = createLaunchView();
    if (launchView != null) {
      Log.d(TAG, "Showing launch view.");
      container.addView(launchView, MATCH_PARENT);
    }
  }

  @Override
  public void onStart() {
    super.onStart();
    Log.d(TAG, "onStart()");
    mFlutterLifecycleChannel.send("AppLifecycleState.inactive");
  }

  public void onPostResume() {
    Log.d(TAG, "onPostResume()");
    // TODO(mattcarroll): what does 'updateAccessibilityFeatures' do? why is it called here?
    flutterView.updateAccessibilityFeatures();
    // TODO(mattcarroll): why does the platform plugin care about "post resume"?
    platformPlugin.onPostResume();
    // TODO(mattcarroll): why does the lifecycle wait for "post resume" to say we're resumed?
    mFlutterLifecycleChannel.send("AppLifecycleState.resumed");
  }

  @Override
  public void onPause() {
    super.onPause();
    Log.d(TAG, "onPause()");
    mFlutterLifecycleChannel.send("AppLifecycleState.inactive");
  }

  @Override
  public void onStop() {
    super.onStop();
    Log.d(TAG, "onStop()");
    mFlutterLifecycleChannel.send("AppLifecycleState.paused");
  }

  @Override
  public void onDestroyView() {
    super.onDestroyView();
    Log.d(TAG, "onDestroyView()");
    flutterView.detachFromFlutterRenderer();

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
    platformPlugin = null;
    flutterPlatformChannel.setMethodCallHandler(null);
    flutterPlatformChannel = null;
  }

  /**
   * The hardware back button was pressed.
   *
   * See {@link Activity#onBackPressed()}
   */
  public void onBackPressed() {
    Log.d(TAG, "onBackPressed()");
    popRoute();
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

  public void onTrimMemory(int level) {
    // Use a trim level delivered while the application is running so the
    // framework has a chance to react to the notification.
    if (level == TRIM_MEMORY_RUNNING_LOW) {
      sendMemoryPressureWarningToFlutter();
    }
  }

  @Override
  public void onLowMemory() {
    super.onLowMemory();
    sendMemoryPressureWarningToFlutter();
  }

  private void sendMemoryPressureWarningToFlutter() {
    Map<String, Object> message = new HashMap<>(1);
    message.put("type", "memoryPressure");
    mFlutterSystemChannel.send(message);
  }

  public void enableTransparentBackground() {
    flutterView.enableTransparentBackground();
  }

  public void disableTransparentBackground() {
    flutterView.disableTransparentBackground();
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
   * The {@link FlutterEngine} that backs the Flutter content presented by this {@code Fragment}.
   *
   * @return the {@link FlutterEngine} held by this {@code Fragment}
   */
  @Nullable
  protected FlutterEngine getFlutterEngine() {
    return flutterEngine;
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
      setInitialRoute(getInitialRoute());
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

  private void setInitialRoute(String route) {
    mFlutterNavigationChannel.invokeMethod("setInitialRoute", route);
  }

  private void pushRoute(String route) {
    mFlutterNavigationChannel.invokeMethod("pushRoute", route);
  }

  private void popRoute() {
    mFlutterNavigationChannel.invokeMethod("popRoute", null);
  }
}
