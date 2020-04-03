// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.loader.FlutterLoader;
import io.flutter.embedding.engine.plugins.PluginRegistry;
import io.flutter.embedding.engine.plugins.activity.ActivityControlSurface;
import io.flutter.embedding.engine.plugins.broadcastreceiver.BroadcastReceiverControlSurface;
import io.flutter.embedding.engine.plugins.contentprovider.ContentProviderControlSurface;
import io.flutter.embedding.engine.plugins.service.ServiceControlSurface;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.engine.renderer.RenderSurface;
import io.flutter.embedding.engine.systemchannels.AccessibilityChannel;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel;
import io.flutter.embedding.engine.systemchannels.LifecycleChannel;
import io.flutter.embedding.engine.systemchannels.LocalizationChannel;
import io.flutter.embedding.engine.systemchannels.NavigationChannel;
import io.flutter.embedding.engine.systemchannels.PlatformChannel;
import io.flutter.embedding.engine.systemchannels.SettingsChannel;
import io.flutter.embedding.engine.systemchannels.SystemChannel;
import io.flutter.embedding.engine.systemchannels.TextInputChannel;
import io.flutter.plugin.platform.PlatformViewsController;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.Set;

/**
 * A single Flutter execution environment.
 *
 * <p>The {@code FlutterEngine} is the container through which Dart code can be run in an Android
 * application.
 *
 * <p>Dart code in a {@code FlutterEngine} can execute in the background, or it can be render to the
 * screen by using the accompanying {@link FlutterRenderer} and Dart code using the Flutter
 * framework on the Dart side. Rendering can be started and stopped, thus allowing a {@code
 * FlutterEngine} to move from UI interaction to data-only processing and then back to UI
 * interaction.
 *
 * <p>Multiple {@code FlutterEngine}s may exist, execute Dart code, and render UIs within a single
 * Android app. Flutter at this point makes no guarantees on the performance of running multiple
 * engines. Use at your own risk. See https://github.com/flutter/flutter/issues/37644 for details.
 *
 * <p>To start running Dart and/or Flutter within this {@code FlutterEngine}, get a reference to
 * this engine's {@link DartExecutor} and then use {@link
 * DartExecutor#executeDartEntrypoint(DartExecutor.DartEntrypoint)}. The {@link
 * DartExecutor#executeDartEntrypoint(DartExecutor.DartEntrypoint)} method must not be invoked twice
 * on the same {@code FlutterEngine}.
 *
 * <p>To start rendering Flutter content to the screen, use {@link #getRenderer()} to obtain a
 * {@link FlutterRenderer} and then attach a {@link RenderSurface}. Consider using a {@link
 * io.flutter.embedding.android.FlutterView} as a {@link RenderSurface}.
 *
 * <p>Instatiating the first {@code FlutterEngine} per process will also load the Flutter engine's
 * native library and start the Dart VM. Subsequent {@code FlutterEngine}s will run on the same VM
 * instance but will have their own Dart <a
 * href="https://api.dartlang.org/stable/dart-isolate/Isolate-class.html">Isolate</a> when the
 * {@link DartExecutor} is run. Each Isolate is a self-contained Dart environment and cannot
 * communicate with each other except via Isolate ports.
 */
public class FlutterEngine {
  private static final String TAG = "FlutterEngine";

  @NonNull private FlutterJNI flutterJNI;
  @NonNull private FlutterRenderer renderer;
  @NonNull private DartExecutor dartExecutor;
  @NonNull private FlutterEnginePluginRegistry pluginRegistry;

  // System channels.
  @NonNull private AccessibilityChannel accessibilityChannel;
  @NonNull private KeyEventChannel keyEventChannel;
  @NonNull private LifecycleChannel lifecycleChannel;
  @NonNull private LocalizationChannel localizationChannel;
  @NonNull private NavigationChannel navigationChannel;
  @NonNull private PlatformChannel platformChannel;
  @NonNull private SettingsChannel settingsChannel;
  @NonNull private SystemChannel systemChannel;
  @NonNull private TextInputChannel textInputChannel;
  @NonNull private Context context;
  @NonNull private FlutterLoader flutterLoader;
  private boolean automaticallyRegisterPlugins;

  // Platform Views.
  @NonNull private PlatformViewsController platformViewsController;

  // Engine Lifecycle.
  @NonNull private final Set<EngineLifecycleListener> engineLifecycleListeners = new HashSet<>();

  @NonNull
  private final EngineLifecycleListener engineLifecycleListener =
      new EngineLifecycleListener() {
        @SuppressWarnings("unused")
        public void onPreEngineRestart() {
          Log.v(TAG, "onPreEngineRestart()");
          for (EngineLifecycleListener lifecycleListener : engineLifecycleListeners) {
            lifecycleListener.onPreEngineRestart();
          }

          platformViewsController.onPreEngineRestart();
        }

        @Override
        public void onAsyncAttachEnd(boolean success) {
          for (EngineLifecycleListener lifecycleListener : engineLifecycleListeners) {
            lifecycleListener.onAsyncAttachEnd(success);
          }
          if (success) {
            initAfterAttachNative();
            onAsyncCreateEngineEnd(true);

          } else {
            Log.e(TAG, "asyncAttach failed");
            onAsyncCreateEngineEnd(false);
          }
        }

        @Override
        public void onAsyncCreateEngineEnd(boolean success) {
          for (EngineLifecycleListener lifecycleListener : engineLifecycleListeners) {
            lifecycleListener.onAsyncCreateEngineEnd(success);
          }
        }
      };

  /**
   * Constructs a new {@code FlutterEngine}.
   *
   * <p>A new {@code FlutterEngine} does not execute any Dart code automatically. See {@link
   * #getDartExecutor()} and {@link DartExecutor#executeDartEntrypoint(DartExecutor.DartEntrypoint)}
   * to begin executing Dart code within this {@code FlutterEngine}.
   *
   * <p>A new {@code FlutterEngine} will not display any UI until a {@link RenderSurface} is
   * registered. See {@link #getRenderer()} and {@link
   * FlutterRenderer#startRenderingToSurface(RenderSurface)}.
   *
   * <p>A new {@code FlutterEngine} automatically attaches all plugins. See {@link #getPlugins()}.
   *
   * <p>A new {@code FlutterEngine} does come with all default system channels attached.
   *
   * <p>The first {@code FlutterEngine} instance constructed per process will also load the Flutter
   * native library and start a Dart VM.
   *
   * <p>In order to pass Dart VM initialization arguments (see {@link
   * io.flutter.embedding.engine.FlutterShellArgs}) when creating the VM, manually set the
   * initialization arguments by calling {@link FlutterLoader#startInitialization(Context)} and
   * {@link FlutterLoader#ensureInitializationComplete(Context, String[])}.
   */
  public FlutterEngine(@NonNull Context context) {
    this(context, null);
  }

  /**
   * Same as {@link #FlutterEngine(Context)} with added support for passing Dart VM arguments.
   *
   * <p>If the Dart VM has already started, the given arguments will have no effect.
   */
  public FlutterEngine(@NonNull Context context, @Nullable String[] dartVmArgs) {
    this(context, FlutterLoader.getInstance(), new FlutterJNI(), dartVmArgs, true);
  }

  /**
   * Same as {@link #FlutterEngine(Context)} with added support for passing Dart VM arguments and
   * avoiding automatic plugin registration.
   *
   * <p>If the Dart VM has already started, the given arguments will have no effect.
   */
  public FlutterEngine(
      @NonNull Context context,
      @Nullable String[] dartVmArgs,
      boolean automaticallyRegisterPlugins) {
    this(
        context,
        FlutterLoader.getInstance(),
        new FlutterJNI(),
        dartVmArgs,
        automaticallyRegisterPlugins);
  }

  /**
   * Same as {@link #FlutterEngine(Context, FlutterLoader, FlutterJNI, String[])} but with no Dart
   * VM flags.
   *
   * <p>{@code flutterJNI} should be a new instance that has never been attached to an engine
   * before.
   */
  public FlutterEngine(
      @NonNull Context context,
      @NonNull FlutterLoader flutterLoader,
      @NonNull FlutterJNI flutterJNI) {
    this(context, flutterLoader, flutterJNI, null, true);
  }

  /**
   * Same as {@link #FlutterEngine(Context, FlutterLoader, FlutterJNI)}, plus Dart VM flags in
   * {@code dartVmArgs}, and control over whether plugins are automatically registered with this
   * {@code FlutterEngine} in {@code automaticallyRegisterPlugins}. If plugins are automatically
   * registered, then they are registered during the execution of this constructor.
   */
  public FlutterEngine(
      @NonNull Context context,
      @NonNull FlutterLoader flutterLoader,
      @NonNull FlutterJNI flutterJNI,
      @Nullable String[] dartVmArgs,
      boolean automaticallyRegisterPlugins) {
    this(
        context,
        flutterLoader,
        flutterJNI,
        new PlatformViewsController(),
        dartVmArgs,
        automaticallyRegisterPlugins);
  }

  /** Fully configurable {@code FlutterEngine} constructor. */
  public FlutterEngine(
      @NonNull Context context,
      @NonNull FlutterLoader flutterLoader,
      @NonNull FlutterJNI flutterJNI,
      @NonNull PlatformViewsController platformViewsController,
      @Nullable String[] dartVmArgs,
      boolean automaticallyRegisterPlugins) {
    doInit(
        context,
        flutterLoader,
        flutterJNI,
        platformViewsController,
        dartVmArgs,
        automaticallyRegisterPlugins,
        null);
  }

  public FlutterEngine() {}

  public void initAsync(
      @NonNull Context appContext, @NonNull EngineLifecycleListener asyncInitCallback) {
    if (asyncInitCallback == null) {
      Log.w(
          TAG,
          "initAsync: callback is null, you should care lifecycle, and called other api after initCallback called");
      asyncInitCallback =
          new EngineLifecycleListener() {
            @Override
            public void onPreEngineRestart() {}

            @Override
            public void onAsyncAttachEnd(boolean success) {}

            @Override
            public void onAsyncCreateEngineEnd(boolean success) {}
          };
    }
    doInit(
        appContext,
        FlutterLoader.getInstance(),
        new FlutterJNI(),
        new PlatformViewsController(),
        null,
        true,
        asyncInitCallback);
  }

  private void doInit(
      @NonNull Context context,
      @NonNull FlutterLoader flutterLoader,
      @NonNull FlutterJNI flutterJNI,
      @NonNull PlatformViewsController platformViewsController,
      @Nullable String[] dartVmArgs,
      boolean automaticallyRegisterPlugins,
      EngineLifecycleListener asyncInitListener) {
    this.flutterJNI = flutterJNI;
    this.context = context;
    this.automaticallyRegisterPlugins = automaticallyRegisterPlugins;
    this.flutterLoader = flutterLoader;
    this.platformViewsController = platformViewsController;
    flutterLoader.startInitialization(context.getApplicationContext());
    flutterLoader.ensureInitializationComplete(context, dartVmArgs);

    flutterJNI.addEngineLifecycleListener(engineLifecycleListener);
    if (asyncInitListener != null) {
      addEngineLifecycleListener(asyncInitListener);
      attachToJni(true);
      return;
    }
    attachToJni(false);
    initAfterAttachNative();
  }

  private void initAfterAttachNative() {
    this.dartExecutor = new DartExecutor(this.flutterJNI, this.context.getAssets());
    this.dartExecutor.onAttachedToJNI();

    // TODO(mattcarroll): FlutterRenderer is temporally coupled to attach(). Remove that coupling if
    // possible.
    this.renderer = new FlutterRenderer(this.flutterJNI);

    accessibilityChannel = new AccessibilityChannel(dartExecutor, this.flutterJNI);
    keyEventChannel = new KeyEventChannel(dartExecutor);
    lifecycleChannel = new LifecycleChannel(dartExecutor);
    localizationChannel = new LocalizationChannel(dartExecutor);
    navigationChannel = new NavigationChannel(dartExecutor);
    platformChannel = new PlatformChannel(dartExecutor);
    settingsChannel = new SettingsChannel(dartExecutor);
    systemChannel = new SystemChannel(dartExecutor);
    textInputChannel = new TextInputChannel(dartExecutor);

    this.platformViewsController.onAttachedToJNI();
    this.pluginRegistry =
        new FlutterEnginePluginRegistry(
            this.context.getApplicationContext(), this, this.flutterLoader);

    if (automaticallyRegisterPlugins) {
      registerPlugins();
    }
  }

  private void attachToJni(boolean asyncInitMode) {
    Log.v(TAG, "Attaching to JNI.");
    // TODO(mattcarroll): update native call to not take in "isBackgroundView"
    flutterJNI.attachToNative(false, asyncInitMode);

    if (!asyncInitMode && !isAttachedToJni()) {
      throw new RuntimeException("FlutterEngine failed to attach to its native Object reference.");
    }
  }

  @SuppressWarnings("BooleanMethodIsAlwaysInverted")
  private boolean isAttachedToJni() {
    return flutterJNI.isAttached();
  }

  /**
   * Registers all plugins that an app lists in its pubspec.yaml.
   *
   * <p>The Flutter tool generates a class called GeneratedPluginRegistrant, which includes the code
   * necessary to register every plugin in the pubspec.yaml with a given {@code FlutterEngine}. The
   * GeneratedPluginRegistrant must be generated per app, because each app uses different sets of
   * plugins. Therefore, the Android embedding cannot place a compile-time dependency on this
   * generated class. This method uses reflection to attempt to locate the generated file and then
   * use it at runtime.
   *
   * <p>This method fizzles if the GeneratedPluginRegistrant cannot be found or invoked. This
   * situation should never occur, but if any eventuality comes up that prevents an app from using
   * this behavior, that app can still write code that explicitly registers plugins.
   */
  private void registerPlugins() {
    try {
      Class<?> generatedPluginRegistrant =
          Class.forName("io.flutter.plugins.GeneratedPluginRegistrant");
      Method registrationMethod =
          generatedPluginRegistrant.getDeclaredMethod("registerWith", FlutterEngine.class);
      registrationMethod.invoke(null, this);
    } catch (Exception e) {
      Log.w(
          TAG,
          "Tried to automatically register plugins with FlutterEngine ("
              + this
              + ") but could not find and invoke the GeneratedPluginRegistrant.");
    }
  }

  /**
   * Cleans up all components within this {@code FlutterEngine} and destroys the associated Dart
   * Isolate. All state held by the Dart Isolate, such as the Flutter Elements tree, is lost.
   *
   * <p>This {@code FlutterEngine} instance should be discarded after invoking this method.
   */
  public void destroy() {
    Log.v(TAG, "Destroying.");
    // The order that these things are destroyed is important.
    pluginRegistry.destroy();
    platformViewsController.onDetachedFromJNI();
    dartExecutor.onDetachedFromJNI();
    flutterJNI.removeEngineLifecycleListener(engineLifecycleListener);
    flutterJNI.detachFromNativeAndReleaseResources();
  }

  /**
   * Adds a {@code listener} to be notified of Flutter engine lifecycle events, e.g., {@code
   * onPreEngineStart()}.
   */
  public void addEngineLifecycleListener(@NonNull EngineLifecycleListener listener) {
    engineLifecycleListeners.add(listener);
  }

  /**
   * Removes a {@code listener} that was previously added with {@link
   * #addEngineLifecycleListener(EngineLifecycleListener)}.
   */
  public void removeEngineLifecycleListener(@NonNull EngineLifecycleListener listener) {
    engineLifecycleListeners.remove(listener);
  }

  /**
   * The Dart execution context associated with this {@code FlutterEngine}.
   *
   * <p>The {@link DartExecutor} can be used to start executing Dart code from a given entrypoint.
   * See {@link DartExecutor#executeDartEntrypoint(DartExecutor.DartEntrypoint)}.
   *
   * <p>Use the {@link DartExecutor} to connect any desired message channels and method channels to
   * facilitate communication between Android and Dart/Flutter.
   */
  @NonNull
  public DartExecutor getDartExecutor() {
    return dartExecutor;
  }

  /**
   * The rendering system associated with this {@code FlutterEngine}.
   *
   * <p>To render a Flutter UI that is produced by this {@code FlutterEngine}'s Dart code, attach a
   * {@link RenderSurface} to this {@link FlutterRenderer}.
   */
  @NonNull
  public FlutterRenderer getRenderer() {
    return renderer;
  }

  /** System channel that sends accessibility requests and events from Flutter to Android. */
  @NonNull
  public AccessibilityChannel getAccessibilityChannel() {
    return accessibilityChannel;
  }

  /** System channel that sends key events from Android to Flutter. */
  @NonNull
  public KeyEventChannel getKeyEventChannel() {
    return keyEventChannel;
  }

  /** System channel that sends Android lifecycle events to Flutter. */
  @NonNull
  public LifecycleChannel getLifecycleChannel() {
    return lifecycleChannel;
  }

  /** System channel that sends locale data from Android to Flutter. */
  @NonNull
  public LocalizationChannel getLocalizationChannel() {
    return localizationChannel;
  }

  /** System channel that sends Flutter navigation commands from Android to Flutter. */
  @NonNull
  public NavigationChannel getNavigationChannel() {
    return navigationChannel;
  }

  /**
   * System channel that sends platform-oriented requests and information to Flutter, e.g., requests
   * to play sounds, requests for haptics, system chrome settings, etc.
   */
  @NonNull
  public PlatformChannel getPlatformChannel() {
    return platformChannel;
  }

  /**
   * System channel that sends platform/user settings from Android to Flutter, e.g., time format,
   * scale factor, etc.
   */
  @NonNull
  public SettingsChannel getSettingsChannel() {
    return settingsChannel;
  }

  /** System channel that sends memory pressure warnings from Android to Flutter. */
  @NonNull
  public SystemChannel getSystemChannel() {
    return systemChannel;
  }

  /** System channel that sends and receives text input requests and state. */
  @NonNull
  public TextInputChannel getTextInputChannel() {
    return textInputChannel;
  }

  /**
   * Plugin registry, which registers plugins that want to be applied to this {@code FlutterEngine}.
   */
  @NonNull
  public PluginRegistry getPlugins() {
    return pluginRegistry;
  }

  /**
   * {@code PlatformViewsController}, which controls all platform views running within this {@code
   * FlutterEngine}.
   */
  @NonNull
  public PlatformViewsController getPlatformViewsController() {
    return platformViewsController;
  }

  @NonNull
  public ActivityControlSurface getActivityControlSurface() {
    return pluginRegistry;
  }

  @NonNull
  public ServiceControlSurface getServiceControlSurface() {
    return pluginRegistry;
  }

  @NonNull
  public BroadcastReceiverControlSurface getBroadcastReceiverControlSurface() {
    return pluginRegistry;
  }

  @NonNull
  public ContentProviderControlSurface getContentProviderControlSurface() {
    return pluginRegistry;
  }

  /** Lifecycle callbacks for Flutter engine lifecycle events. */
  public interface EngineLifecycleListener {
    /** Lifecycle callback invoked before a hot restart of the Flutter engine. */
    void onPreEngineRestart();
    /**
     * Lifecycle callback invoked after flutterJni.asyncAttachNative end (native engine env ready).
     */
    void onAsyncAttachEnd(boolean success);

    void onAsyncCreateEngineEnd(boolean success);
  }
}
