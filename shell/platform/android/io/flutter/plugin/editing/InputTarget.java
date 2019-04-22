package io.flutter.plugin.editing;

import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

/**
 * An object that creates and manages an {@code InputConnection}.
 */
public interface InputTarget {

    /**
     * @return The {@code View} that is requesting input.
     */
    View getTargetView();

    /**
     * @param outAttrs
     * @return An {@code InputConnection} to the target view.
     */
    InputConnection createInputConnection(EditorInfo outAttrs);

    /**
     * Called whenever this {@code InputTarget} is not currently active.
     */
    void disposeInputConnection();
}
