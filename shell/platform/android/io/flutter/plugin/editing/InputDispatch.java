package io.flutter.plugin.editing;

import android.support.annotation.Nullable;

public interface InputDispatch {
    void updateInputTarget(@Nullable InputTarget target, boolean setAsTarget, boolean force);
}
