package io.flutter.embedding.engine.plugins.host;

import android.app.Activity;
import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.Lifecycle;
import io.flutter.embedding.engine.FlutterShellArgs;

public interface HostComponent {

  @NonNull
  Context getContext();

  @Nullable
  Activity getActivity();

  /**
   * Returns the {@link Lifecycle} that backs the host {@link android.app.Activity} or {@code
   * Fragment}.
   */
  @NonNull
  Lifecycle getLifecycle();

  /** Returns the {@link FlutterShellArgs} that should be used when initializing Flutter. */
  @NonNull
  FlutterShellArgs getFlutterShellArgs();
}
