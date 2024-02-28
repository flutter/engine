package dev.flutter.scenarios.externaltextures;

import android.view.Surface;

import java.util.concurrent.CountDownLatch;

public interface SurfaceRenderer {
    void attach(Surface surface, CountDownLatch onFirstFrame);

    void repaint();

    void destroy();
}
