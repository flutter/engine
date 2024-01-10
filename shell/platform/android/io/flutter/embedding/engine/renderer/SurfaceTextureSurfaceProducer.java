package io.flutter.embedding.engine.renderer;

import android.graphics.SurfaceTexture;
import android.view.Surface;
import androidx.annotation.NonNull;
import io.flutter.view.TextureRegistry;

/** Uses a {@link android.graphics.SurfaceTexture} to populate the texture registry. */
final class SurfaceTextureSurfaceProducer
    implements TextureRegistry.SurfaceProducer, TextureRegistry.GLTextureConsumer {
  private final long id;
  private int requestBufferWidth;
  private int requestedBufferHeight;

  @NonNull private final SurfaceTexture texture;

  SurfaceTextureSurfaceProducer(long id) {
    this.id = id;
    this.texture = new SurfaceTexture(0);
  }

  @Override
  public long id() {
    return id;
  }

  @Override
  public void release() {
    texture.release();
  }

  @Override
  @NonNull
  public SurfaceTexture getSurfaceTexture() {
    return texture;
  }

  @Override
  public void setSize(int width, int height) {
    requestBufferWidth = width;
    requestedBufferHeight = height;
    getSurfaceTexture().setDefaultBufferSize(width, height);
  }

  @Override
  public int getWidth() {
    return requestBufferWidth;
  }

  @Override
  public int getHeight() {
    return requestedBufferHeight;
  }

  @Override
  public Surface getSurface() {
    return new Surface(getSurfaceTexture());
  }
}
