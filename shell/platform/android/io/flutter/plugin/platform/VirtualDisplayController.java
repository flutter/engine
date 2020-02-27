// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import static android.view.View.OnFocusChangeListener;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.support.annotation.NonNull;
import android.view.Surface;
import android.view.View;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import io.flutter.view.TextureRegistry;

@TargetApi(Build.VERSION_CODES.KITKAT_WATCH)
class VirtualDisplayController {

    public static VirtualDisplayController create(
            Context context,
            AccessibilityEventsDelegate accessibilityEventsDelegate,
            PlatformViewFactory viewFactory,
            TextureRegistry.SurfaceTextureEntry textureEntry,
            int width,
            int height,
            int viewId,
            FrameLayout platformViewsContainer,
            Object createParams,
            OnFocusChangeListener focusChangeListener
    ) {
        textureEntry.surfaceTexture().setDefaultBufferSize(width, height);

        Surface vdRouterSurface = new Surface(textureEntry.surfaceTexture());

        ViewDrawingRerouter viewDrawingRerouter = new ViewDrawingRerouter(
            context,
            vdRouterSurface,
            width,
            height);

        viewDrawingRerouter.attachToPlatformViewContainer(platformViewsContainer);

        return new VirtualDisplayController(
                context,
                accessibilityEventsDelegate,
                viewFactory,
                textureEntry,
                focusChangeListener,
                viewId,
                vdRouterSurface,
                viewDrawingRerouter,
                createParams
        );
    }

    private final Context context;
    private final AccessibilityEventsDelegate accessibilityEventsDelegate;
    private final int densityDpi;
    private final TextureRegistry.SurfaceTextureEntry textureEntry;
    private final OnFocusChangeListener focusChangeListener;
    private SingleViewPresentationAlternative presentationAlternative;
    private Surface surface;
    private ViewDrawingRerouter viewDrawingRerouter;
    private View flutterView;


    private VirtualDisplayController(
            Context context,
            AccessibilityEventsDelegate accessibilityEventsDelegate,
            PlatformViewFactory viewFactory,
            TextureRegistry.SurfaceTextureEntry textureEntry,
            OnFocusChangeListener focusChangeListener,
            int viewId,
            Surface surface,
            ViewDrawingRerouter viewDrawingRerouter,
            Object createParams
    ) {
        this.context = context;
        this.accessibilityEventsDelegate = accessibilityEventsDelegate;
        this.textureEntry = textureEntry;
        this.focusChangeListener = focusChangeListener;
        this.surface = surface;
        this.viewDrawingRerouter = viewDrawingRerouter;
        densityDpi = context.getResources().getDisplayMetrics().densityDpi;

        presentationAlternative = new SingleViewPresentationAlternative(context,
                viewDrawingRerouter,
                viewFactory,
                accessibilityEventsDelegate,
                viewId,
                createParams,
                focusChangeListener
            );
        presentationAlternative.init();

    }

    public void resize(final int width, final int height, final Runnable onNewSizeFrameAvailable) {
        viewDrawingRerouter.resize(height, width);
    }

    public void dispose() {
        PlatformView view = presentationAlternative.getView();

        presentationAlternative.dispose();

        view.dispose();

        viewDrawingRerouter.release();

        textureEntry.release();
    }
    presentation.getView().onFlutterViewDetached();
  }

    /**
     * See {@link PlatformView#onFlutterViewAttached(View)}
     */
    /*package*/ void onFlutterViewAttached(@NonNull View flutterView) {
        if (presentationAlternative == null || presentationAlternative.getView() == null) {
            return;
        }
        presentationAlternative.getView().onFlutterViewAttached(flutterView);
    }
    presentation.getView().onInputConnectionLocked();
  }

    /**
     * See {@link PlatformView#onFlutterViewDetached()}
     */
    /*package*/ void onFlutterViewDetached() {
        if (presentationAlternative == null || presentationAlternative.getView() == null) {
            return;
        }
        presentationAlternative.getView().onFlutterViewDetached();
    }

    /*package*/ void onInputConnectionLocked() {
        if (presentationAlternative == null || presentationAlternative.getView() == null) {
            return;
        }
        presentationAlternative.getView().onInputConnectionLocked();
    }

    /*package*/ void onInputConnectionUnlocked() {
        if (presentationAlternative == null || presentationAlternative.getView() == null) {
            return;
        }
        presentationAlternative.getView().onInputConnectionUnlocked();
    }

    public View getView() {
        if (presentationAlternative == null)
            return null;
        PlatformView platformView = presentationAlternative.getView();
        return platformView.getView();
    }

    @Override
    public void onDraw() {
      if (mOnDrawRunnable == null) {
        return;
      }
      mOnDrawRunnable.run();
      mOnDrawRunnable = null;
      mView.post(
          new Runnable() {
            @Override
            public void run() {
              mView.getViewTreeObserver().removeOnDrawListener(OneTimeOnDrawListener.this);
            }
          });
    }
  }
}
