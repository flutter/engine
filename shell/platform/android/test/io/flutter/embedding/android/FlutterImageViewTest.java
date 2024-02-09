package io.flutter.embedding.android;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.when;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Bitmap;
import android.hardware.HardwareBuffer;
import android.media.Image;
import android.media.Image.Plane;
import android.media.ImageReader;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(AndroidJUnit4.class)
@TargetApi(30)
public class FlutterImageViewTest {
  private final Context ctx = ApplicationProvider.getApplicationContext();

  @Test
  public void acquireLatestImage() {
    final ImageReader mockReader = mock(ImageReader.class);
    final Image mockImage = mock(Image.class);
    final HardwareBuffer mockHardwareBuffer = mock(HardwareBuffer.class);
    final Bitmap mockBitmap = mock(Bitmap.class);
    final FlutterImageView imageView =
        spy(new FlutterImageView(ctx, mockReader, FlutterImageView.SurfaceKind.background));

    when(mockReader.getMaxImages()).thenReturn(2);
    when(mockReader.acquireLatestImage()).thenReturn(mockImage);
    when(mockImage.getPlanes()).thenReturn(new Plane[0]);
    when(mockImage.getHardwareBuffer()).thenReturn(mockHardwareBuffer);
    when(mockHardwareBuffer.getUsage()).thenReturn(HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE);
    when(imageView.convertImageToBitmap(mockImage)).thenReturn(mockBitmap);

    final FlutterJNI jni = mock(FlutterJNI.class);
    imageView.attachToRenderer(new FlutterRenderer(jni));
    doNothing().when(imageView).invalidate();

    assertFalse(imageView.acquireLatestImage());

    // Simulate the next frame available.
    imageView.onImageAvailable(mockReader);
    assertTrue(imageView.acquireLatestImage());
  }
}
