package io.flutter.embedding.android;

import android.os.Build;
import android.support.annotation.IntDef;
import android.support.annotation.NonNull;
import android.util.LongSparseArray;
import android.view.InputDevice;
import android.view.MotionEvent;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.*;
import io.flutter.embedding.engine.renderer.FlutterRenderer;

/**
 * convert touch information from Android to Flutter in a format that Flutter
 * understands.
 */
public class AndroidTouchConverter {

  // Must match the PointerChange enum in pointer.dart.
  @IntDef({
      PointerChange.CANCEL,
      PointerChange.ADD,
      PointerChange.REMOVE,
      PointerChange.HOVER,
      PointerChange.DOWN,
      PointerChange.MOVE,
      PointerChange.UP
  })
  public @interface PointerChange {
    int CANCEL = 0;
    int ADD = 1;
    int REMOVE = 2;
    int HOVER = 3;
    int DOWN = 4;
    int MOVE = 5;
    int UP = 6;
  }

  // Must match the PointerDeviceKind enum in pointer.dart.
  @IntDef({
      PointerDeviceKind.TOUCH,
      PointerDeviceKind.MOUSE,
      PointerDeviceKind.STYLUS,
      PointerDeviceKind.INVERTED_STYLUS,
      PointerDeviceKind.UNKNOWN
  })
  private @interface PointerDeviceKind {
    int TOUCH = 0;
    int MOUSE = 1;
    int STYLUS = 2;
    int INVERTED_STYLUS = 3;
    int UNKNOWN = 4;
  }

  // Must match the PointerSignalKind enum in pointer.dart.
  @IntDef({
      PointerSignalKind.NONE,
      PointerSignalKind.SCROLL,
      PointerSignalKind.UNKNOWN
  })
  private @interface PointerSignalKind {
    int NONE = 0;
    int SCROLL = 1;
    int UNKNOWN = 2;
  }

  // Must match the unpacking code in hooks.dart.
  private static final int POINTER_DATA_FIELD_COUNT = 24;
  private static final int BYTES_PER_FIELD = 8;

  private final LongSparseArray<PointerState> pointers;
  private List<FrameworkPointerEvent> eventQueue;
  /**
   * Constructs an {@code AndroidTouchConverter} that will convert touch event data to a set of
   # framework understandable events
   */
  public AndroidTouchConverter() {
    pointers = new LongSparseArray<PointerState>();
    eventQueue = new ArrayList<FrameworkPointerEvent>();
  }

  public void registerAction(MotionEvent event, int pointerIndex, int actionType, int batched) {
    PointerProperty pointerProperty = new PointerProperty(event, pointerIndex);
    long device = pointerProperty.device;
    PointerState state;
    // the android touch events do not always follow clear
    // guideline. We normalize the events so that the framework
    // does not have to deal with android specific issue.
    switch (actionType) {
      case PointerChange.CANCEL:
      case PointerChange.UP:
        // Some android phones will send a cancel event to non-existent pointer.
        // ie, xiaomi one plus three points screenshot gesture.
        state = pointers.get(device);
        if (state != null) {
          if (!state.isLastLocation(pointerProperty.dx, pointerProperty.dy)) {
            eventQueue.add(new FrameworkPointerEvent(
              new PointerProperty(pointerProperty),
              PointerChange.MOVE,
              batched
            ));
            state.setLastLocation(pointerProperty.dx, pointerProperty.dy);
          }
          state.setUp();
          eventQueue.add(new FrameworkPointerEvent(
            new PointerProperty(pointerProperty),
            actionType,
            batched
          ));
        }
        break;
      case PointerChange.ADD:
        state = pointers.get(device);
        if (state != null) {
          state.setLastLocation(pointerProperty.dx, pointerProperty.dy);
        } else {
            state = new PointerState(device, pointerProperty.dx, pointerProperty.dy);
          pointers.put(device, state);
        }
        eventQueue.add(new FrameworkPointerEvent(
          new PointerProperty(pointerProperty),
          actionType,
          batched
        ));
        break;
      case PointerChange.REMOVE:
        state = pointers.get(device);
        if (state != null) {
          if (state.isDown()) {
            eventQueue.add(new FrameworkPointerEvent(
              new PointerProperty(pointerProperty),
              PointerChange.CANCEL,
              batched
            ));
          }
          eventQueue.add(new FrameworkPointerEvent(
            new PointerProperty(pointerProperty),
            actionType,
            batched
          ));
          pointers.remove(device);
        }
        break;
      case PointerChange.HOVER:
        state = pointers.get(device);
        if (state == null) {
          state = new PointerState(device, pointerProperty.dx, pointerProperty.dy);
          pointers.put(device, state);
          eventQueue.add(new FrameworkPointerEvent(
            new PointerProperty(pointerProperty),
            PointerChange.ADD,
            batched
          ));
        }
        eventQueue.add(new FrameworkPointerEvent(
          new PointerProperty(pointerProperty),
          actionType,
          batched
        ));
        state.setLastLocation(pointerProperty.dx, pointerProperty.dy);
        break;
      case PointerChange.DOWN:
        state = pointers.get(device);
        if (state == null) {
          state = new PointerState(device, pointerProperty.dx, pointerProperty.dy);
          pointers.put(device, state);
          eventQueue.add(new FrameworkPointerEvent(
            new PointerProperty(pointerProperty),
            PointerChange.ADD,
            batched
          ));
        }
        state = pointers.get(device);
        if (!state.isLastLocation(pointerProperty.dx, pointerProperty.dy)) {
          eventQueue.add(new FrameworkPointerEvent(
            new PointerProperty(pointerProperty),
            PointerChange.HOVER,
            batched
          ));
          state.setLastLocation(pointerProperty.dx, pointerProperty.dy);
        }
        eventQueue.add(new FrameworkPointerEvent(
          new PointerProperty(pointerProperty),
          actionType,
          batched
        ));
        state.setDown();
        break;
      case PointerChange.MOVE:
        eventQueue.add(new FrameworkPointerEvent(
          new PointerProperty(pointerProperty),
          actionType,
          batched
        ));
        state = pointers.get(device);
        state.setLastLocation(pointerProperty.dx, pointerProperty.dy);
        break;
      default:
        // unknown action
        throw new AssertionError("Unknown Touch event.");

    }
    return;
  }

  public void clearActions(){
    eventQueue = new ArrayList<FrameworkPointerEvent>();
  }

  public ByteBuffer toFlutterPacket() {
    // Prepare a data packet of the appropriate size and order.
    ByteBuffer packet = ByteBuffer.allocateDirect(eventQueue.size() * POINTER_DATA_FIELD_COUNT * BYTES_PER_FIELD);
    packet.order(ByteOrder.LITTLE_ENDIAN);

    for (FrameworkPointerEvent event : eventQueue) {
      addToPacket(event, packet);
    }

    if (packet.position() % (POINTER_DATA_FIELD_COUNT * BYTES_PER_FIELD) != 0) {
      throw new AssertionError("Packet position is not on field boundary");
    }

    return packet;
  }

  private void addToPacket(
      FrameworkPointerEvent event,
      ByteBuffer packet
  ) {
    if (event.actionType == -1) {
      return;
    }

    int pointerKind = event.property.pointerKind;

    packet.putLong(event.property.timeStamp); // time_stamp
    packet.putLong(event.actionType); // change
    packet.putLong(pointerKind); // kind
    packet.putLong(PointerSignalKind.NONE); // signal_kind
    packet.putLong(event.property.device); // device
    packet.putDouble(event.property.dx); // physical_x
    packet.putDouble(event.property.dy); // physical_y

    if (pointerKind == PointerDeviceKind.MOUSE) {
      packet.putLong(event.property.buttonState & 0x1F); // buttons
    } else if (pointerKind == PointerDeviceKind.STYLUS) {
      packet.putLong((event.property.buttonState >> 4) & 0xF); // buttons
    } else {
      packet.putLong(0); // buttons
    }

    packet.putLong(0); // obscured

    packet.putDouble(event.property.pressure); // pressure
    double pressureMin = 0.0;
    double pressureMax = 1.0;
    if (event.property.inputDevice != null) {
      InputDevice.MotionRange pressureRange = event.property.inputDevice.getMotionRange(MotionEvent.AXIS_PRESSURE);
      if (pressureRange != null) {
        pressureMin = pressureRange.getMin();
        pressureMax = pressureRange.getMax();
      }
    }
    packet.putDouble(pressureMin); // pressure_min
    packet.putDouble(pressureMax); // pressure_max

    if (pointerKind == PointerDeviceKind.STYLUS) {
      packet.putDouble(event.property.distance); // distance
      packet.putDouble(0.0); // distance_max
    } else {
      packet.putDouble(0.0); // distance
      packet.putDouble(0.0); // distance_max
    }

    packet.putDouble(event.property.size); // size

    packet.putDouble(event.property.radiusMajor); // radius_major
    packet.putDouble(event.property.radiusMinor); // radius_minor

    packet.putDouble(0.0); // radius_min
    packet.putDouble(0.0); // radius_max

    packet.putDouble(event.property.orientation); // orientation

    if (pointerKind == PointerDeviceKind.STYLUS) {
      packet.putDouble(event.property.tilt); // tilt
    } else {
      packet.putDouble(0.0); // tilt
    }

    packet.putLong(event.batched); // platformData

    packet.putDouble(0.0); // scroll_delta_x
    packet.putDouble(0.0); // scroll_delta_y
  }


  @PointerDeviceKind
  private int getPointerDeviceTypeForToolType(int toolType) {
    switch (toolType) {
      case MotionEvent.TOOL_TYPE_FINGER:
        return PointerDeviceKind.TOUCH;
      case MotionEvent.TOOL_TYPE_STYLUS:
        return PointerDeviceKind.STYLUS;
      case MotionEvent.TOOL_TYPE_MOUSE:
        return PointerDeviceKind.MOUSE;
      case MotionEvent.TOOL_TYPE_ERASER:
        return PointerDeviceKind.INVERTED_STYLUS;
      default:
        // MotionEvent.TOOL_TYPE_UNKNOWN will reach here.
        return PointerDeviceKind.UNKNOWN;
    }
  }

  private class PointerState {
    private long device;
    private double dx;
    private double dy;
    private boolean down;

    public PointerState(long device, double dx, double dy) {
      this.device = device;
      this.dx = dx;
      this.dy = dy;
      this.down = false;
    }

    public boolean isLastLocation(double dx, double dy) {
      return this.dx == dx && this.dy == dy;
    }

    public void setLastLocation (double dx, double dy) {
      this.dx = dx;
      this.dy = dy;
    }

    public boolean isDown() {
      return down;
    }

    public void setUp() {
      down = false;
    }

    public void setDown() {
      down = true;
    }
  }

  private class FrameworkPointerEvent {

    public final PointerProperty property;
    public final int actionType;
    public final int batched;
    
    public FrameworkPointerEvent(
      PointerProperty property,
      int actionType,
      int batched
    ) {
      this.property = property;
      this.actionType = actionType;
      this.batched = batched;
    }
  }

  private class PointerProperty {

    public final InputDevice inputDevice;
    public final long device;
    public final double dx;
    public final double dy;
    public final int pointerKind;
    public final int buttonState;
    public final long timeStamp;
    public final float pressure;
    public final float distance;
    public final float size;
    public final float radiusMajor;
    public final float radiusMinor;
    public final float orientation;
    public final float tilt;

    public PointerProperty(MotionEvent event, int pointerIndex){
      this(
        event.getDevice(),
        event.getPointerId(pointerIndex),
        event.getX(pointerIndex),
        event.getY(pointerIndex),
        getPointerDeviceTypeForToolType(event.getToolType(pointerIndex)),
        event.getButtonState(),
        event.getEventTime() * 1000,
        event.getPressure(pointerIndex),
        event.getAxisValue(MotionEvent.AXIS_DISTANCE, pointerIndex),
        event.getSize(pointerIndex),
        event.getToolMajor(pointerIndex),
        event.getToolMinor(pointerIndex),
        event.getAxisValue(MotionEvent.AXIS_ORIENTATION, pointerIndex),
        event.getAxisValue(MotionEvent.AXIS_TILT, pointerIndex)
      );
    }

    public PointerProperty(PointerProperty source){
      this(
        source.inputDevice,
        source.device,
        source.dx,
        source.dy,
        source.pointerKind,
        source.buttonState,
        source.timeStamp,
        source.pressure,
        source.distance,
        source.size,
        source.radiusMajor,
        source.radiusMinor,
        source.orientation,
        source.tilt
      );
    }

    public PointerProperty(
      InputDevice inputDevice,
      long device,
      double dx,
      double dy,
      int pointerKind,
      int buttonState,
      long timeStamp,
      float pressure,
      float distance,
      float size,
      float radiusMajor,
      float radiusMinor,
      float orientation,
      float tilt
    ) {
      this.inputDevice = inputDevice;
      this.device = device;
      this.dx = dx;
      this.dy = dy;
      this.pointerKind = pointerKind;
      this.buttonState = buttonState;
      this.timeStamp = timeStamp;
      this.pressure = pressure;
      this.distance = distance;
      this.size = size;
      this.radiusMajor = radiusMajor;
      this.radiusMinor = radiusMinor;
      this.orientation = orientation;
      this.tilt = tilt;

    }
  }
}
