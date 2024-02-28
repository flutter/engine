package dev.flutter.scenarios.externaltextures;

import android.view.Surface;
import java.util.concurrent.CountDownLatch;

/**
 * Represents a generic interface for different types of surface rendering.
 *
 * <p>TODO(matanlurey): Eliminate this interface.
 *
 * <p>This interface is ... not ideal. Not every implementation (of which there are only 3) even
 * implements all 3 methods. In future refactoring, it should be possible to just have a few
 * different external texture tests that are self-contained, instead of requiring so much
 * indirection and unused polymorphism.
 */
public interface SurfaceRenderer {
  /**
   * Attaches the renderer to the provided surface.
   *
   * <p>This is a flawed method that acts more like a constructor, that is, classes that implement
   * it (and they must, because there is no other way to get access to these parameters) must at
   * least store these parameters, and often do any initialization work required.
   *
   * @param surface which surface to render to.
   * @param onFirstFrame a latch to call after the first frame has rendered.
   */
  void attach(Surface surface, CountDownLatch onFirstFrame);

  /**
   * Invoked at least once after {@code attach}, and again after structural changes to the surface
   * (such as format or size, {@link android.view.SurfaceHolder.Callback} for details, especially
   * around {@code surfaceChanged}).
   */
  void repaint();

  /** Invoked when the surface should be destroyed and resources reclaimed. */
  void destroy();
}
