package io.flutter.embedding.engine.renderer;

import static org.junit.Assert.assertEquals;
import static org.robolectric.Shadows.shadowOf;

import android.content.Context;
import android.graphics.Canvas;
import android.os.Looper;
import android.view.Surface;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.concurrent.atomic.AtomicInteger;

@RunWith(AndroidJUnit4.class)
public final class SurfaceTextureSurfaceProducerTest {
  private final Context ctx = ApplicationProvider.getApplicationContext();

  @Test
  public void createsSurfaceTextureOfGivenSizeAndResizesWhenRequested() {
    // Create a surface and set the initial size.
    final SurfaceTextureSurfaceProducer producer = new SurfaceTextureSurfaceProducer(0);
    final Surface surface = producer.getSurface();
    AtomicInteger frames = new AtomicInteger();
    producer.getSurfaceTexture().setOnFrameAvailableListener((texture) -> {
      if (texture.isReleased()) {
        return;
      }
      frames.getAndIncrement();
    });
    producer.setSize(100, 200);

    // Draw.
    Canvas canvas = surface.lockHardwareCanvas();
    canvas.drawARGB(255, 255, 0, 0);
    surface.unlockCanvasAndPost(canvas);
    shadowOf(Looper.getMainLooper()).idle();
    assertEquals(frames.get(), 1);

    // Resize and redraw.
    producer.setSize(400, 800);
    canvas = surface.lockHardwareCanvas();
    canvas.drawARGB(255, 255, 0, 0);
    surface.unlockCanvasAndPost(canvas);
    shadowOf(Looper.getMainLooper()).idle();
    assertEquals(frames.get(), 2);

    // Done.
    producer.release();
  }
}
