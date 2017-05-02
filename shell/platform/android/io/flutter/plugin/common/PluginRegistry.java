// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.common;

import android.app.Activity;
import android.content.Intent;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Registry used by plugins to register their interaction with Android APIs.
 *
 * <p>Flutter applications by default include an auto-generated
 * PluginRegistry sub-class that overrides {@link #registerPlugins(PluginContext)}
 * with contributions from each plugin mentioned in the application's pubspec
 * file. Their main {@link Activity} is a {@link io.flutter.app.FlutterActivity}
 * constructed with that subclass.</p>
 */
public class PluginRegistry {
    private final Map<String, Object> map = new LinkedHashMap<>(0);
    private final List<RequestPermissionResultListener> requestPermissionResultListeners = new ArrayList<>(0);
    private final List<ActivityResultListener> activityResultListeners = new ArrayList<>(0);
    private final List<NewIntentListener> newIntentListeners = new ArrayList<>(0);

    /**
     * Registers the plugins of this registry by delegation to the #registerPlugins
     * hook method. Plugins install components and uses the provided {@link Activity}
     * and {@link BinaryMessenger} for Flutter/platform.
     *
     * <p>Intended to be called by the main {@link Activity}'s {@code onCreate}
     * method.
     */
    public final void registerPlugins(Activity activity, BinaryMessenger messenger) {
      registerPlugins(new PluginContext(activity, messenger));
    }

    /**
     * Hook method for registering plugins. Subclasses are expected to call a
     * static method on each plugin's main implementation class in turn,
     * providing the {@link PluginContext} as argument.
     */
    protected void registerPlugins(PluginContext context) {
    }

    /**
     * Returns whether an entry for the specified key has been put.
     */
    public final boolean containsKey(String key) {
      return map.containsKey(key);
    }

    /**
     * Returns the value associated to the specified key.
     * <p>The static type of the value is determed by the call site.</p>
     *
     * @return the value, possibly null.
     */
    @SuppressWarnings("unchecked")
    public final <T> T get(String key) {
      return (T) map.get(key);
    }

    /**
     * Delegates handling of request permissions results to registered plugins.
     *
     * <p>Intended to be called by the application's main {@link Activity}.</a>
     *
     * @see {@link Activity#onRequestPermissionResult(int,String[],int[])}
     */
    public final void onRequestPermissionResult(int requestCode, String[] permissions, int[] grantResults) {
      for (RequestPermissionResultListener listener: requestPermissionResultListeners) {
        if (listener.onRequestPermissionResult(requestCode, permissions, grantResults)) {
          break;
        }
      }
    }

    /**
     * Delegates handling of activity results to registered plugins.
     *
     * <p>Intended to be called by the application's main {@link Activity}.</a>
     *
     * @see {@link Activity#onActivityResult(int,int,Intent)}
     */
    public final void onActivityResult(int requestCode, int resultCode, Intent data) {
      for (ActivityResultListener listener: activityResultListeners) {
        if (listener.onActivityResult(requestCode, resultCode, data)) {
          break;
        }
      }
    }

    /**
     * Delegates handling of new intents to registered plugins.
     *
     * <p>Intended to be called by the application's main {@link Activity}.</a>
     *
     * @see {@link Activity#onNewIntent(Intent)}
     */
    public final void onNewIntent(Intent intent) {
      for (NewIntentListener listener: newIntentListeners) {
        if (listener.onNewIntent(intent)) {
          break;
        }
      }
    }

    /**
     * Context used by plugins when registering. Provides access to the
     * application context (via {@link #activity()}), Flutter/Android messaging
     * (via {@link #messenger()} and {@link #newMethodChannel(String)}), and
     * allows plugins to register callbacks to activity lifecycle methods.
     */
    public final class PluginContext {
      private final Activity activity;
      private final BinaryMessenger messenger;

      PluginContext(Activity activity, BinaryMessenger messenger) {
        this.activity = activity;
        this.messenger = messenger;
      }

      /**
       * Registers a key and value for lookup by interested clients which are
       * expected to know the type of value for a given key.
       *
       * <p>Plugins should prefix keys by inverted domain names to avoid clashes:
       * {@code com.example.myplugin.SomeKey}.</p>
       */
      public final void put(String key, Object value) {
        if (map.containsKey(key)) {
          throw new IllegalArgumentException("Double registration of " + key);
        }
        map.put(key, value);
      }

      public void addRequestPermissionResultListener(RequestPermissionResultListener listener) {
        requestPermissionResultListeners.add(listener);
      }

      public void addActivityResultListener(ActivityResultListener listener) {
        activityResultListeners.add(listener);
      }

      public void addNewIntentListener(NewIntentListener listener) {
        newIntentListeners.add(listener);
      }

      public Activity activity() {
        return activity;
      }

      public BinaryMessenger messenger() {
        return messenger;
      }

      public MethodChannel newMethodChannel(String name) {
        return new MethodChannel(messenger, name);
      }
    }

    /**
     * Delegate interface for handling results of permission requests on
     * behalf of an {@link Activity}.
     */
    public interface RequestPermissionResultListener {
      /**
       * @return true if the result has been handled.
       */
      boolean onRequestPermissionResult(int requestCode, String[] permissions, int[] grantResults);
    }

    /**
     * Delegate interface for handling activity results on behalf of an
     * {@link Activity}.
     */
    public interface ActivityResultListener {
      /**
       * @return true if the result has been handled.
       */
      boolean onActivityResult(int requestCode, int resultCode, Intent data);
    }

    /**
     * Delegate interface for handling new intents on behalf of an
     * {@link Activity}.
     */
    public interface NewIntentListener {
      /**
       * @return true if the new intent has been handled.
       */
      boolean onNewIntent(Intent intent);
    }
}
