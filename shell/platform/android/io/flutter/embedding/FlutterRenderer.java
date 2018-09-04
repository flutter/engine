package io.flutter.embedding;

import android.annotation.TargetApi;
import android.os.Build;
import android.support.annotation.NonNull;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class FlutterRenderer {

  private final Set<OnFirstFrameRenderedListener> firstFrameListeners = new CopyOnWriteArraySet<>();

  public void attachToView() {
    // TODO(mattcarroll): do whatever is needed to start rendering and processing user input
  }

  public void detachFromView() {
    // TODO(mattcarroll): stop rendering to surface and stop processing user input
  }

  public void addOnFirstFrameRenderedListener(@NonNull OnFirstFrameRenderedListener listener) {
    firstFrameListeners.add(listener);
  }

  public void removeOnFirstFrameRenderedListener(@NonNull OnFirstFrameRenderedListener listener) {
    firstFrameListeners.remove(listener);
  }

  private void notifyFirstFrameListeners() {
    for (OnFirstFrameRenderedListener listener : firstFrameListeners) {
      listener.onFirstFrameRendered();
    }
  }

  public interface OnFirstFrameRenderedListener {
    void onFirstFrameRendered();
  }
}
