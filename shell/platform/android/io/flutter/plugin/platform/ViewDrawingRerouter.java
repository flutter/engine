package io.flutter.plugin.platform;

import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.ViewGroup.LayoutParams;
import android.view.View;
import android.view.Surface;
import android.content.Context;
import android.widget.FrameLayout;
import android.graphics.Canvas;
import android.graphics.Rect;


class ViewDrawingRerouter {
    
    private Surface surface;
    private View contentView;
    private ViewContainer viewContainer;
    int height;
    int width;
    private FrameLayout platformViewsContainer;

    public ViewDrawingRerouter(
            Context context,
            Surface surface,
            int width,
            int height
    ) {
        this.surface = surface;
        this.width = width;
        this.height = height;
        this.contentView = null;
        this.viewContainer = new ViewContainer(context, surface);
        this.clearSurface();
    }
    private void clearSurface() {
        Canvas c = surface.lockHardwareCanvas();
        c.drawARGB(0, 0, 0, 0);
        surface.unlockCanvasAndPost(c);
    }



    public void setContentView(View view) {
        contentView = view;

        if(viewContainer != null) {
            viewContainer.removeAllViews();
            if(view != null) {
                viewContainer.addView(view);
            }
        }

    }

    public ViewGroup getContainer() {
        return viewContainer;
    }

    private void setContainerSize(int height, int width) {
        if(contentView == null) {
            LayoutParams params = contentView.getLayoutParams();
            params.height = height;
            params.width = width;
            contentView.setLayoutParams(params);
        }
    }
    
    public void resize(int height, int width) {
        this.height = height;
        this.width = width;
        setContainerSize(height, width);
    }

    public void attachToPlatformViewContainer(FrameLayout platformViewsContainer) {
        this.platformViewsContainer = platformViewsContainer;
        platformViewsContainer.addView(viewContainer, width, height);
    }

    public void detachFromPlatformViewContainer() {
        if(platformViewsContainer == null)
            return;
        platformViewsContainer.removeView(viewContainer);
    }



	public void release() {
        detachFromPlatformViewContainer();
        this.setContentView(null);
        surface.release();
    }

    private class ViewContainer extends FrameLayout {
        private Surface surface;

        public ViewContainer(Context context, Surface surface) {
            super(context);
            this.surface = surface;
        }

        private Canvas lockReplacementCanvas(Canvas canvas) {
            if(canvas.isHardwareAccelerated()) {
                return surface.lockHardwareCanvas();
            }
            Rect rect = canvas.getClipBounds();
            return surface.lockCanvas(rect);
        }

        private void unlockCanvasAndPost(Canvas canvas) {
            surface.unlockCanvasAndPost(canvas);
        }

        @Override
        protected void dispatchDraw(Canvas canvas) {
            Canvas surfaceCanvas = lockReplacementCanvas(canvas);
            super.dispatchDraw(surfaceCanvas);
            unlockCanvasAndPost(surfaceCanvas);
        }

        @Override
        public ViewParent invalidateChildInParent (int[] location, Rect dirty) {
            postInvalidate();
            return super.invalidateChildInParent(location, dirty);
        }

        @Override
        public void onDescendantInvalidated(View child, View target) {
            postInvalidate();
            super.onDescendantInvalidated(child, target);
        }
    }
}