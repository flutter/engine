// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.app;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import io.flutter.app.FlutterActivityDelegate.ViewFactory;
import io.flutter.plugin.common.PluginRegistry;
import io.flutter.plugin.common.PluginRegistry.Registrar;
import io.flutter.view.FlutterView;

/**
 * Base class for activities that use Flutter.
 */
public class FlutterActivity extends Activity implements FlutterView.Provider, PluginRegistry, ViewFactory {
    private final FlutterActivityDelegate delegate = new FlutterActivityDelegate(this, this);

    /**
     * Returns the Flutter view used by this activity; will be null before
     * {@link #onCreate(Bundle)} is called.
     */
    @Override
    public FlutterView getFlutterView() {
        return delegate.getFlutterView();
    }

    /**
     * Hook for subclasses to customize their startup behavior.
     *
     * @deprecated Just override {@link #onCreate(Bundle)} instead, and add your
     * logic after calling {@code super.onCreate()}.
     */
    @Deprecated
    protected void onFlutterReady() {}

    /**
     * Hook for subclasses to customize the creation of the
     * {@code FlutterView}.
     * <p/>
     * The default implementation returns {@code null}, which will cause the
     * activity to use a newly instantiated full-screen view.
     */
    @Override
    public FlutterView createFlutterView(Context context) {
        return null;
    }

    @Override
    public final boolean hasPlugin(String key) {
        return delegate.hasPlugin(key);
    }

    @Override
    public final <T> T valuePublishedByPlugin(String pluginKey) {
        return delegate.valuePublishedByPlugin(pluginKey);
    }

    @Override
    public final Registrar registrarFor(String pluginKey) {
        return delegate.registrarFor(pluginKey);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        delegate.onCreate(savedInstanceState);
        onFlutterReady();
    }

    @Override
    protected void onDestroy() {
        delegate.onDestroy();
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (!delegate.onBackPressed()) {
            super.onBackPressed();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        delegate.onPause();
    }

    @Override
    protected void onPostResume() {
        super.onPostResume();
        delegate.onPostResume();
    }

    // @Override - added in API level 23
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        delegate.onRequestPermissionResult(requestCode, permissions, grantResults);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (!delegate.onActivityResult(requestCode, resultCode, data)) {
            super.onActivityResult(requestCode, resultCode, data);
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        delegate.onNewIntent(intent);
    }

    @Override
    public void onUserLeaveHint() {
        delegate.onUserLeaveHint();
    }

    @Override
    public void onTrimMemory(int level) {
        delegate.onTrimMemory(level);
    }

    @Override
    public void onLowMemory() {
        delegate.onLowMemory();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        delegate.onConfigurationChanged(newConfig);
    }
}
