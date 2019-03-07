package io.flutter.embedding.android;

import android.os.Build;
import android.support.annotation.NonNull;
import android.view.InputDevice;
import android.view.MotionEvent;

import java.nio.ByteBuffer;
import java.util.*;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.engine.android.AndroidTouchConverter;

/**
 * Sends touch information from Android to Flutter in a format that Flutter
 * understands.
 */
public class AndroidTouchProcessor {

  // This value must match the value in framework's platform_view.dart.
  // This flag indicates whether the original Android pointer events were batched together.
  private static final int POINTER_DATA_FLAG_BATCHED = 1;

  @NonNull
  private final FlutterRenderer renderer;
  private final AndroidTouchConverter touchConverter;
  /**
   * Constructs an {@code AndroidTouchProcessor} that will send touch event data
   * to the Flutter execution context represented by the given {@link FlutterRenderer}.
   */
  // TODO(mattcarroll): consider moving packet behavior to a FlutterInteractionSurface instead of FlutterRenderer
  public AndroidTouchProcessor(@NonNull FlutterRenderer renderer) {
    this.renderer = renderer;
    touchConverter = new AndroidTouchConverter();
  }

  /**
   * Sends the given {@link MotionEvent} data to Flutter in a format that
   * Flutter understands.
   */
  public boolean onTouchEvent(MotionEvent event) {
    int pointerCount = event.getPointerCount();

    int maskedAction = event.getActionMasked();
    int pointerChange = getPointerChangeForAction(event.getActionMasked());
    boolean updateForSinglePointer = maskedAction == MotionEvent.ACTION_DOWN || maskedAction == MotionEvent.ACTION_POINTER_DOWN;
    boolean updateForMultiplePointers = !updateForSinglePointer && (maskedAction == MotionEvent.ACTION_UP || maskedAction == MotionEvent.ACTION_POINTER_UP);
    touchConverter.clearActions();

    if (updateForSinglePointer) {
      // ACTION_DOWN and ACTION_POINTER_DOWN always apply to a single pointer only.
      touchConverter.registerAction(event, event.getActionIndex(), pointerChange, 0);
    } else if (updateForMultiplePointers) {
      // ACTION_UP and ACTION_POINTER_UP may contain position updates for other pointers.
      // We are converting these updates to move events here in order to preserve this data.
      // We also mark these events with a flag in order to help the framework reassemble
      // the original Android event later, should it need to forward it to a PlatformView.
      for (int p = 0; p < pointerCount; p++) {
        if (p != event.getActionIndex() && event.getToolType(p) == MotionEvent.TOOL_TYPE_FINGER) {
          touchConverter.registerAction(event, p, AndroidTouchConverter.PointerChange.MOVE, POINTER_DATA_FLAG_BATCHED);
        }
      }
      // It's important that we're sending the UP event last. This allows PlatformView
      // to correctly batch everything back into the original Android event if needed.
      touchConverter.registerAction(event, event.getActionIndex(), pointerChange, 0);
    } else {
      // ACTION_MOVE may not actually mean all pointers have moved
      // but it's the responsibility of a later part of the system to
      // ignore 0-deltas if desired.
      for (int p = 0; p < pointerCount; p++) {
        touchConverter.registerAction(event, p, pointerChange, 0);
      }
    }

    // Prepare a data packet of the appropriate size and order.
    ByteBuffer packet = touchConverter.toFlutterPacket();

    touchConverter.clearActions();

    // Send the packet to Flutter.
    renderer.dispatchPointerDataPacket(packet, packet.position());

    return true;
  }

  /**
   * Sends the given generic {@link MotionEvent} data to Flutter in a format that Flutter
   * understands.
   *
   * Generic motion events include joystick movement, mouse hover, track pad touches, scroll wheel
   * movements, etc.
   */
  public boolean onGenericMotionEvent(MotionEvent event) {
    // Method isFromSource is only available in API 18+ (Jelly Bean MR2)
    // Mouse hover support is not implemented for API < 18.
    boolean isPointerEvent = Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2
        && event.isFromSource(InputDevice.SOURCE_CLASS_POINTER);
    if (!isPointerEvent || event.getActionMasked() != MotionEvent.ACTION_HOVER_MOVE) {
      return false;
    }

    touchConverter.clearActions();

    int pointerChange = getPointerChangeForAction(event.getActionMasked());

    // ACTION_HOVER_MOVE always applies to a single pointer only.
    touchConverter.registerAction(event, event.getActionIndex(), pointerChange, 0);

    ByteBuffer packet = touchConverter.toFlutterPacket();

    touchConverter.clearActions();

    renderer.dispatchPointerDataPacket(packet, packet.position());
    return true;
  }


  @AndroidTouchConverter.PointerChange
  private int getPointerChangeForAction(int maskedAction) {
    // Primary pointer:
    if (maskedAction == MotionEvent.ACTION_DOWN) {
      return AndroidTouchConverter.PointerChange.DOWN;
    }
    if (maskedAction == MotionEvent.ACTION_UP) {
      return AndroidTouchConverter.PointerChange.UP;
    }
    // Secondary pointer:
    if (maskedAction == MotionEvent.ACTION_POINTER_DOWN) {
      return AndroidTouchConverter.PointerChange.DOWN;
    }
    if (maskedAction == MotionEvent.ACTION_POINTER_UP) {
      return AndroidTouchConverter.PointerChange.UP;
    }
    // All pointers:
    if (maskedAction == MotionEvent.ACTION_MOVE) {
      return AndroidTouchConverter.PointerChange.MOVE;
    }
    if (maskedAction == MotionEvent.ACTION_HOVER_MOVE) {
      return AndroidTouchConverter.PointerChange.HOVER;
    }
    if (maskedAction == MotionEvent.ACTION_CANCEL) {
      return AndroidTouchConverter.PointerChange.CANCEL;
    }
    return -1;
  }
}
