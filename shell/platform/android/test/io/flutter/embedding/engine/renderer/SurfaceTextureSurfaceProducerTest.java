package io.flutter.embedding.engine.renderer;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceView;

import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public final class SurfaceTextureSurfaceProducerTest {
    private final Context ctx = ApplicationProvider.getApplicationContext();

    @Test
    public void createsSurfaceTextureOfGivenSizeAndResizesWhenRequested() {
        // Create a surface and set the initial size.
        final SurfaceTextureSurfaceProducer producer = new SurfaceTextureSurfaceProducer(0);
        final Surface surface = producer.getSurface();
        producer.setSize(100, 200);

        // TODO: Finish implementing.
    }
}
