package io.flutter.plugin.platform;

import android.view.WindowManager;

/** Default implementation when using the regular Android SDK. */
final class WindowManagerHandler extends SingleViewWindowManager {

    WindowManagerHandler(WindowManager delegate, SingleViewFakeWindowViewGroup fakeWindowViewGroup) {
        super(delegate, fakeWindowViewGroup);
    }
}
