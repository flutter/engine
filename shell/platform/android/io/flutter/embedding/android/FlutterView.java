// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.res.Configuration;
import android.database.ContentObserver;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.provider.Settings;
import android.support.annotation.ColorInt;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.format.DateFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Locale;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.legacy.AccessibilityBridge;
import io.flutter.embedding.legacy.TextInputPlugin;
import io.flutter.plugin.common.StandardMessageCodec;
import io.flutter.view.VsyncWaiter;

/**
 * {@code View} which can render a Flutter UI by attaching itself to a {@link FlutterRenderer}.
 *
 * {@code FlutterView} does not take in any Flutter references within its constructor. This is done
 * so that {@code FlutterView} can be instantiated from XML, and recreated on configuration change
 * without introducing asymmetric APIs for construction.  As a result, users of {@code FlutterView}
 * must explicitly attach to Flutter, and detach from Flutter, as desired.
 *
 * To start rendering a Flutter UI, use {@link #attachToFlutterEngine(FlutterEngine)}.
 *
 * To stop rendering a Flutter UI, use {@link #detachFromFlutterEngine()}.
 *
 * {@code FlutterView} extends {@link SurfaceView} because Flutter renders directly to a
 * {@link android.view.Surface}, which is then displayed by this {@link SurfaceView}.
 *
 * {@code FlutterView} implements {@link FlutterRenderer.RenderSurface} so that it can operate with
 * a {@link FlutterRenderer} to render an interactive Flutter UI. {@code FlutterView} sends commands
 * to its {@link FlutterRenderer}, and the {@link FlutterRenderer} sends updates to this
 * {@code FlutterView} by utilizing it as a {@link FlutterRenderer.RenderSurface}.
 *
 * See also
 *  - {@link SurfaceView}, which displays {@link android.view.Surface}s.
 *  - {@link FlutterRenderer.RenderSurface}, which is the interface that is implemented by anything
 *    that wants to render a Flutter UI.
 *  - {@link android.view.accessibility.AccessibilityManager.AccessibilityStateChangeListener}, which
 *    receives callbacks when Android accessibility settings are changed.
 */
@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class FlutterView extends SurfaceView implements
    FlutterRenderer.RenderSurface,
    AccessibilityManager.AccessibilityStateChangeListener {

  private static final String TAG = "FlutterView";

  // Must match the PointerChange enum in pointer.dart.
  private static final int kPointerChangeCancel = 0;
  private static final int kPointerChangeAdd = 1;
  private static final int kPointerChangeRemove = 2;
  private static final int kPointerChangeHover = 3;
  private static final int kPointerChangeDown = 4;
  private static final int kPointerChangeMove = 5;
  private static final int kPointerChangeUp = 6;

  // Must match the PointerDeviceKind enum in pointer.dart.
  private static final int kPointerDeviceKindTouch = 0;
  private static final int kPointerDeviceKindMouse = 1;
  private static final int kPointerDeviceKindStylus = 2;
  private static final int kPointerDeviceKindInvertedStylus = 3;
  private static final int kPointerDeviceKindUnknown = 4;

  static final class ViewportMetrics {
    float devicePixelRatio = 1.0f;
    int physicalWidth = 0;
    int physicalHeight = 0;
    int physicalPaddingTop = 0;
    int physicalPaddingRight = 0;
    int physicalPaddingBottom = 0;
    int physicalPaddingLeft = 0;
    int physicalViewInsetTop = 0;
    int physicalViewInsetRight = 0;
    int physicalViewInsetBottom = 0;
    int physicalViewInsetLeft = 0;
  }

  // Splash
  private boolean isPreRender = true; // TODO(mattcarroll): save this to View state for rotation

  // Transparency
  private boolean isTransparencyDesired = false; // TODO(mattcarroll): save this to View state for rotation

  // Flutter engine
  private FlutterEngine flutterEngine;
  private boolean isAttachedToFlutterEngine = false;

  // Track surface status
  private boolean isSurfaceAvailableForRendering = false;
  private SurfaceHolder surfaceHolder;

  // User input
  private final InputMethodManager imm;
  private TextInputPlugin textInputPlugin;
  private InputConnection lastInputConnection;

  // Android UI properties
  private final ViewportMetrics metrics;
  private final AnimationScaleObserver animationScaleObserver;
  private boolean isSoftwareRenderingEnabled = false; // using the software renderer or not

  // Accessibility
  private final AccessibilityManager accessibilityManager;
  private AccessibilityBridge accessibilityNodeProvider;
  private TouchExplorationListener touchExplorationListener;
  private boolean accessibilityEnabled = false;
  private boolean touchExplorationEnabled = false;
  private int accessibilityFeatureFlags = 0;

  // Connects the {@code Surface} beneath this {@code SurfaceView} with Flutter's native code.
  // Callbacks are received by this Object and then those messages are forwarded to our
  // FlutterRenderer, and then on to the JNI bridge over to native Flutter code.
  private final SurfaceHolder.Callback surfaceCallback = new SurfaceHolder.Callback() {
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
      Log.d(TAG, "SurfaceHolder.Callback.surfaceCreated()");
      isSurfaceAvailableForRendering = true;
      surfaceHolder = holder;

      if (isAttachedToFlutterEngine) {
        Log.d(TAG, "Already attached to renderer. Notifying of surface creation.");
        flutterEngine.getRenderer().surfaceCreated(holder.getSurface());
      }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
      if (isAttachedToFlutterEngine) {
        flutterEngine.getRenderer().surfaceChanged(width, height);
      }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
      Log.d(TAG, "SurfaceHolder.Callback.surfaceDestroyed()");
      isSurfaceAvailableForRendering = false;
      surfaceHolder = null;

      if (isAttachedToFlutterEngine) {
        flutterEngine.getRenderer().surfaceDestroyed();
      }
    }
  };

  //------ START VIEW OVERRIDES -----
  public FlutterView(Context context) {
    this(context, null);
  }

  public FlutterView(Context context, AttributeSet attrs) {
    super(context, attrs);

    // Cache references to Objects used throughout FlutterView.
    metrics = new ViewportMetrics();
    metrics.devicePixelRatio = context.getResources().getDisplayMetrics().density;
    imm = (InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
    accessibilityManager = (AccessibilityManager) getContext().getSystemService(Context.ACCESSIBILITY_SERVICE);
    animationScaleObserver = new AnimationScaleObserver(new Handler());

    // Apply a splash background color if desired.
    // TODO(mattcarroll): support attr control

    // Initialize this View as needed.
    setFocusable(true);
    setFocusableInTouchMode(true);
  }

  @Override
  protected void onAttachedToWindow() {
    super.onAttachedToWindow();
    Log.d(TAG, "onAttachedToWindow()");

    // Grab a reference to our underlying Surface and register callbacks with that Surface so we
    // can monitor changes and forward those changes on to native Flutter code.
    getHolder().addCallback(surfaceCallback);

    // If we're already attached to a FlutterEngine then we're now attached to both an engine
    // and the Android window, so FlutterView can begin rendering now.
    if (isAttachedToFlutterEngine) {
      onAttachedToWindowAndEngine();
    }
  }

  @Override
  protected void onDetachedFromWindow() {
    super.onDetachedFromWindow();
    Log.d(TAG, "onDetachedFromWindow()");

    // If we're attached to a FlutterEngine then we were rendering a Flutter UI. Now that
    // this FlutterView is detached from the window, we need to stop rendering.
    if (isAttachedToFlutterEngine) {
      onDetachedFromWindowOrEngine();
    }
  }

  /**
   * Start rendering the UI for the given {@link FlutterEngine}.
   *
   * @param flutterEngine the FlutterEngine for which this FlutterView will be a RenderSurface
   */
  public void attachToFlutterEngine(@NonNull FlutterEngine flutterEngine) {
    if (isAttachedToFlutterEngine) {
      if (flutterEngine == this.flutterEngine) {
        // We are already attached to this FlutterEngine
        return;
      }

      detachFromFlutterEngine();
    }

    this.flutterEngine = flutterEngine;
    isAttachedToFlutterEngine = true;

    // Instruct our FlutterRenderer that we are now its designated RenderSurface.
    this.flutterEngine.getRenderer().attachToRenderSurface(this);

    // Configure a TextInputPlugin to send text input events from this FlutterView to
    // the Flutter app, and to respond to input events received from Flutter.
    textInputPlugin = new TextInputPlugin(this, flutterEngine.getSystemChannels().textInput);

    // TODO(mattcarroll): when we get the build system to recognize recent APIs, this call should be "getLocales().get(0)"
    sendLocaleToFlutter(getResources().getConfiguration().locale);
    sendUserSettingsToFlutter();

    isSoftwareRenderingEnabled = this.flutterEngine.getRenderer().isSoftwareRenderingEnabled();

    if (isAttachedToWindow()) {
      onAttachedToWindowAndEngine();
    }
  }

  /**
   * Stop rendering the UI for a given {@link FlutterEngine}.
   *
   * If no {@link FlutterEngine} is currently attached, this method does nothing.
   */
  public void detachFromFlutterEngine() {
    if (!isAttachedToFlutterEngine) {
      return;
    }
    Log.d(TAG, "Detaching from Flutter Engine");

    // Instruct our FlutterRenderer that we are no longer interested in being its RenderSurface.
    flutterEngine.getRenderer().detachFromRenderSurface();
    flutterEngine = null;

    // Destroy the TextInputPlugin because either there is no more FlutterEngine to receive
    // text input.
    textInputPlugin = null;

    isAttachedToFlutterEngine = false;

    // TODO(mattcarroll): clear the surface when JNI doesn't blow up
//    if (isSurfaceAvailableForRendering) {
//      Canvas canvas = surfaceHolder.lockCanvas();
//      canvas.drawColor(Color.RED);
//      surfaceHolder.unlockCanvasAndPost(canvas);
//    }

    if (isAttachedToWindow()) {
      onDetachedFromWindowOrEngine();
    }
  }

  private void onAttachedToWindowAndEngine() {
    Log.d(TAG, "onAttachedToWindowAndEngine()");
    if (isSurfaceAvailableForRendering) {
      Log.d(TAG, "Surface already exists. Connecting it to Flutter.");
      flutterEngine.getRenderer().surfaceCreated(surfaceHolder.getSurface());
    }

    // Read accessibility settings.
    accessibilityEnabled = accessibilityManager.isEnabled();
    touchExplorationEnabled = accessibilityManager.isTouchExplorationEnabled();
    if (accessibilityEnabled || touchExplorationEnabled) {
      ensureAccessibilityEnabled();
    }
    if (touchExplorationEnabled) {
      accessibilityFeatureFlags ^= AccessibilityFeature.ACCESSIBLE_NAVIGATION.value;
    }

    // Apply additional accessibility settings
    updateAccessibilityFeatures();
    resetWillNotDraw();
    accessibilityManager.addAccessibilityStateChangeListener(this);

    // Start listening for changes to accessibility settings.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
      if (touchExplorationListener == null) {
        touchExplorationListener = new TouchExplorationListener();
      }
      accessibilityManager.addTouchExplorationStateChangeListener(touchExplorationListener);
    }

    // Start listening for changes to Android's animation scale setting.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
      Uri transitionUri = Settings.Global.getUriFor(Settings.Global.TRANSITION_ANIMATION_SCALE);
      getContext().getContentResolver().registerContentObserver(transitionUri, false, animationScaleObserver);
    }
  }

  private void onDetachedFromWindowOrEngine() {
    Log.d(TAG, "onDetachedFromWindowOrEngine");
    // Stop forwarding messages from our underlying Surface to native Flutter code.
    getHolder().removeCallback(surfaceCallback);

    // Stop listening for changes to Android's animation scale setting.
    getContext().getContentResolver().unregisterContentObserver(animationScaleObserver);

    // Stop listening for changes to accessibility settings.
    accessibilityManager.removeAccessibilityStateChangeListener(this);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
      accessibilityManager.removeTouchExplorationStateChangeListener(touchExplorationListener);
    }
  }

  @Override
  public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
    if (isAttachedToWindow() && isAttachedToFlutterEngine) {
      lastInputConnection = textInputPlugin.createInputConnection(this, outAttrs);
      return lastInputConnection;
    } else {
      return null;
    }
  }

  @Override
  protected void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
    // TODO(mattcarroll): when we get the build system to recognize recent APIs, this call should be "getLocales().get(0)"
    sendLocaleToFlutter(newConfig.locale);
    sendUserSettingsToFlutter();
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    if (!isAttachedToFlutterEngine) {
      return false;
    }

    // TODO(abarth): This version check might not be effective in some
    // versions of Android that statically compile code and will be upset
    // at the lack of |requestUnbufferedDispatch|. Instead, we should factor
    // version-dependent code into separate classes for each supported
    // version and dispatch dynamically.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      requestUnbufferedDispatch(event);
    }

    // These values must match the unpacking code in hooks.dart.
    final int kPointerDataFieldCount = 19;
    final int kBytePerField = 8;

    int pointerCount = event.getPointerCount();

    ByteBuffer packet = ByteBuffer.allocateDirect(pointerCount * kPointerDataFieldCount * kBytePerField);
    packet.order(ByteOrder.LITTLE_ENDIAN);

    int maskedAction = event.getActionMasked();
    // ACTION_UP, ACTION_POINTER_UP, ACTION_DOWN, and ACTION_POINTER_DOWN
    // only apply to a single pointer, other events apply to all pointers.
    if (maskedAction == MotionEvent.ACTION_UP || maskedAction == MotionEvent.ACTION_POINTER_UP
        || maskedAction == MotionEvent.ACTION_DOWN || maskedAction == MotionEvent.ACTION_POINTER_DOWN) {
      addPointerForIndex(event, event.getActionIndex(), packet);
    } else {
      // ACTION_MOVE may not actually mean all pointers have moved
      // but it's the responsibility of a later part of the system to
      // ignore 0-deltas if desired.
      for (int p = 0; p < pointerCount; p++) {
        addPointerForIndex(event, p, packet);
      }
    }

    assert packet.position() % (kPointerDataFieldCount * kBytePerField) == 0;
    flutterEngine.getRenderer().dispatchPointerDataPacket(packet, packet.position());
    return true;
  }

  private void addPointerForIndex(MotionEvent event, int pointerIndex, ByteBuffer packet) {
    int pointerChange = getPointerChangeForAction(event.getActionMasked());
    if (pointerChange == -1) {
      return;
    }

    int pointerKind = getPointerDeviceTypeForToolType(event.getToolType(pointerIndex));

    long timeStamp = event.getEventTime() * 1000; // Convert from milliseconds to microseconds.

    packet.putLong(timeStamp); // time_stamp
    packet.putLong(pointerChange); // change
    packet.putLong(pointerKind); // kind
    packet.putLong(event.getPointerId(pointerIndex)); // device
    packet.putDouble(event.getX(pointerIndex)); // physical_x
    packet.putDouble(event.getY(pointerIndex)); // physical_y

    if (pointerKind == kPointerDeviceKindMouse) {
      packet.putLong(event.getButtonState() & 0x1F); // buttons
    } else if (pointerKind == kPointerDeviceKindStylus) {
      packet.putLong((event.getButtonState() >> 4) & 0xF); // buttons
    } else {
      packet.putLong(0); // buttons
    }

    packet.putLong(0); // obscured

    // TODO(eseidel): Could get the calibrated range if necessary:
    // event.getDevice().getMotionRange(MotionEvent.AXIS_PRESSURE)
    packet.putDouble(event.getPressure(pointerIndex)); // pressure
    packet.putDouble(0.0); // pressure_min
    packet.putDouble(1.0); // pressure_max

    if (pointerKind == kPointerDeviceKindStylus) {
      packet.putDouble(event.getAxisValue(MotionEvent.AXIS_DISTANCE, pointerIndex)); // distance
      packet.putDouble(0.0); // distance_max
    } else {
      packet.putDouble(0.0); // distance
      packet.putDouble(0.0); // distance_max
    }

    packet.putDouble(event.getToolMajor(pointerIndex)); // radius_major
    packet.putDouble(event.getToolMinor(pointerIndex)); // radius_minor

    packet.putDouble(0.0); // radius_min
    packet.putDouble(0.0); // radius_max

    packet.putDouble(event.getAxisValue(MotionEvent.AXIS_ORIENTATION, pointerIndex)); // orientation

    if (pointerKind == kPointerDeviceKindStylus) {
      packet.putDouble(event.getAxisValue(MotionEvent.AXIS_TILT, pointerIndex)); // tilt
    } else {
      packet.putDouble(0.0); // tilt
    }
  }

  private int getPointerChangeForAction(int maskedAction) {
    // Primary pointer:
    if (maskedAction == MotionEvent.ACTION_DOWN) {
      return kPointerChangeDown;
    }
    if (maskedAction == MotionEvent.ACTION_UP) {
      return kPointerChangeUp;
    }
    // Secondary pointer:
    if (maskedAction == MotionEvent.ACTION_POINTER_DOWN) {
      return kPointerChangeDown;
    }
    if (maskedAction == MotionEvent.ACTION_POINTER_UP) {
      return kPointerChangeUp;
    }
    // All pointers:
    if (maskedAction == MotionEvent.ACTION_MOVE) {
      return kPointerChangeMove;
    }
    if (maskedAction == MotionEvent.ACTION_CANCEL) {
      return kPointerChangeCancel;
    }
    return -1;
  }

  private int getPointerDeviceTypeForToolType(int toolType) {
    switch (toolType) {
      case MotionEvent.TOOL_TYPE_FINGER:
        return kPointerDeviceKindTouch;
      case MotionEvent.TOOL_TYPE_STYLUS:
        return kPointerDeviceKindStylus;
      case MotionEvent.TOOL_TYPE_MOUSE:
        return kPointerDeviceKindMouse;
      case MotionEvent.TOOL_TYPE_ERASER:
        return kPointerDeviceKindInvertedStylus;
      default:
        // MotionEvent.TOOL_TYPE_UNKNOWN will reach here.
        return kPointerDeviceKindUnknown;
    }
  }

  @Override
  public boolean onHoverEvent(MotionEvent event) {
    if (!isAttachedToFlutterEngine) {
      return false;
    }

    boolean handled = handleAccessibilityHoverEvent(event);
    if (!handled) {
      // TODO(ianh): Expose hover events to the platform,
      // implementing ADD, REMOVE, etc.
    }
    return handled;
  }

  private boolean handleAccessibilityHoverEvent(MotionEvent event) {
    if (!touchExplorationEnabled) {
      return false;
    }
    if (event.getAction() == MotionEvent.ACTION_HOVER_ENTER || event.getAction() == MotionEvent.ACTION_HOVER_MOVE) {
      accessibilityNodeProvider.handleTouchExploration(event.getX(), event.getY());
    } else if (event.getAction() == MotionEvent.ACTION_HOVER_EXIT) {
      accessibilityNodeProvider.handleTouchExplorationExit();
    } else {
      Log.d("flutter", "unexpected accessibility hover event: " + event);
      return false;
    }
    return true;
  }

  @Override
  protected void onSizeChanged(int width, int height, int oldWidth, int oldHeight) {
    metrics.physicalWidth = width;
    metrics.physicalHeight = height;
    sendViewportMetricsToFlutter();
    super.onSizeChanged(width, height, oldWidth, oldHeight);
  }

  // TODO(mattcarroll): window insets are API 20. what should we do for lower APIs?
  @SuppressLint("NewApi")
  @Override
  public final WindowInsets onApplyWindowInsets(WindowInsets insets) {
    // Status bar, left/right system insets partially obscure content (padding).
    metrics.physicalPaddingTop = insets.getSystemWindowInsetTop();
    metrics.physicalPaddingRight = insets.getSystemWindowInsetRight();
    metrics.physicalPaddingBottom = 0;
    metrics.physicalPaddingLeft = insets.getSystemWindowInsetLeft();

    // Bottom system inset (keyboard) should adjust scrollable bottom edge (inset).
    metrics.physicalViewInsetTop = 0;
    metrics.physicalViewInsetRight = 0;
    metrics.physicalViewInsetBottom = insets.getSystemWindowInsetBottom();
    metrics.physicalViewInsetLeft = 0;
    sendViewportMetricsToFlutter();
    return super.onApplyWindowInsets(insets);
  }

  @Override
  @SuppressWarnings("deprecation")
  protected boolean fitSystemWindows(Rect insets) {
    if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
      // Status bar, left/right system insets partially obscure content (padding).
      metrics.physicalPaddingTop = insets.top;
      metrics.physicalPaddingRight = insets.right;
      metrics.physicalPaddingBottom = 0;
      metrics.physicalPaddingLeft = insets.left;

      // Bottom system inset (keyboard) should adjust scrollable bottom edge (inset).
      metrics.physicalViewInsetTop = 0;
      metrics.physicalViewInsetRight = 0;
      metrics.physicalViewInsetBottom = insets.bottom;
      metrics.physicalViewInsetLeft = 0;
      sendViewportMetricsToFlutter();
      return true;
    } else {
      return super.fitSystemWindows(insets);
    }
  }

  private void sendViewportMetricsToFlutter() {
    if (!isAttachedToFlutterEngine)
      return;

    flutterEngine.getRenderer().setViewportMetrics(
      metrics.devicePixelRatio,
      metrics.physicalWidth,
      metrics.physicalHeight,
      metrics.physicalPaddingTop,
      metrics.physicalPaddingRight,
      metrics.physicalPaddingBottom,
      metrics.physicalPaddingLeft,
      metrics.physicalViewInsetTop,
      metrics.physicalViewInsetRight,
      metrics.physicalViewInsetBottom,
      metrics.physicalViewInsetLeft
    );

    WindowManager wm = (WindowManager) getContext().getSystemService(Context.WINDOW_SERVICE);
    float fps = wm.getDefaultDisplay().getRefreshRate();
    VsyncWaiter.refreshPeriodNanos = (long) (1000000000.0 / fps);
  }

  @Override
  public boolean onKeyUp(int keyCode, KeyEvent event) {
    if (!isAttachedToFlutterEngine) {
      return super.onKeyUp(keyCode, event);
    }

    flutterEngine.getSystemChannels().keyEvent.keyUp(event);
    return super.onKeyUp(keyCode, event);
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    if (!isAttachedToFlutterEngine) {
      return super.onKeyDown(keyCode, event);
    }

    if (event.getDeviceId() != KeyCharacterMap.VIRTUAL_KEYBOARD) {
      if (lastInputConnection != null && imm.isAcceptingText()) {
        lastInputConnection.sendKeyEvent(event);
      }
    }

    flutterEngine.getSystemChannels().keyEvent.keyDown(event);
    return super.onKeyDown(keyCode, event);
  }
  //------ END VIEW OVERRIDES ----

  //------ START SPLASH CONTROL -----
  @SuppressWarnings("unused")
  public void setSplashColor(@ColorInt int color) {
    // Only worry about the splash background if we have yet to render Flutter's first frame.
    if (isPreRender) {
      disableTransparentBackground();
      setBackgroundColor(color);
    }
  }

  @SuppressWarnings("unused")
  public void setSplashDrawable(@Nullable Drawable drawable) {
    // Only worry about the splash background if we have yet to render Flutter's first frame.
    if (isPreRender) {
      // Only disable background transparency if we're actually showing a background drawable.
      if (drawable != null) {
        disableTransparentBackground();
      }
      setBackground(drawable);
    }
  }

  private void removeSplash() {
    isPreRender = false;

    setBackground(null);

    if (isTransparencyDesired) {
      enableTransparentBackground();
    }
  }
  //------ END SPLASH CONTROL -----

  //------ START TRANSPARENCY -----
  /**
   * Updates this to support rendering as a transparent {@link SurfaceView}.
   *
   * Sets it on top of its window. The background color still needs to be
   * controlled from within the Flutter UI itself.
   */
  public void enableTransparentBackground() {
    isTransparencyDesired = true;
    doEnableTransparentBackground();
  }

  // This private method is provided so that we can differentiate between an external
  // desire to have a transparent background vs a temporary internal need to have
  // a transparent background.
  private void doEnableTransparentBackground() {
    setZOrderOnTop(true);
    getHolder().setFormat(PixelFormat.TRANSPARENT);
  }

  /**
   * Reverts this back to the {@link SurfaceView} defaults, at the back of its
   * window and opaque.
   */
  public void disableTransparentBackground() {
    isTransparencyDesired = false;
    doDisableTransparentBackground();
  }

  // This private method is provided so that we can differentiate between an external
  // desire to have an opaque background vs a temporary internal need to have
  // an opaque background.
  private void doDisableTransparentBackground() {
    setZOrderOnTop(false);
    getHolder().setFormat(PixelFormat.OPAQUE);
  }
  //------ END TRANSPARENCY -----

  //----- START AccessibilityStateChangeListener -----
  @Override
  public void onAccessibilityStateChanged(boolean enabled) {
    if (enabled) {
      ensureAccessibilityEnabled();
    } else {
      accessibilityEnabled = false;
      if (accessibilityNodeProvider != null) {
        accessibilityNodeProvider.setAccessibilityEnabled(false);
      }
      flutterEngine.getRenderer().setSemanticsEnabled(false);
    }
    resetWillNotDraw();
  }

  @Override
  public AccessibilityNodeProvider getAccessibilityNodeProvider() {
    if (accessibilityEnabled)
      return accessibilityNodeProvider;
    // TODO(goderbauer): when a11y is off this should return a one-off snapshot of
    // the a11y
    // tree.
    return null;
  }

  private void resetWillNotDraw() {
    if (isSoftwareRenderingEnabled) {
      setWillNotDraw(false);
    } else {
      boolean willDraw = accessibilityEnabled || touchExplorationEnabled;
      setWillNotDraw(!willDraw);
    }
  }
  //----- END AccessibilityStateChangeListener ----

  //------- START ACCESSIBILITY ------
  public void dispatchSemanticsAction(int id, AccessibilityBridge.Action action) {
    dispatchSemanticsAction(id, action, null);
  }

  public void dispatchSemanticsAction(int id, AccessibilityBridge.Action action, Object args) {
    if (!isAttachedToFlutterEngine) {
      return;
    }

    ByteBuffer encodedArgs = null;
    int position = 0;
    if (args != null) {
      encodedArgs = StandardMessageCodec.INSTANCE.encodeMessage(args);
      position = encodedArgs.position();
    }
    flutterEngine.getRenderer().dispatchSemanticsAction(id, action.value, encodedArgs, position);
  }

  public void updateAccessibilityFeatures() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
      String transitionAnimationScale = Settings.Global.getString(getContext().getContentResolver(),
          Settings.Global.TRANSITION_ANIMATION_SCALE);
      if (transitionAnimationScale != null && transitionAnimationScale.equals("0")) {
        accessibilityFeatureFlags ^= AccessibilityFeature.DISABLE_ANIMATIONS.value;
      } else {
        accessibilityFeatureFlags &= ~AccessibilityFeature.DISABLE_ANIMATIONS.value;
      }
    } else {
      // TODO(mattcarroll): we need to do something here for API 16
    }
    flutterEngine.getRenderer().setAccessibilityFeatures(accessibilityFeatureFlags);
  }

  void ensureAccessibilityEnabled() {
    if (!isAttachedToFlutterEngine)
      return;
    accessibilityEnabled = true;
    if (accessibilityNodeProvider == null) {
      accessibilityNodeProvider = new AccessibilityBridge(this, flutterEngine.getSystemChannels().accessibility);
    }
    flutterEngine.getRenderer().setSemanticsEnabled(true);
    accessibilityNodeProvider.setAccessibilityEnabled(true);
  }

  public void resetAccessibilityTree() {
    if (accessibilityNodeProvider != null) {
      accessibilityNodeProvider.reset();
    }
  }
  //------- END ACCESSIBILITY ----

  //----- START FLUTTER INTEGRATION -----
  /**
   * Send the given {@link Locale} configuration to Flutter.
   * @param locale the user's locale
   */
  private void sendLocaleToFlutter(@NonNull Locale locale) {
    flutterEngine.getSystemChannels().localization.setLocale(locale.getLanguage(), locale.getCountry());
  }

  /**
   * Send various user preferences of this Android device to Flutter.
   *
   * For example, sends the user's "text scale factor" preferences, as well as the user's clock
   * format preference.
   */
  private void sendUserSettingsToFlutter() {
    flutterEngine.getSystemChannels().settings.startMessage()
        .setTextScaleFactor(getResources().getConfiguration().fontScale)
        .setUse24HourFormat(DateFormat.is24HourFormat(getContext()))
        .send();
  }
  //----- END FLUTTER INTEGRATION -----

  //------ START RenderingSurface -----
  @Override
  public void updateCustomAccessibilityActions(ByteBuffer buffer, String[] strings) {
    try {
      if (accessibilityNodeProvider != null) {
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        accessibilityNodeProvider.updateCustomAccessibilityActions(buffer, strings);
      }
    } catch (Exception ex) {
      Log.e(TAG, "Uncaught exception while updating local context actions", ex);
    }
  }

  @Override
  public void updateSemantics(ByteBuffer buffer, String[] strings) {
    try {
      if (accessibilityNodeProvider != null) {
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        accessibilityNodeProvider.updateSemantics(buffer, strings);
      }
    } catch (Exception ex) {
      Log.e(TAG, "Uncaught exception while updating semantics", ex);
    }
  }

  @Override
  public void onFirstFrameRendered() {
    removeSplash();
  }
  //------ END RenderingSurface ----

  /// Must match the enum defined in window.dart.
  private enum AccessibilityFeature {
    @SuppressWarnings("PointlessBitwiseExpression")
    ACCESSIBLE_NAVIGATION(1 << 0),
    INVERT_COLORS(1 << 1), // NOT SUPPORTED
    DISABLE_ANIMATIONS(1 << 2);

    AccessibilityFeature(int value) {
      this.value = value;
    }

    final int value;
  }

  // Listens to the global TRANSITION_ANIMATION_SCALE property and notifies us so
  // that we can disable animations in Flutter.
  private class AnimationScaleObserver extends ContentObserver {
    AnimationScaleObserver(Handler handler) {
      super(handler);
    }

    @Override
    public void onChange(boolean selfChange) {
      this.onChange(selfChange, null);
    }

    @Override
    public void onChange(boolean selfChange, Uri uri) {
      String value;

      if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) {
        value = Settings.System.getString(
            getContext().getContentResolver(),
            Settings.System.TRANSITION_ANIMATION_SCALE
        );
      } else {
        value = Settings.Global.getString(
            getContext().getContentResolver(),
            Settings.Global.TRANSITION_ANIMATION_SCALE
        );
      }

      if (value.equals("0")) {
        accessibilityFeatureFlags ^= AccessibilityFeature.DISABLE_ANIMATIONS.value;
      } else {
        accessibilityFeatureFlags &= ~AccessibilityFeature.DISABLE_ANIMATIONS.value;
      }
      flutterEngine.getRenderer().setAccessibilityFeatures(accessibilityFeatureFlags);
    }
  }

  // TODO: TouchExplorationStateChangeListener requires API 19. What do we do about earlier APIs?
  @SuppressLint("NewApi")
  class TouchExplorationListener implements AccessibilityManager.TouchExplorationStateChangeListener {
    @Override
    public void onTouchExplorationStateChanged(boolean enabled) {
      if (enabled) {
        touchExplorationEnabled = true;
        ensureAccessibilityEnabled();
        accessibilityFeatureFlags ^= AccessibilityFeature.ACCESSIBLE_NAVIGATION.value;
        flutterEngine.getRenderer().setAccessibilityFeatures(accessibilityFeatureFlags);
      } else {
        touchExplorationEnabled = false;
        if (accessibilityNodeProvider != null) {
          accessibilityNodeProvider.handleTouchExplorationExit();
        }
        accessibilityFeatureFlags &= ~AccessibilityFeature.ACCESSIBLE_NAVIGATION.value;
        flutterEngine.getRenderer().setAccessibilityFeatures(accessibilityFeatureFlags);
      }
      resetWillNotDraw();
    }
  }
}
