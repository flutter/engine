package io.flutter.embedding.engine;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import java.util.HashMap;
import java.util.Map;

/**
 * Static singleton cache that holds {@link FlutterEngine} instances identified by {@code String}s.
 * <p>
 * {@code FlutterEngineCache} is useful for storing pre-warmed {@link FlutterEngine} instances.
 * {@link io.flutter.embedding.android.FlutterActivity} and
 * {@link io.flutter.embedding.android.FlutterFragment} use the {@code FlutterEngineCache} singleton
 * internally when instructed to use a cached {@link FlutterEngine} based on a given ID. See
 * {@link io.flutter.embedding.android.FlutterActivity.IntentBuilder} and
 * {@link io.flutter.embedding.android.FlutterFragment.Builder} for related APIs.
 */
public class FlutterEngineCache {
  private static FlutterEngineCache instance;

  /**
   * Returns the static singleton instance of {@code FlutterEngineCache}.
   * <p>
   * Creates a new instance if one does not yet exist.
   */
  @NonNull
  public static FlutterEngineCache getInstance() {
    if (instance == null) {
      instance = new FlutterEngineCache();
    }
    return instance;
  }

  private final Map<String, FlutterEngine> cachedEngines = new HashMap<>();

  private FlutterEngineCache() {}

  /**
   * Returns {@code true} if a {@link FlutterEngine} in this cache is associated with the
   * given {@code engineId}.
   */
  public boolean contains(@NonNull String engineId) {
    return cachedEngines.containsKey(engineId);
  }

  /**
   * Returns the {@link FlutterEngine} in this cache that is associated with the given
   * {@code engineId}, or {@code null} is no such {@link FlutterEngine} exists.
   */
  @Nullable
  public FlutterEngine get(@NonNull String engineId) {
    return cachedEngines.get(engineId);
  }

  /**
   * Places the given {@link FlutterEngine} in this cache and associates it with the given
   * {@code engineId}.
   * <p>
   * If a {@link FlutterEngine} already exists in this cache for the given {@code engineId}, that
   * {@link FlutterEngine} is removed from this cache.
   */
  public void put(@NonNull String engineId, @NonNull FlutterEngine engine) {
    cachedEngines.put(engineId, engine);
  }
}
