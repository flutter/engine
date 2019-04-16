package io.flutter.plugin.editing;

import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

public interface InputTarget {

    interface Disposable extends  InputTarget {
        void disposeInputConnection();
    }

    View getTargetView();
    InputConnection createInputConnection(EditorInfo outAttrs);
}
