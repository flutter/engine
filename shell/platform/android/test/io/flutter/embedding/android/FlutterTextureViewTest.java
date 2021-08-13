package test.io.flutter.embedding.android;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import android.annotation.TargetApi;
import android.view.Surface;
import android.view.TextureView;
import io.flutter.embedding.android.FlutterTextureView;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
@TargetApi(30)
public class FlutterTextureViewTest {
  @Test
  public void surfaceTextureListenerReleasesRenderer() {
    final FlutterTextureView flutterTextureView =
        new FlutterTextureView(RuntimeEnvironment.application);
    final Surface mockRenderSurface = mock(Surface.class);

    flutterTextureView.setRenderSurface(mockRenderSurface);

    final TextureView.SurfaceTextureListener listener = textureView.getSurfaceTextureListener();
    listener.onSurfaceTextureDestroyed(mock(Surface.class));

    verify(mockRenderSurface).release();
  }
}
