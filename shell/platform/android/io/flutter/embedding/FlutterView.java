// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.res.Configuration;
import android.database.ContentObserver;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.provider.Settings;
import android.support.annotation.NonNull;
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

import org.json.JSONException;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import io.flutter.embedding.legacy.AccessibilityBridge;
import io.flutter.embedding.legacy.TextInputPlugin;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.JSONMessageCodec;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodChannel;
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
 * To start rendering a Flutter UI, use {@link #attachToFlutterRenderer(FlutterRenderer, BinaryMessenger)}.
 *
 * To stop rendering a Flutter UI, use {@link #detachFromFlutterRenderer()}.
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

  private final InputMethodManager mImm;
  private final ViewportMetrics mMetrics;
  private final AccessibilityManager mAccessibilityManager;
  private final AnimationScaleObserver mAnimationScaleObserver;

  private FlutterRenderer flutterRenderer;
  private BinaryMessenger pluginMessenger;

  private TextInputPlugin mTextInputPlugin;
  private MethodChannel mFlutterLocalizationChannel;
  private BasicMessageChannel<Object> mFlutterKeyEventChannel;
  private BasicMessageChannel<Object> mFlutterSettingsChannel;
  private InputConnection mLastInputConnection;

  private boolean isAttachedToRenderer = false;
  private boolean mIsSoftwareRenderingEnabled = false; // using the software renderer or not

  // Accessibility
  private boolean mAccessibilityEnabled = false;
  private boolean mTouchExplorationEnabled = false;
  private int mAccessibilityFeatureFlags = 0;
  private AccessibilityBridge mAccessibilityNodeProvider;
  private TouchExplorationListener mTouchExplorationListener;

  // Connects the {@code Surface} beneath this {@code SurfaceView} with Flutter's native code.
  // Callbacks are received by this Object and then those messages are forwarded to our
  // FlutterRenderer, and then on to the JNI bridge over to native Flutter code.
  private final SurfaceHolder.Callback mSurfaceCallback = new SurfaceHolder.Callback() {
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
      assertAttachedToFlutterRenderer();
      flutterRenderer.surfaceCreated(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
      assertAttachedToFlutterRenderer();
      flutterRenderer.surfaceChanged(width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
      assertAttachedToFlutterRenderer();
      flutterRenderer.surfaceDestroyed();
    }
  };

  //------ START VIEW OVERRIDES -----
  public FlutterView(Context context) {
    this(context, null);
  }

  public FlutterView(Context context, AttributeSet attrs) {
    super(context, attrs);

    // Cache references to Objects used throughout FlutterView.
    mMetrics = new ViewportMetrics();
    mMetrics.devicePixelRatio = context.getResources().getDisplayMetrics().density;
    mImm = (InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
    mAccessibilityManager = (AccessibilityManager) getContext().getSystemService(Context.ACCESSIBILITY_SERVICE);
    mAnimationScaleObserver = new AnimationScaleObserver(new Handler());

    // Apply a splash background color if desired.
    // TODO(mattcarroll): support attr and programmatic control

    // Initialize this View as needed.
    setFocusable(true);
    setFocusableInTouchMode(true);
  }

  @Override
  protected void onAttachedToWindow() {
    super.onAttachedToWindow();
    Log.d(TAG, "onAttachedToWindow()");

    // Read accessibility settings.
    mAccessibilityEnabled = mAccessibilityManager.isEnabled();
    mTouchExplorationEnabled = mAccessibilityManager.isTouchExplorationEnabled();
    if (mAccessibilityEnabled || mTouchExplorationEnabled) {
      ensureAccessibilityEnabled();
    }
    if (mTouchExplorationEnabled) {
      mAccessibilityFeatureFlags ^= AccessibilityFeature.ACCESSIBLE_NAVIGATION.value;
    }

    // Apply additional accessibility settings
    updateAccessibilityFeatures();
    resetWillNotDraw();
    mAccessibilityManager.addAccessibilityStateChangeListener(this);

    // Start listening for changes to accessibility settings.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
      if (mTouchExplorationListener == null) {
        mTouchExplorationListener = new TouchExplorationListener();
      }
      mAccessibilityManager.addTouchExplorationStateChangeListener(mTouchExplorationListener);
    }

    // Start listening for changes to Android's animation scale setting.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
      Uri transitionUri = Settings.Global.getUriFor(Settings.Global.TRANSITION_ANIMATION_SCALE);
      getContext().getContentResolver().registerContentObserver(transitionUri, false, mAnimationScaleObserver);
    }
  }

  @Override
  protected void onDetachedFromWindow() {
    super.onDetachedFromWindow();
    Log.d(TAG, "onDetachedFromWindow()");

    // Stop listening for changes to Android's animation scale setting.
    getContext().getContentResolver().unregisterContentObserver(mAnimationScaleObserver);

    // Stop listening for changes to accessibility settings.
    mAccessibilityManager.removeAccessibilityStateChangeListener(this);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
      mAccessibilityManager.removeTouchExplorationStateChangeListener(mTouchExplorationListener);
    }
  }

  @Override
  public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
    try {
      mLastInputConnection = mTextInputPlugin.createInputConnection(this, outAttrs);
      return mLastInputConnection;
    } catch (JSONException e) {
      Log.e(TAG, "Failed to create input connection", e);
      return null;
    }
  }

  @Override
  protected void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
    setFlutterLocale(newConfig.locale);
    setFlutterUserSettings();
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    if (!isAttachedToRenderer) {
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
    flutterRenderer.dispatchPointerDataPacket(packet, packet.position());
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
    if (!isAttachedToRenderer) {
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
    if (!mTouchExplorationEnabled) {
      return false;
    }
    if (event.getAction() == MotionEvent.ACTION_HOVER_ENTER || event.getAction() == MotionEvent.ACTION_HOVER_MOVE) {
      mAccessibilityNodeProvider.handleTouchExploration(event.getX(), event.getY());
    } else if (event.getAction() == MotionEvent.ACTION_HOVER_EXIT) {
      mAccessibilityNodeProvider.handleTouchExplorationExit();
    } else {
      Log.d("flutter", "unexpected accessibility hover event: " + event);
      return false;
    }
    return true;
  }

  @Override
  protected void onSizeChanged(int width, int height, int oldWidth, int oldHeight) {
    mMetrics.physicalWidth = width;
    mMetrics.physicalHeight = height;
    updateViewportMetrics();
    super.onSizeChanged(width, height, oldWidth, oldHeight);
  }

  // TODO(mattcarroll): window insets are API 20. what should we do for lower APIs?
  @SuppressLint("NewApi")
  @Override
  public final WindowInsets onApplyWindowInsets(WindowInsets insets) {
    // Status bar, left/right system insets partially obscure content (padding).
    mMetrics.physicalPaddingTop = insets.getSystemWindowInsetTop();
    mMetrics.physicalPaddingRight = insets.getSystemWindowInsetRight();
    mMetrics.physicalPaddingBottom = 0;
    mMetrics.physicalPaddingLeft = insets.getSystemWindowInsetLeft();

    // Bottom system inset (keyboard) should adjust scrollable bottom edge (inset).
    mMetrics.physicalViewInsetTop = 0;
    mMetrics.physicalViewInsetRight = 0;
    mMetrics.physicalViewInsetBottom = insets.getSystemWindowInsetBottom();
    mMetrics.physicalViewInsetLeft = 0;
    updateViewportMetrics();
    return super.onApplyWindowInsets(insets);
  }

  @Override
  @SuppressWarnings("deprecation")
  protected boolean fitSystemWindows(Rect insets) {
    if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
      // Status bar, left/right system insets partially obscure content (padding).
      mMetrics.physicalPaddingTop = insets.top;
      mMetrics.physicalPaddingRight = insets.right;
      mMetrics.physicalPaddingBottom = 0;
      mMetrics.physicalPaddingLeft = insets.left;

      // Bottom system inset (keyboard) should adjust scrollable bottom edge (inset).
      mMetrics.physicalViewInsetTop = 0;
      mMetrics.physicalViewInsetRight = 0;
      mMetrics.physicalViewInsetBottom = insets.bottom;
      mMetrics.physicalViewInsetLeft = 0;
      updateViewportMetrics();
      return true;
    } else {
      return super.fitSystemWindows(insets);
    }
  }

  private void updateViewportMetrics() {
    if (!isAttachedToRenderer)
      return;

    flutterRenderer.setViewportMetrics(
      mMetrics.devicePixelRatio,
      mMetrics.physicalWidth,
      mMetrics.physicalHeight,
      mMetrics.physicalPaddingTop,
      mMetrics.physicalPaddingRight,
      mMetrics.physicalPaddingBottom,
      mMetrics.physicalPaddingLeft,
      mMetrics.physicalViewInsetTop,
      mMetrics.physicalViewInsetRight,
      mMetrics.physicalViewInsetBottom,
      mMetrics.physicalViewInsetLeft
    );

    WindowManager wm = (WindowManager) getContext().getSystemService(Context.WINDOW_SERVICE);
    float fps = wm.getDefaultDisplay().getRefreshRate();
    VsyncWaiter.refreshPeriodNanos = (long) (1000000000.0 / fps);
  }

  @Override
  public boolean onKeyUp(int keyCode, KeyEvent event) {
    if (!isAttachedToRenderer) {
      return super.onKeyUp(keyCode, event);
    }

    Map<String, Object> message = new HashMap<>();
    message.put("type", "keyup");
    message.put("keymap", "android");
    encodeKeyEvent(event, message);
    mFlutterKeyEventChannel.send(message);
    return super.onKeyUp(keyCode, event);
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    if (!isAttachedToRenderer) {
      return super.onKeyDown(keyCode, event);
    }

    if (event.getDeviceId() != KeyCharacterMap.VIRTUAL_KEYBOARD) {
      if (mLastInputConnection != null && mImm.isAcceptingText()) {
        mLastInputConnection.sendKeyEvent(event);
      }
    }

    Map<String, Object> message = new HashMap<>();
    message.put("type", "keydown");
    message.put("keymap", "android");
    encodeKeyEvent(event, message);
    mFlutterKeyEventChannel.send(message);
    return super.onKeyDown(keyCode, event);
  }

  private void encodeKeyEvent(KeyEvent event, Map<String, Object> message) {
    message.put("flags", event.getFlags());
    message.put("codePoint", event.getUnicodeChar());
    message.put("keyCode", event.getKeyCode());
    message.put("scanCode", event.getScanCode());
    message.put("metaState", event.getMetaState());
  }
  //------ END VIEW OVERRIDES ----

  //----- START AccessibilityStateChangeListener -----
  @Override
  public void onAccessibilityStateChanged(boolean enabled) {
    if (enabled) {
      ensureAccessibilityEnabled();
    } else {
      mAccessibilityEnabled = false;
      if (mAccessibilityNodeProvider != null) {
        mAccessibilityNodeProvider.setAccessibilityEnabled(false);
      }
      flutterRenderer.setSemanticsEnabled(false);
    }
    resetWillNotDraw();
  }

  @Override
  public AccessibilityNodeProvider getAccessibilityNodeProvider() {
    if (mAccessibilityEnabled)
      return mAccessibilityNodeProvider;
    // TODO(goderbauer): when a11y is off this should return a one-off snapshot of
    // the a11y
    // tree.
    return null;
  }

  private void resetWillNotDraw() {
    if (!mIsSoftwareRenderingEnabled) {
      setWillNotDraw(!(mAccessibilityEnabled || mTouchExplorationEnabled));
    } else {
      setWillNotDraw(false);
    }
  }
  //----- END AccessibilityStateChangeListener ----

  //------- START ACCESSIBILITY ------
  public void dispatchSemanticsAction(int id, AccessibilityBridge.Action action) {
    dispatchSemanticsAction(id, action, null);
  }

  public void dispatchSemanticsAction(int id, AccessibilityBridge.Action action, Object args) {
    if (!isAttachedToRenderer)
      return;
    ByteBuffer encodedArgs = null;
    int position = 0;
    if (args != null) {
      encodedArgs = StandardMessageCodec.INSTANCE.encodeMessage(args);
      position = encodedArgs.position();
    }
    flutterRenderer.dispatchSemanticsAction(id, action.value, encodedArgs, position);
  }

  public void updateAccessibilityFeatures() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
      String transitionAnimationScale = Settings.Global.getString(getContext().getContentResolver(),
          Settings.Global.TRANSITION_ANIMATION_SCALE);
      if (transitionAnimationScale != null && transitionAnimationScale.equals("0")) {
        mAccessibilityFeatureFlags ^= AccessibilityFeature.DISABLE_ANIMATIONS.value;
      } else {
        mAccessibilityFeatureFlags &= ~AccessibilityFeature.DISABLE_ANIMATIONS.value;
      }
    }
    flutterRenderer.setAccessibilityFeatures(mAccessibilityFeatureFlags);
  }

  void ensureAccessibilityEnabled() {
    if (!isAttachedToRenderer)
      return;
    mAccessibilityEnabled = true;
    if (mAccessibilityNodeProvider == null) {
      mAccessibilityNodeProvider = new AccessibilityBridge(this, pluginMessenger);
    }
    flutterRenderer.setSemanticsEnabled(true);
    mAccessibilityNodeProvider.setAccessibilityEnabled(true);
  }

  public void resetAccessibilityTree() {
    if (mAccessibilityNodeProvider != null) {
      mAccessibilityNodeProvider.reset();
    }
  }
  //------- END ACCESSIBILITY ----

  //----- START FLUTTER INTEGRATION -----

  /**
   * Start rendering the UI for the given {@link FlutterRenderer}.
   *
   * @param flutterRenderer the FlutterRenderer for which this FlutterView will be a RenderSurface
   * @param pluginMessenger the BinaryMessenger to use with any plugins that this FlutterView needs
   *                        to register
   */
  public void attachToFlutterRenderer(@NonNull FlutterRenderer flutterRenderer, @NonNull BinaryMessenger pluginMessenger) {
    if (isAttachedToRenderer) {
      detachFromFlutterRenderer();
    }

    // TODO(mattcarroll): what should we do if we're not currently attached to the window?
    this.flutterRenderer = flutterRenderer;
    this.pluginMessenger = pluginMessenger;

    // Instruct our FlutterRenderer that we are now its designated RenderSurface.
    this.flutterRenderer.attachToRenderSurface(this);
    isAttachedToRenderer = true;

    // Configure the platform plugins and flutter channels.
    mFlutterLocalizationChannel = new MethodChannel(pluginMessenger, "flutter/localization", JSONMethodCodec.INSTANCE);
    mFlutterKeyEventChannel = new BasicMessageChannel<>(pluginMessenger, "flutter/keyevent", JSONMessageCodec.INSTANCE);
    mFlutterSettingsChannel = new BasicMessageChannel<>(pluginMessenger, "flutter/settings", JSONMessageCodec.INSTANCE);
    mTextInputPlugin = new TextInputPlugin(this, pluginMessenger);

    setFlutterLocale(getResources().getConfiguration().locale);
    setFlutterUserSettings();

    // Grab a reference to our underlying Surface and register callbacks with that Surface so we
    // can monitor changes and forward those changes on to native Flutter code.
    getHolder().addCallback(mSurfaceCallback);

    mIsSoftwareRenderingEnabled = flutterRenderer.isSoftwareRenderingEnabled();
  }

  private void assertAttachedToFlutterRenderer() {
    if (!isAttachedToRenderer) {
      throw new AssertionError("FlutterView is not attached to a FlutterRenderer.");
    }
  }

  /**
   * Stop rendering the UI for a given {@link FlutterRenderer}.
   *
   * If no {@link FlutterRenderer} is currently attached, this method does nothing.
   */
  public void detachFromFlutterRenderer() {
    if (!isAttachedToRenderer) {
      return;
    }

    // Stop forwarding messages from our underlying Surface to native Flutter code.
    getHolder().removeCallback(mSurfaceCallback);

    // Instruct our FlutterRenderer that we are no longer interested in being its RenderSurface.
    flutterRenderer.detachFromRenderSurface();
    flutterRenderer = null;
    isAttachedToRenderer = false;
  }

  /**
   * Send the given {@link Locale} configuration to Flutter.
   * @param locale the user's locale
   */
  private void setFlutterLocale(@NonNull Locale locale) {
    mFlutterLocalizationChannel.invokeMethod("setLocale", Arrays.asList(locale.getLanguage(), locale.getCountry()));
  }

  /**
   * Send various user preferences of this Android device to Flutter.
   *
   * For example, sends the user's "text scale factor" preferences, as well as the user's clock
   * format preference.
   */
  private void setFlutterUserSettings() {
    Map<String, Object> message = new HashMap<>();
    message.put("textScaleFactor", getResources().getConfiguration().fontScale);
    message.put("alwaysUse24HourFormat", DateFormat.is24HourFormat(getContext()));
    mFlutterSettingsChannel.send(message);
  }
  //----- END FLUTTER INTEGRATION -----

  //------ START RenderingSurface -----
  @Override
  public void updateCustomAccessibilityActions(ByteBuffer buffer, String[] strings) {
    try {
      if (mAccessibilityNodeProvider != null) {
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        mAccessibilityNodeProvider.updateCustomAccessibilityActions(buffer, strings);
      }
    } catch (Exception ex) {
      Log.e(TAG, "Uncaught exception while updating local context actions", ex);
    }
  }

  @Override
  public void updateSemantics(ByteBuffer buffer, String[] strings) {
    try {
      if (mAccessibilityNodeProvider != null) {
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        mAccessibilityNodeProvider.updateSemantics(buffer, strings);
      }
    } catch (Exception ex) {
      Log.e(TAG, "Uncaught exception while updating semantics", ex);
    }
  }

  @Override
  public void onFirstFrameRendered() {
    // no-op
  }
  //------ END RenderingSurface ----

  /// Must match the enum defined in window.dart.
  private enum AccessibilityFeature {
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

    // TODO(mattcarroll): getString() requires API 17. what should we do for earlier APIs?
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    @Override
    public void onChange(boolean selfChange, Uri uri) {
      String value = Settings.Global.getString(getContext().getContentResolver(),
          Settings.Global.TRANSITION_ANIMATION_SCALE);
      if (value.equals("0")) {
        mAccessibilityFeatureFlags ^= AccessibilityFeature.DISABLE_ANIMATIONS.value;
      } else {
        mAccessibilityFeatureFlags &= ~AccessibilityFeature.DISABLE_ANIMATIONS.value;
      }
      flutterRenderer.setAccessibilityFeatures(mAccessibilityFeatureFlags);
    }
  }

  // TODO: TouchExplorationStateChangeListener requires API 19. What do we do about earlier APIs?
  @SuppressLint("NewApi")
  class TouchExplorationListener implements AccessibilityManager.TouchExplorationStateChangeListener {
    @Override
    public void onTouchExplorationStateChanged(boolean enabled) {
      if (enabled) {
        mTouchExplorationEnabled = true;
        ensureAccessibilityEnabled();
        mAccessibilityFeatureFlags ^= AccessibilityFeature.ACCESSIBLE_NAVIGATION.value;
        flutterRenderer.setAccessibilityFeatures(mAccessibilityFeatureFlags);
      } else {
        mTouchExplorationEnabled = false;
        if (mAccessibilityNodeProvider != null) {
          mAccessibilityNodeProvider.handleTouchExplorationExit();
        }
        mAccessibilityFeatureFlags &= ~AccessibilityFeature.ACCESSIBLE_NAVIGATION.value;
        flutterRenderer.setAccessibilityFeatures(mAccessibilityFeatureFlags);
      }
      resetWillNotDraw();
    }
  }
}
