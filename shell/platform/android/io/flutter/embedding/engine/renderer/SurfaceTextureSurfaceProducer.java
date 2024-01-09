package io.flutter.embedding.engine.renderer;

import android.graphics.SurfaceTexture;
import android.media.Image;
import android.view.Surface;
import androidx.annotation.NonNull;
import io.flutter.view.TextureRegistry;

// An incomplete implementation of TextureRegistry.SurfaceProducer.
//
// TODO(https://github.com/flutter/flutter/issues/139702).
final class SurfaceTextureSurfaceProducer
    implements TextureRegistry.SurfaceProducer, TextureRegistry.ImageConsumer {
  private final long id;

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

  @NonNull
  SurfaceTexture getSurfaceTexture() {
    return texture;
  }

  @Override
  public void setSize(int width, int height) {
    getSurfaceTexture().setDefaultBufferSize(width, height);
  }

  @Override
  public int getWidth() {
    return 0;
  }

  @Override
  public int getHeight() {
    return 0;
  }

  @Override
  public Surface getSurface() {
    return new Surface(getSurfaceTexture());
  }

  @Override
  public Image acquireLatestImage() {
    return null;
  }
}
