package test.io.flutter.embedding.android;

import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.mock;

import org.robolectric.annotation.Config;
import org.robolectric.RobolectricTestRunner;
import org.junit.Test;
import org.junit.runner.RunWith;
import android.annotation.TargetApi;
import android.view.TextureView;
import android.view.Surface;
import io.flutter.embedding.android.FlutterTextureView;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
@TargetApi(30)
public class FlutterTextureViewTest {
  @Test
  public void surfaceTextureListenerReleasesRenderer() {
    final FlutterTextureView flutterTextureView = new FlutterTextureView(RuntimeEnvironment.application);
    final Surface mockRenderSurface = mock(Surface.class);

    flutterTextureView.setRenderSurface(mockRenderSurface);

    final TextureView.SurfaceTextureListener listener = textureView.getSurfaceTextureListener();
    listener.onSurfaceTextureDestroyed(mock(Surface.class));

    verify(mockRenderSurface).release();
  }
}
