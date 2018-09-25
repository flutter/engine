package io.flutter.embedding.engine.renderer;

public interface OnFirstFrameRenderedListener {
  /**
   * A {@link FlutterRenderer} has painted its first frame since being initialized.
   *
   * This method will not be invoked if this listener is added after the first frame is rendered.
   */
  void onFirstFrameRendered();
}
