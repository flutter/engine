package io.flutter.plugin.editing;

import android.support.annotation.Nullable;

/**
 * The object responsible for keeping track of the currently active {@code InputTarget},
 * and delegating input connection creation to said {@code InputTarget}.
 */
public interface InputProxy {
    /**
     * Will potentially update the currently active {@code InputTarget}.
     * @param target The {@code InputTarget} that is requesting an update.
     * @param setAsTarget If true, it will set {@param target} as the currently
     *                    active {@code InputTarget} if it's not already. If false,
     *                    it will revert back to the default {@code InputTarget}, if
     *                    {@param target} is the currently active {@code InputTarget}.
     */
    void updateInputTarget(@Nullable InputTarget target, boolean setAsTarget);
}
