package io.flutter.embedding.engine.renderer;

import static org.junit.Assert.assertArrayEquals;
import static org.mockito.Matchers.anyFloat;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.graphics.Rect;
import android.view.Surface;
import io.flutter.embedding.engine.FlutterJNI;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterRendererTest {

  private FlutterJNI fakeFlutterJNI;
  private Surface fakeSurface;

  @Before
  public void setup() {
    fakeFlutterJNI = mock(FlutterJNI.class);
    fakeSurface = mock(Surface.class);
  }

  @Test
  public void itForwardsSurfaceCreationNotificationToFlutterJNI() {
    // Setup the test.
    Surface fakeSurface = mock(Surface.class);
    FlutterRenderer flutterRenderer = new FlutterRenderer(fakeFlutterJNI);

    // Execute the behavior under test.
    flutterRenderer.startRenderingToSurface(fakeSurface);

    // Verify the behavior under test.
    verify(fakeFlutterJNI, times(1)).onSurfaceCreated(eq(fakeSurface));
  }

  @Test
  public void itForwardsSurfaceChangeNotificationToFlutterJNI() {
    // Setup the test.
    Surface fakeSurface = mock(Surface.class);
    FlutterRenderer flutterRenderer = new FlutterRenderer(fakeFlutterJNI);

    flutterRenderer.startRenderingToSurface(fakeSurface);

    // Execute the behavior under test.
    flutterRenderer.surfaceChanged(100, 50);

    // Verify the behavior under test.
    verify(fakeFlutterJNI, times(1)).onSurfaceChanged(eq(100), eq(50));
  }

  @Test
  public void itForwardsSurfaceDestructionNotificationToFlutterJNI() {
    // Setup the test.
    Surface fakeSurface = mock(Surface.class);
    FlutterRenderer flutterRenderer = new FlutterRenderer(fakeFlutterJNI);

    flutterRenderer.startRenderingToSurface(fakeSurface);

    // Execute the behavior under test.
    flutterRenderer.stopRenderingToSurface();

    // Verify the behavior under test.
    verify(fakeFlutterJNI, times(1)).onSurfaceDestroyed();
  }

  @Test
  public void itStopsRenderingToOneSurfaceBeforeRenderingToANewSurface() {
    // Setup the test.
    Surface fakeSurface2 = mock(Surface.class);
    FlutterRenderer flutterRenderer = new FlutterRenderer(fakeFlutterJNI);

    flutterRenderer.startRenderingToSurface(fakeSurface);

    // Execute behavior under test.
    flutterRenderer.startRenderingToSurface(fakeSurface2);

    // Verify behavior under test.
    verify(fakeFlutterJNI, times(1)).onSurfaceDestroyed(); // notification of 1st surface's removal.
  }

  @Test
  public void itStopsRenderingToSurfaceWhenRequested() {
    // Setup the test.
    FlutterRenderer flutterRenderer = new FlutterRenderer(fakeFlutterJNI);

    flutterRenderer.startRenderingToSurface(fakeSurface);

    // Execute the behavior under test.
    flutterRenderer.stopRenderingToSurface();

    // Verify behavior under test.
    verify(fakeFlutterJNI, times(1)).onSurfaceDestroyed();
  }

  @Test
  public void itStopsSurfaceTextureCallbackWhenDetached() {
    // Setup the test.
    FlutterRenderer flutterRenderer = new FlutterRenderer(fakeFlutterJNI);

    fakeFlutterJNI.detachFromNativeAndReleaseResources();

    FlutterRenderer.SurfaceTextureRegistryEntry entry =
        (FlutterRenderer.SurfaceTextureRegistryEntry) flutterRenderer.createSurfaceTexture();

    flutterRenderer.startRenderingToSurface(fakeSurface);

    // Execute the behavior under test.
    flutterRenderer.stopRenderingToSurface();

    // Verify behavior under test.
    verify(fakeFlutterJNI, times(0)).markTextureFrameAvailable(eq(entry.id()));
  }

  @Test
  public void itConvertsDisplayFeatureArrayToPrimitiveArrays() {
    // Setup the test.
    FlutterRenderer flutterRenderer = new FlutterRenderer(fakeFlutterJNI);
    FlutterRenderer.ViewportMetrics metrics = new FlutterRenderer.ViewportMetrics();
    metrics.displayFeatures.add(
        new FlutterRenderer.DisplayFeature(
            new Rect(10, 20, 30, 40),
            FlutterRenderer.DisplayFeatureType.FOLD,
            FlutterRenderer.DisplayFeatureState.POSTURE_FLIPPED));
    metrics.displayFeatures.add(
        new FlutterRenderer.DisplayFeature(
            new Rect(50, 60, 70, 80), FlutterRenderer.DisplayFeatureType.CUTOUT));

    // Execute the behavior under test.
    flutterRenderer.setViewportMetrics(metrics);

    // Verify behavior under test.
    ArgumentCaptor<int[]> boundsCaptor = ArgumentCaptor.forClass(int[].class);
    ArgumentCaptor<int[]> typeCaptor = ArgumentCaptor.forClass(int[].class);
    ArgumentCaptor<int[]> stateCaptor = ArgumentCaptor.forClass(int[].class);
    verify(fakeFlutterJNI)
        .setViewportMetrics(
            anyFloat(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            anyInt(),
            boundsCaptor.capture(),
            typeCaptor.capture(),
            stateCaptor.capture());

    assertArrayEquals(new int[] {10, 20, 30, 40, 50, 60, 70, 80}, boundsCaptor.getValue());
    assertArrayEquals(
        new int[] {
          FlutterRenderer.DisplayFeatureType.FOLD.encodedValue,
          FlutterRenderer.DisplayFeatureType.CUTOUT.encodedValue
        },
        typeCaptor.getValue());
    assertArrayEquals(
        new int[] {
          FlutterRenderer.DisplayFeatureState.POSTURE_FLIPPED.encodedValue,
          FlutterRenderer.DisplayFeatureState.UNKNOWN.encodedValue
        },
        stateCaptor.getValue());
  }
}
