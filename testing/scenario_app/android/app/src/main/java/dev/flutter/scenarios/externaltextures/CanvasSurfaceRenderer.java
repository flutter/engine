package dev.flutter.scenarios.externaltextures;

import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;
import android.os.Build;
import android.view.Surface;

import java.util.concurrent.CountDownLatch;

/**
 * Paints a simple gradient onto the attached Surface.
 */
public class CanvasSurfaceRenderer implements SurfaceRenderer {
    private Surface surface;
    private CountDownLatch onFirstFrame;

    @Override
    public void attach(Surface surface, CountDownLatch onFirstFrame) {
        this.surface = surface;
        this.onFirstFrame = onFirstFrame;
    }

    @Override
    public void repaint() {
        Canvas canvas =
                Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
                        ? surface.lockHardwareCanvas()
                        : surface.lockCanvas(null);
        Paint paint = new Paint();
        paint.setShader(
                new LinearGradient(
                        0.0f,
                        0.0f,
                        canvas.getWidth(),
                        canvas.getHeight(),
                        new int[]{
                                // Cyan (#00FFFF)
                                0xFF00FFFF,
                                // Magenta (#FF00FF)
                                0xFFFF00FF,
                                // Yellow (#FFFF00)
                                0xFFFFFF00,
                        },
                        null,
                        Shader.TileMode.REPEAT));
        canvas.drawPaint(paint);
        surface.unlockCanvasAndPost(canvas);

        if (onFirstFrame != null) {
            onFirstFrame.countDown();
            onFirstFrame = null;
        }
    }

    @Override
    public void destroy() {
        // Nothing to clean up, canvas rendering has no external resources.
    }
}
