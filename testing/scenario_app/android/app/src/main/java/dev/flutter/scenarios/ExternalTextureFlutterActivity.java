// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package dev.flutter.scenarios;

import android.content.res.AssetFileDescriptor;
import android.graphics.SurfaceTexture;
import android.media.MediaExtractor;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.view.Gravity;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.FrameLayout.LayoutParams;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import dev.flutter.scenarios.externaltextures.CanvasSurfaceRenderer;
import dev.flutter.scenarios.externaltextures.ImageSurfaceRenderer;
import dev.flutter.scenarios.externaltextures.MediaSurfaceRenderer;
import dev.flutter.scenarios.externaltextures.SurfaceRenderer;
import io.flutter.view.TextureRegistry.SurfaceTextureEntry;
import java.io.IOException;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.CountDownLatch;

public class ExternalTextureFlutterActivity extends TestActivity {
  static final String TAG = "Scenarios";
  private static final int SURFACE_WIDTH = 192;
  private static final int SURFACE_HEIGHT = 256;

  private SurfaceRenderer surfaceViewRenderer, flutterRenderer;

  // Latch used to ensure both SurfaceRenderers produce a frame before taking a screenshot.
  private final CountDownLatch firstFrameLatch = new CountDownLatch(2);

  private long textureId = 0;
  private SurfaceTextureEntry surfaceTextureEntry;

  @Override
  protected void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    String surfaceRenderer = getIntent().getStringExtra("surface_renderer");
    assert surfaceRenderer != null;
    surfaceViewRenderer = selectSurfaceRenderer(surfaceRenderer, getIntent().getExtras());
    flutterRenderer = selectSurfaceRenderer(surfaceRenderer, getIntent().getExtras());

    // Create and place a SurfaceView above the Flutter content.
    SurfaceView surfaceView = new SurfaceView(getContext());
    surfaceView.setZOrderMediaOverlay(true);
    surfaceView.setMinimumWidth(SURFACE_WIDTH);
    surfaceView.setMinimumHeight(SURFACE_HEIGHT);

    FrameLayout frameLayout = new FrameLayout(getContext());
    frameLayout.addView(
        surfaceView,
        new LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT,
            Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL));

    addContentView(
        frameLayout,
        new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

    SurfaceHolder surfaceHolder = surfaceView.getHolder();
    surfaceHolder.setFixedSize(SURFACE_WIDTH, SURFACE_HEIGHT);
    surfaceHolder.addCallback(new SurfaceRendererCallback(surfaceViewRenderer, firstFrameLatch));
  }

  @Override
  public void waitUntilFlutterRendered() {
    super.waitUntilFlutterRendered();

    try {
      firstFrameLatch.await();
    } catch (InterruptedException e) {
      throw new RuntimeException(e);
    }
  }

  private SurfaceRenderer selectSurfaceRenderer(String surfaceRenderer, Bundle extras) {
    switch (surfaceRenderer) {
      case "image":
        if (VERSION.SDK_INT >= VERSION_CODES.M) {
          // CanvasSurfaceRenderer doesn't work correctly when used with ImageSurfaceRenderer.
          // Use MediaSurfaceRenderer for now.
          return new ImageSurfaceRenderer(
              selectSurfaceRenderer("media", extras), extras.getParcelable("crop"));
        } else {
          throw new RuntimeException("ImageSurfaceRenderer not supported");
        }
      case "media":
        if (VERSION.SDK_INT >= VERSION_CODES.LOLLIPOP) {
          return new MediaSurfaceRenderer(this::createMediaExtractor, extras.getInt("rotation", 0));
        } else {
          throw new RuntimeException("MediaSurfaceRenderer not supported");
        }
      case "canvas":
      default:
        return new CanvasSurfaceRenderer();
    }
  }

  private MediaExtractor createMediaExtractor() {
    // Sample Video generated with FFMPEG.
    // ffmpeg -loop 1 -i ~/engine/src/flutter/lib/ui/fixtures/DashInNooglerHat.jpg -c:v libx264
    // -profile:v main -level:v 5.2 -t 1 -r 1 -vf scale=192:256 -b:v 1M sample.mp4
    try {
      MediaExtractor extractor = new MediaExtractor();
      try (AssetFileDescriptor afd = getAssets().openFd("sample.mp4")) {
        extractor.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
      }
      return extractor;
    } catch (IOException e) {
      e.printStackTrace();
      throw new RuntimeException(e);
    }
  }

  @Override
  public void onPause() {
    surfaceViewRenderer.destroy();
    flutterRenderer.destroy();
    surfaceTextureEntry.release();
    super.onPause();
  }

  @Override
  public void onFlutterUiDisplayed() {
    surfaceTextureEntry =
        Objects.requireNonNull(getFlutterEngine()).getRenderer().createSurfaceTexture();
    SurfaceTexture surfaceTexture = surfaceTextureEntry.surfaceTexture();
    surfaceTexture.setDefaultBufferSize(SURFACE_WIDTH, SURFACE_HEIGHT);
    flutterRenderer.attach(new Surface(surfaceTexture), firstFrameLatch);
    flutterRenderer.repaint();
    textureId = surfaceTextureEntry.id();

    super.onFlutterUiDisplayed();
  }

  @Override
  protected void getScenarioParams(@NonNull Map<String, Object> args) {
    super.getScenarioParams(args);
    args.put("texture_id", textureId);
    args.put("texture_width", SURFACE_WIDTH);
    args.put("texture_height", SURFACE_HEIGHT);
  }

  private static class SurfaceRendererCallback implements SurfaceHolder.Callback {
    final SurfaceRenderer surfaceRenderer;
    final CountDownLatch onFirstFrame;

    public SurfaceRendererCallback(SurfaceRenderer surfaceRenderer, CountDownLatch onFirstFrame) {
      this.surfaceRenderer = surfaceRenderer;
      this.onFirstFrame = onFirstFrame;
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
      surfaceRenderer.attach(holder.getSurface(), onFirstFrame);
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
      surfaceRenderer.repaint();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {}
  }
}
