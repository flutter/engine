package io.flutter.embedding.android;

import static android.view.MotionEvent.PointerCoords;
import static android.view.MotionEvent.PointerProperties;
import static junit.framework.TestCase.assertEquals;
import static org.mockito.Mockito.inOrder;

import android.annotation.TargetApi;
import android.view.InputDevice;
import android.view.MotionEvent;
import androidx.test.core.view.MotionEventBuilder;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import java.nio.ByteBuffer;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.InOrder;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(AndroidJUnit4.class)
@TargetApi(28)
public class AndroidTouchProcessorTest {
  @Mock FlutterRenderer mockRenderer;
  AndroidTouchProcessor touchProcessor;
  @Captor ArgumentCaptor<ByteBuffer> packetCaptor;
  @Captor ArgumentCaptor<Integer> packetSizeCaptor;

  @Before
  public void setUp() {
    MockitoAnnotations.openMocks(this);
    touchProcessor = new AndroidTouchProcessor(mockRenderer, false);
  }

  private long readPointerChange(ByteBuffer buffer) {
    return buffer.getLong(2 * AndroidTouchProcessor.BYTES_PER_FIELD);
  }

  private long readPointerDeviceKind(ByteBuffer buffer) {
    return buffer.getLong(3 * AndroidTouchProcessor.BYTES_PER_FIELD);
  }

  private long readPointerSignalKind(ByteBuffer buffer) {
    return buffer.getLong(4 * AndroidTouchProcessor.BYTES_PER_FIELD);
  }

  private double readPointerPhysicalX(ByteBuffer buffer) {
    return buffer.getDouble(7 * AndroidTouchProcessor.BYTES_PER_FIELD);
  }

  private double readPointerPhysicalY(ByteBuffer buffer) {
    return buffer.getDouble(8 * AndroidTouchProcessor.BYTES_PER_FIELD);
  }

  private double readPointerPanX(ByteBuffer buffer) {
    return buffer.getDouble(29 * AndroidTouchProcessor.BYTES_PER_FIELD);
  }

  private double readPointerPanY(ByteBuffer buffer) {
    return buffer.getDouble(30 * AndroidTouchProcessor.BYTES_PER_FIELD);
  }

  @Test
  public void normalTouch() {
    PointerProperties properties = new PointerProperties();
    properties.id = 0;
    properties.toolType = MotionEvent.TOOL_TYPE_FINGER;
    PointerCoords coordinates = new PointerCoords();
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_DOWN)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .build());
    InOrder inOrder = inOrder(mockRenderer);
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    ByteBuffer packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.DOWN, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.TOUCH, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(0.0, readPointerPhysicalX(packet));
    assertEquals(0.0, readPointerPhysicalY(packet));
    coordinates.x = 10;
    coordinates.y = 5;
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_MOVE)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .build());
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.MOVE, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.TOUCH, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(10.0, readPointerPhysicalX(packet));
    assertEquals(5.0, readPointerPhysicalY(packet));
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_UP)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .build());
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.UP, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.TOUCH, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(10.0, readPointerPhysicalX(packet));
    assertEquals(5.0, readPointerPhysicalY(packet));
    inOrder.verifyNoMoreInteractions();
  }

  @Test
  public void trackpadGesture() {
    PointerProperties properties = new PointerProperties();
    properties.id = 0;
    properties.toolType = MotionEvent.TOOL_TYPE_MOUSE;
    PointerCoords coordinates = new PointerCoords();
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_DOWN)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .setSource(InputDevice.SOURCE_MOUSE)
            .build());
    InOrder inOrder = inOrder(mockRenderer);
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    ByteBuffer packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.PAN_ZOOM_START, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.TRACKPAD, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(0.0, readPointerPhysicalX(packet));
    assertEquals(0.0, readPointerPhysicalY(packet));
    coordinates.x = 10;
    coordinates.y = 5;
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_MOVE)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .setSource(InputDevice.SOURCE_MOUSE)
            .build());
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.PAN_ZOOM_UPDATE, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.TRACKPAD, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(0.0, readPointerPhysicalX(packet));
    assertEquals(0.0, readPointerPhysicalY(packet));
    assertEquals(10.0, readPointerPanX(packet));
    assertEquals(5.0, readPointerPanY(packet));
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_UP)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .setSource(InputDevice.SOURCE_MOUSE)
            .build());
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.PAN_ZOOM_END, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.TRACKPAD, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(0.0, readPointerPhysicalX(packet));
    assertEquals(0.0, readPointerPhysicalY(packet));
    inOrder.verifyNoMoreInteractions();
  }

  @Test
  public void mouse() {
    PointerProperties properties = new PointerProperties();
    properties.id = 0;
    properties.toolType = MotionEvent.TOOL_TYPE_MOUSE;
    PointerCoords coordinates = new PointerCoords();
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_DOWN)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .build());
    InOrder inOrder = inOrder(mockRenderer);
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    ByteBuffer packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.DOWN, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.MOUSE, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(0.0, readPointerPhysicalX(packet));
    assertEquals(0.0, readPointerPhysicalY(packet));
    coordinates.x = 10;
    coordinates.y = 5;
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_MOVE)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .build());
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.MOVE, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.MOUSE, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(10.0, readPointerPhysicalX(packet));
    assertEquals(5.0, readPointerPhysicalY(packet));
    touchProcessor.onTouchEvent(
        MotionEventBuilder.newBuilder()
            .setAction(MotionEvent.ACTION_UP)
            .setActionIndex(0)
            .setDeviceId(1)
            .setPointer(properties, coordinates)
            .build());
    inOrder
        .verify(mockRenderer)
        .dispatchPointerDataPacket(packetCaptor.capture(), packetSizeCaptor.capture());
    packet = packetCaptor.getValue();
    assertEquals(AndroidTouchProcessor.PointerChange.UP, readPointerChange(packet));
    assertEquals(AndroidTouchProcessor.PointerDeviceKind.MOUSE, readPointerDeviceKind(packet));
    assertEquals(AndroidTouchProcessor.PointerSignalKind.NONE, readPointerSignalKind(packet));
    assertEquals(10.0, readPointerPhysicalX(packet));
    assertEquals(5.0, readPointerPhysicalY(packet));
    inOrder.verifyNoMoreInteractions();
  }
}
