package dev.flutter.scenarios.externaltextures;

import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.hardware.HardwareBuffer;
import android.media.Image;
import android.media.ImageReader;
import android.media.ImageWriter;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.Surface;

import androidx.annotation.RequiresApi;

import java.util.concurrent.CountDownLatch;

import dev.flutter.scenarios.ExternalTextureFlutterActivity;

/**
 * Takes frames from the inner SurfaceRenderer and feeds it through an ImageReader and ImageWriter
 * pair.
 */
@RequiresApi(Build.VERSION_CODES.M)
public class ImageSurfaceRenderer implements SurfaceRenderer {
    private final SurfaceRenderer inner;
    private final Rect crop;

    private CountDownLatch onFirstFrame;
    private ImageReader reader;
    private ImageWriter writer;

    private Handler handler;
    private HandlerThread handlerThread;

    private boolean canReadImage = true;
    private boolean canWriteImage = true;

    private final int surfaceWidth;
    private final int surfaceHeight;

    public ImageSurfaceRenderer(SurfaceRenderer inner, Rect crop, int surfaceWidth, int surfaceHeight) {
        this.inner = inner;
        this.crop = crop;
        this.surfaceWidth = surfaceWidth;
        this.surfaceHeight = surfaceHeight;
    }

    @Override
    public void attach(Surface surface, CountDownLatch onFirstFrame) {
        this.onFirstFrame = onFirstFrame;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            // On Android Q+, use PRIVATE image format.
            // Also let the frame producer know the images will
            // be sampled from by the GPU.
            writer = ImageWriter.newInstance(surface, 3, ImageFormat.PRIVATE);
            reader =
                    ImageReader.newInstance(
                            surfaceWidth,
                            surfaceHeight,
                            ImageFormat.PRIVATE,
                            2,
                            HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE);
        } else {
            // Before Android Q, this will change the format of the surface to match the images.
            writer = ImageWriter.newInstance(surface, 3);
            reader = ImageReader.newInstance(surfaceWidth, surfaceHeight, writer.getFormat(), 2);
        }
        inner.attach(reader.getSurface(), null);

        handlerThread = new HandlerThread("image reader/writer thread");
        handlerThread.start();

        handler = new Handler(handlerThread.getLooper());
        reader.setOnImageAvailableListener(this::onImageAvailable, handler);
        writer.setOnImageReleasedListener(this::onImageReleased, handler);
    }

    private void onImageAvailable(ImageReader reader) {
        Log.v("Scenarios", "Image available");

        if (!canWriteImage) {
            // If the ImageWriter hasn't released the latest image, don't attempt to enqueue another
            // image.
            // Otherwise the reader writer pair locks up if the writer runs behind, as the reader runs
            // out of images and the writer has no more space for images.
            canReadImage = true;
            return;
        }

        canReadImage = false;
        Image image = reader.acquireLatestImage();
        image.setCropRect(crop);
        try {
            canWriteImage = false;
            writer.queueInputImage(image);
        } catch (IllegalStateException e) {
            // If the output surface disconnects, this method will be interrupted with an
            // IllegalStateException.
            // Simply log and return.
            Log.i("Scenarios", "Surface disconnected from ImageWriter", e);
            image.close();
        }

        Log.v("Scenarios", "Output image");

        if (onFirstFrame != null) {
            onFirstFrame.countDown();
            onFirstFrame = null;
        }
    }

    private void tryAcquireImage() {
        if (canReadImage) {
            onImageAvailable(reader);
        }
    }

    private void onImageReleased(ImageWriter imageWriter) {
        Log.v("Scenarios", "Image released");

        if (!canWriteImage) {
            canWriteImage = true;
            if (canReadImage) {
                // Try acquire the image in a handler message, as we may have another call to
                // onImageAvailable in the thread's message queue.
                handler.post(this::tryAcquireImage);
            }
        }
    }

    @Override
    public void repaint() {
        inner.repaint();
    }

    @Override
    public void destroy() {
        Log.i("Scenarios", "Destroying ImageSurfaceRenderer");
        inner.destroy();
        handler.post(this::destroyReaderWriter);
    }

    private void destroyReaderWriter() {
        writer.close();
        Log.i("Scenarios", "ImageWriter destroyed");
        reader.close();
        Log.i("Scenarios", "ImageReader destroyed");
        handlerThread.quitSafely();
    }
}
