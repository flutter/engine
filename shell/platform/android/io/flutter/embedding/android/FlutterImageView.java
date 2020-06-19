// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

impo

class FlutterImageView extends View implements RenderSurface {
  private final ImageReader imageReader;
  @Nullable private Image nextImage;
  private Image currentImage;

  public FlutterImageView(Context context, ImageReader imageReader) {
    super(context, null);
    this.imageReader = imageReader;
  }

  @RequiresApi(api = Build.VERSION_CODES.KITKAT)
  public void acquireLatestImage() {
    nextImage = imageReader.acquireLatestImage();
    invalidate();
  }

  @RequiresApi(api = Build.VERSION_CODES.P)
  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    if (nextImage == null) {
      return;
    }

    currentImage.close();
    currentImage = nextImage;
    nextImage = null;

    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
      drawImageBuffer(canvas);
    } else if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
      drawImagePlane(canvas);
    }
  }

  @RequiresApi(api = Build.VERSION_CODES.P)
  private void drawImageBuffer(Canvas canvas) {
    final HardwareBuffer buffer = currentImage.getHardwareBuffer();

    final Bitmap bitmap = Bitmap.wrapHardwareBuffer(
        buffer,
        ColorSpace.get(ColorSpace.Named.SRGB));
    canvas.drawBitmap(bitmap, 0, 0, null);
  }

  @RequiresApi(api = Build.VERSION_CODES.KITKAT)
  private void drawImagePlane(Canvas canvas) {
    if (currentImage == null) {
      return;
    }

    final Plane[] imagePlanes = currentImage.getPlanes();
    if (imagePlanes.length != 1) {
      return;
    }

    final Plane imagePlane = imagePlanes[0];
    final int desiredWidth = imagePlane.getRowStride() / imagePlane.getPixelStride();
    final int desiredHeight = currentImage.getHeight();

    final Bitmap bitmap = android.graphics.Bitmap.createBitmap(
      desiredWidth,
      desiredHeight,
      android.graphics.Bitmap.Config.ARGB_8888);

    bitmap.copyPixelsFromBuffer(imagePlane.getBuffer());
    canvas.drawBitmap(bitmap, 0, 0, null);
  }
}