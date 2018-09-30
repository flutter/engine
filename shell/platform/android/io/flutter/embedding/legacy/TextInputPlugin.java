// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.legacy;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.support.annotation.NonNull;
import android.text.Editable;
import android.text.InputType;
import android.text.Selection;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import io.flutter.embedding.android.FlutterView;
import io.flutter.embedding.engine.systemchannels.TextInputChannel;

/**
 * Android implementation of the text input plugin.
 */
@TargetApi(Build.VERSION_CODES.CUPCAKE)
public class TextInputPlugin {
    private final View view;
    private final InputMethodManager imm;
    private final TextInputChannel textInputChannel;
    private int textInputClientId = 0;
    private TextInputChannel.Configuration configuration;
    private Editable editable;
    private boolean restartInputPending;

    private final TextInputChannel.TextInputMethodHandler textInputMethodHandler = new TextInputChannel.TextInputMethodHandler() {
        @Override
        public void show() {
            showTextInput();
        }

        @Override
        public void hide() {
            hideTextInput();
        }

        @Override
        public void setClient(int textInputClientId, TextInputChannel.Configuration configuration) {
            setTextInputClient(textInputClientId, configuration);
        }

        @Override
        public void setEditingState(TextInputChannel.TextEditState editingState) {
            setTextInputEditingState(editingState);
        }

        @Override
        public void clearClient() {
            clearTextInputClient();
        }
    };

    public TextInputPlugin(@NonNull View view, @NonNull TextInputChannel textInputChannel) {
        this.view = view;
        this.imm = (InputMethodManager) view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        this.textInputChannel = textInputChannel;
        this.textInputChannel.setTextInputMethodHandler(textInputMethodHandler);
    }

    public void release() {
        this.textInputChannel.setTextInputMethodHandler(null);
    }

    public InputConnection createInputConnection(FlutterView view, EditorInfo outAttrs) {
        if (textInputClientId == 0) return null;

        outAttrs.inputType = inputTypeFromTextInputType(
            configuration.inputType,
            configuration.obscureText,
            configuration.autocorrect,
            configuration.textCapitalization
        );
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_FULLSCREEN;
        int enterAction;
        if (configuration.inputAction == null) {
            // If an explicit input action isn't set, then default to none for multi-line fields
            // and done for single line fields.
            enterAction = (InputType.TYPE_TEXT_FLAG_MULTI_LINE & outAttrs.inputType) != 0
                    ? EditorInfo.IME_ACTION_NONE
                    : EditorInfo.IME_ACTION_DONE;
        } else {
            enterAction = configuration.inputAction;
        }
        if (configuration.actionLabel != null) {
            outAttrs.actionLabel = configuration.actionLabel;
            outAttrs.actionId = enterAction;
        }
        outAttrs.imeOptions |= enterAction;

        InputConnectionAdaptor connection =
                new InputConnectionAdaptor(view, textInputClientId, textInputChannel, editable);
        outAttrs.initialSelStart = Selection.getSelectionStart(editable);
        outAttrs.initialSelEnd = Selection.getSelectionEnd(editable);

        return connection;
    }

    private static int inputTypeFromTextInputType(
        TextInputChannel.InputType inputType,
        boolean obscureText,
        boolean autocorrect,
        TextInputChannel.TextCapitalization textCapitalization
    ) {
        switch (inputType.type) {
            case DATETIME:
                return InputType.TYPE_CLASS_DATETIME;
            case NUMBER:
                int textType = InputType.TYPE_CLASS_NUMBER;
                if (inputType.isSigned) {
                    textType |= InputType.TYPE_NUMBER_FLAG_SIGNED;
                }
                if (inputType.isDecimal) {
                    textType |= InputType.TYPE_NUMBER_FLAG_DECIMAL;
                }
                return textType;
            case PHONE:
                return InputType.TYPE_CLASS_PHONE;
        }

        int textType = InputType.TYPE_CLASS_TEXT;
        switch (inputType.type) {
            case MULTILINE:
                textType |= InputType.TYPE_TEXT_FLAG_MULTI_LINE;
                break;
            case EMAIL_ADDRESS:
                textType |= InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
                break;
            case URL:
                textType |= InputType.TYPE_TEXT_VARIATION_URI;
                break;
        }

        if (obscureText) {
            // Note: both required. Some devices ignore TYPE_TEXT_FLAG_NO_SUGGESTIONS.
            textType |= InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            textType |= InputType.TYPE_TEXT_VARIATION_PASSWORD;
        } else {
            if (autocorrect) {
                textType |= InputType.TYPE_TEXT_FLAG_AUTO_CORRECT;
            }
        }

        switch (textCapitalization) {
            case CHARACTERS:
                textType |= InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS;
                break;
            case WORDS:
                textType |= InputType.TYPE_TEXT_FLAG_CAP_WORDS;
                break;
            case SENTENCES:
                textType |= InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;
                break;
        }

        return textType;
    }

    private void showTextInput() {
        imm.showSoftInput(view, 0);
    }

    private void hideTextInput() {
        imm.hideSoftInputFromWindow(view.getApplicationWindowToken(), 0);
    }

    private void setTextInputClient(int client, TextInputChannel.Configuration configuration) {
        textInputClientId = client;
        this.configuration = configuration;
        editable = Editable.Factory.getInstance().newEditable("");

        // setTextInputClient will be followed by a call to setTextInputEditingState.
        // Do a restartInput at that time.
        restartInputPending = true;
    }

    private void setTextInputEditingState(TextInputChannel.TextEditState state) {
        if (!restartInputPending && state.text.equals(editable.toString())) {
            applyStateToSelection(state);
            imm.updateSelection(view, Math.max(Selection.getSelectionStart(editable), 0),
                Math.max(Selection.getSelectionEnd(editable), 0),
                BaseInputConnection.getComposingSpanStart(editable),
                BaseInputConnection.getComposingSpanEnd(editable));
        } else {
            editable.replace(0, editable.length(), state.text);
            applyStateToSelection(state);
            imm.restartInput(view);
            restartInputPending = false;
        }
    }

    private void applyStateToSelection(TextInputChannel.TextEditState state) {
        if (state.selectionStart >= 0 && state.selectionEnd <= editable.length() && state.selectionEnd >= 0
            && state.selectionEnd <= editable.length()) {
            Selection.setSelection(editable, state.selectionStart, state.selectionEnd);
        } else {
            Selection.removeSelection(editable);
        }
    }

    private void clearTextInputClient() {
        textInputClientId = 0;
    }
}
