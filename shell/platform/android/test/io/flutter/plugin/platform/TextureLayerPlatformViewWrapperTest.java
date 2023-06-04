package io.flutter.plugin.platform;

import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.*;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.View;
import androidx.annotation.NonNull;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;
import org.robolectric.annotation.Implements;

@TargetApi(31)
@RunWith(AndroidJUnit4.class)
public class TextureLayerPlatformViewWrapperTest {
  private final Context ctx = ApplicationProvider.getApplicationContext();

  @Test
  public void setTexture_writesToBuffer() {
    final Surface surface = mock(Surface.class);
    final TextureLayerPlatformViewWrapper wrapper =
        new TextureLayerPlatformViewWrapper(ctx) {
          @Override
          protected Surface createSurface(@NonNull SurfaceTexture tx) {
            return surface;
          }
        };

    final SurfaceTexture tx = mock(SurfaceTexture.class);
    when(tx.isReleased()).thenReturn(false);

    final Canvas canvas = mock(Canvas.class);
    when(surface.lockHardwareCanvas()).thenReturn(canvas);

    // Test.
    wrapper.setTexture(tx);

    // Verify.
    verify(surface, times(1)).lockHardwareCanvas();
    verify(surface, times(1)).unlockCanvasAndPost(canvas);
    verify(canvas, times(1)).drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
    verifyNoMoreInteractions(surface);
    verifyNoMoreInteractions(canvas);
  }

  @Test
  public void draw_writesToBuffer() {
    final Surface surface = mock(Surface.class);
    final TextureLayerPlatformViewWrapper wrapper =
        new TextureLayerPlatformViewWrapper(ctx) {
          @Override
          protected Surface createSurface(@NonNull SurfaceTexture tx) {
            return surface;
          }
        };

    wrapper.addView(
        new View(ctx) {
          @Override
          public void draw(Canvas canvas) {
            super.draw(canvas);
            canvas.drawColor(Color.RED);
          }
        });

    final int size = 100;
    wrapper.measure(size, size);
    wrapper.layout(0, 0, size, size);

    final SurfaceTexture tx = mock(SurfaceTexture.class);
    when(tx.isReleased()).thenReturn(false);

    when(surface.lockHardwareCanvas()).thenReturn(mock(Canvas.class));

    wrapper.setTexture(tx);

    reset(surface);

    final Canvas canvas = mock(Canvas.class);
    when(surface.lockHardwareCanvas()).thenReturn(canvas);
    when(surface.isValid()).thenReturn(true);

    // Test.
    wrapper.invalidate();
    wrapper.draw(new Canvas());

    // Verify.
    verify(canvas, times(1)).drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
    verify(surface, times(1)).isValid();
    verify(surface, times(1)).lockHardwareCanvas();
    verify(surface, times(1)).unlockCanvasAndPost(canvas);
    verifyNoMoreInteractions(surface);
  }

  @Test
  @Config(
      shadows = {
        ShadowView.class,
      })
  public void draw_withoutSurface() {
    final TextureLayerPlatformViewWrapper wrapper =
        new TextureLayerPlatformViewWrapper(ctx) {
          @Override
          public void onDraw(Canvas canvas) {
            canvas.drawColor(Color.RED);
          }
        };
    // Test.
    final Canvas canvas = mock(Canvas.class);
    wrapper.draw(canvas);

    // Verify.
    verify(canvas, times(1)).drawColor(Color.RED);
  }

  @Test
  public void release() {
    final Surface surface = mock(Surface.class);
    final TextureLayerPlatformViewWrapper wrapper =
        new TextureLayerPlatformViewWrapper(ctx) {
          @Override
          protected Surface createSurface(@NonNull SurfaceTexture tx) {
            return surface;
          }
        };

    final SurfaceTexture tx = mock(SurfaceTexture.class);
    when(tx.isReleased()).thenReturn(false);

    final Canvas canvas = mock(Canvas.class);
    when(surface.lockHardwareCanvas()).thenReturn(canvas);

    wrapper.setTexture(tx);
    reset(surface);
    reset(tx);

    // Test.
    wrapper.release();

    // Verify.
    verify(surface, times(1)).release();
    verifyNoMoreInteractions(surface);
    verifyNoMoreInteractions(tx);
  }

  @Implements(View.class)
  public static class ShadowView {}
}
