// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

import android.annotation.SuppressLint;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.DynamicLayout;
import android.text.Editable;
import android.text.InputType;
import android.text.Layout;
import android.text.Selection;
import android.text.TextPaint;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.CursorAnchorInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.ExtractedText;
import android.view.inputmethod.ExtractedTextRequest;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.InputMethodSubtype;
import io.flutter.Log;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.systemchannels.TextInputChannel;

class InputConnectionAdaptor extends BaseInputConnection {
  private static final String TAG = "flutter";

  private final View mFlutterView;
  private final int mClient;
  private final TextInputChannel textInputChannel;
  private final Editable mEditable;
  private final EditorInfo mEditorInfo;
  private ExtractedTextRequest mExtractRequest;
  private boolean mMonitorCursorUpdate = false;
  private CursorAnchorInfo.Builder mCursorAnchorInfoBuilder;
  private ExtractedText mExtractedText = new ExtractedText();
  private int mBatchCount;
  private InputMethodManager mImm;
  private final Layout mLayout;
  private FlutterTextUtils flutterTextUtils;
  // Used to determine if Samsung-specific hacks should be applied.
  private final boolean isSamsung;

  private TextEditingValue mLastUpdatedImmEditingValue;
  private TextEditingValue mLastKnownTextEditingValue;
  // Data class used to get and store the last-sent values via updateEditingState to
  // the  framework. These are then compared against to prevent redundant messages
  // with the same data before any valid operations were made to the contents.
  private class TextEditingValue {
    public int selectionStart;
    public int selectionEnd;
    public int composingStart;
    public int composingEnd;
    public String text;

    public TextEditingValue(TextInputChannel.TextEditState state) {
      text = state.text;
      selectionStart = state.selectionStart;
      selectionEnd = state.selectionEnd;
      composingStart = state.composingStart;
      composingEnd = state.composingEnd;
    }

    public TextEditingValue(Editable editable) {
      selectionStart = Selection.getSelectionStart(editable);
      selectionEnd = Selection.getSelectionEnd(editable);
      composingStart = BaseInputConnection.getComposingSpanStart(editable);
      composingEnd = BaseInputConnection.getComposingSpanEnd(editable);
      text = editable.toString();
    }

    @Override
    public boolean equals(Object o) {
      if (o == this) {
        return true;
      }
      if (!(o instanceof TextEditingValue)) {
        return false;
      }
      TextEditingValue value = (TextEditingValue) o;
      return selectionStart == value.selectionStart
          && selectionEnd == value.selectionEnd
          && composingStart == value.composingStart
          && composingEnd == value.composingEnd
          && text.equals(value.text);
    }

    @Override
    public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + selectionStart;
      result = prime * result + selectionEnd;
      result = prime * result + composingStart;
      result = prime * result + composingEnd;
      result = prime * result + text.hashCode();
      return result;
    }
  }

  @SuppressWarnings("deprecation")
  public InputConnectionAdaptor(
      View view,
      int client,
      TextInputChannel textInputChannel,
      Editable editable,
      EditorInfo editorInfo,
      FlutterJNI flutterJNI) {
    super(view, true);
    mFlutterView = view;
    mClient = client;
    this.textInputChannel = textInputChannel;
    mEditable = editable;
    mEditorInfo = editorInfo;
    mBatchCount = 0;
    // Initialize the "last seen" text editing values to a non-null value.
    mLastUpdatedImmEditingValue = new TextEditingValue(mEditable);
    mLastKnownTextEditingValue = mLastUpdatedImmEditingValue;
    this.flutterTextUtils = new FlutterTextUtils(flutterJNI);
    // We create a dummy Layout with max width so that the selection
    // shifting acts as if all text were in one line.
    mLayout =
        new DynamicLayout(
            mEditable,
            new TextPaint(),
            Integer.MAX_VALUE,
            Layout.Alignment.ALIGN_NORMAL,
            1.0f,
            0.0f,
            false);
    mImm = (InputMethodManager) view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);

    isSamsung = isSamsung();
  }

  public InputConnectionAdaptor(
      View view,
      int client,
      TextInputChannel textInputChannel,
      Editable editable,
      EditorInfo editorInfo) {
    this(view, client, textInputChannel, editable, editorInfo, new FlutterJNI());
  }

  // Send the current state of the editable to Flutter.
  private void updateEditingState() {
    // If the IME is in the middle of a batch edit, then wait until it completes.
    if (mBatchCount > 0) {
      return;
    }

    TextEditingValue currentValue = new TextEditingValue(mEditable);

    // Return if no meaningful changes have occurred.
    if (currentValue.equals(mLastKnownTextEditingValue)) {
      return;
    }

    Log.v(TAG, "send EditingState to flutter: " + currentValue.toString());
    textInputChannel.updateEditingState(
        mClient,
        currentValue.text,
        currentValue.selectionStart,
        currentValue.selectionEnd,
        currentValue.composingStart,
        currentValue.composingEnd);

    mLastKnownTextEditingValue = currentValue;
  }

  private ExtractedText getExtractedText(TextEditingValue editingValue) {
    mExtractedText.startOffset = 0;
    mExtractedText.partialStartOffset = -1;
    mExtractedText.partialEndOffset = -1;
    mExtractedText.selectionStart = editingValue.selectionStart;
    mExtractedText.selectionEnd = editingValue.selectionEnd;
    mExtractedText.text = editingValue.text;
    return mExtractedText;
  }

  private CursorAnchorInfo getCursorAnchorInfo(TextEditingValue editingValue) {
    if (mCursorAnchorInfoBuilder == null) {
      mCursorAnchorInfoBuilder = new CursorAnchorInfo.Builder();
    } else {
      mCursorAnchorInfoBuilder.reset();
    }

    mCursorAnchorInfoBuilder.setSelectionRange(
        editingValue.selectionStart, editingValue.selectionEnd);
    final int composingStart = editingValue.composingStart;
    final int composingEnd = editingValue.composingEnd;
    if (composingStart >= 0 && composingEnd > composingStart) {
      mCursorAnchorInfoBuilder.setComposingText(
          composingStart, editingValue.text.subSequence(composingStart, composingEnd));
    } else {
      mCursorAnchorInfoBuilder.setComposingText(-1, "");
    }
    return mCursorAnchorInfoBuilder.build();
  }

  private void updateIMMIfNeeded() {
    if (mBatchCount > 0) {
      return;
    }

    TextEditingValue currentValue = new TextEditingValue(mEditable);

    // Always send selection update. InputMethodManager#updateSelection skips sending the message
    // if none of the parameters have changed since the last time we called it.
    mImm.updateSelection(
        mFlutterView,
        currentValue.selectionStart,
        currentValue.selectionEnd,
        currentValue.composingStart,
        currentValue.composingEnd);

    if (currentValue == mLastUpdatedImmEditingValue) {
      return;
    }

    if (mExtractRequest != null) {
      mImm.updateExtractedText(mFlutterView, mExtractRequest.token, getExtractedText(currentValue));
    }

    if (mMonitorCursorUpdate) {
      final CursorAnchorInfo info = getCursorAnchorInfo(currentValue);
      mImm.updateCursorAnchorInfo(mFlutterView, info);
      Log.v(TAG, "update CursorAnchorInfo: " + info.toString());
    }

    mLastUpdatedImmEditingValue = currentValue;
  }

  // Called when the current text editing state held by the text input plugin (in mEditable) is
  // overwritten by a newly received value from the framework.
  public void didUpdateEditingValue() {
    mLastKnownTextEditingValue = new TextEditingValue(mEditable);
    // Try to update the input method immediately after the internal state change. Or defer it to
    // endBatchEdit if we're in a nested edit.
    updateIMMIfNeeded();
  }

  @Override
  public Editable getEditable() {
    return mEditable;
  }

  @Override
  public boolean beginBatchEdit() {
    mBatchCount++;
    return super.beginBatchEdit();
  }

  @Override
  public boolean endBatchEdit() {
    boolean result = super.endBatchEdit();
    mBatchCount--;
    // These 2 methods do nothing if mBatchCount > 0.
    updateEditingState();
    updateIMMIfNeeded();
    return result;
  }

  @Override
  public boolean commitText(CharSequence text, int newCursorPosition) {
    beginBatchEdit();
    boolean result = super.commitText(text, newCursorPosition);
    endBatchEdit();
    return result;
  }

  @Override
  public boolean deleteSurroundingText(int beforeLength, int afterLength) {
    if (Selection.getSelectionStart(mEditable) == -1) return true;

    beginBatchEdit();
    boolean result = super.deleteSurroundingText(beforeLength, afterLength);
    endBatchEdit();
    return result;
  }

  @Override
  public boolean deleteSurroundingTextInCodePoints(int beforeLength, int afterLength) {
    beginBatchEdit();
    boolean result = super.deleteSurroundingTextInCodePoints(beforeLength, afterLength);
    endBatchEdit();
    return result;
  }

  @Override
  public boolean setComposingRegion(int start, int end) {
    beginBatchEdit();
    Log.i("flutter", "engine: set CR: " + String.valueOf(start) + " - " + String.valueOf(end));
    boolean result = super.setComposingRegion(start, end);
    endBatchEdit();
    return result;
  }

  @Override
  public boolean setComposingText(CharSequence text, int newCursorPosition) {
    boolean result;
    beginBatchEdit();
    Log.i("flutter", "engine: set CT: " + text + ", " + String.valueOf(newCursorPosition));
    if (text.length() == 0) {
      result = super.commitText(text, newCursorPosition);
    } else {
      result = super.setComposingText(text, newCursorPosition);
    }
    endBatchEdit();
    return result;
  }

  @Override
  public boolean finishComposingText() {
    Log.i("flutter", "engine: finish composing");
    beginBatchEdit();
    boolean result = super.finishComposingText();
    endBatchEdit();
    return result;
  }

  // TODO(garyq): Implement a more feature complete version of getExtractedText
  @Override
  public ExtractedText getExtractedText(ExtractedTextRequest request, int flags) {
    // Input methods may use this method to get the current content of the
    final boolean textMonitor = (flags & GET_EXTRACTED_TEXT_MONITOR) != 0;
    if (textMonitor == (mExtractRequest == null)) {
      Log.d(TAG, "The input method toggled text monitoring " + (textMonitor ? "on" : "off"));
    }
    if (textMonitor) {
      // Enables text monitoring. See updateIMMIfNeeded.
      mExtractRequest = request;
    } else {
      mExtractRequest = null;
    }
    ExtractedText extractedText = getExtractedText(new TextEditingValue(mEditable));
    return extractedText;
  }

  @Override
  public boolean requestCursorUpdates(int cursorUpdateMode) {
    //

    if ((cursorUpdateMode & CURSOR_UPDATE_IMMEDIATE) != 0) {
      mImm.updateCursorAnchorInfo(
          mFlutterView, getCursorAnchorInfo(new TextEditingValue(mEditable)));
    }

    // Enables cursor monitoring.
    mMonitorCursorUpdate = (cursorUpdateMode & CURSOR_UPDATE_MONITOR) != 0;
    return true;
  }

  @Override
  public boolean clearMetaKeyStates(int states) {
    boolean result = super.clearMetaKeyStates(states);
    return result;
  }

  // Detect if the keyboard is a Samsung keyboard, where we apply Samsung-specific hacks to
  // fix critical bugs that make the keyboard otherwise unusable. See finishComposingText() for
  // more details.
  @SuppressLint("NewApi") // New API guard is inline, the linter can't see it.
  @SuppressWarnings("deprecation")
  private boolean isSamsung() {
    InputMethodSubtype subtype = mImm.getCurrentInputMethodSubtype();
    // Impacted devices all shipped with Android Lollipop or newer.
    if (subtype == null
        || Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP
        || !Build.MANUFACTURER.equals("samsung")) {
      return false;
    }
    String keyboardName =
        Settings.Secure.getString(
            mFlutterView.getContext().getContentResolver(), Settings.Secure.DEFAULT_INPUT_METHOD);
    // The Samsung keyboard is called "com.sec.android.inputmethod/.SamsungKeypad" but look
    // for "Samsung" just in case Samsung changes the name of the keyboard.
    return keyboardName.contains("Samsung");
  }

  @Override
  public boolean setSelection(int start, int end) {
    beginBatchEdit();
    boolean result = super.setSelection(start, end);
    endBatchEdit();
    return result;
  }

  // Sanitizes the index to ensure the index is within the range of the
  // contents of editable.
  private static int clampIndexToEditable(int index, Editable editable) {
    int clamped = Math.max(0, Math.min(editable.length(), index));
    if (clamped != index) {
      Log.d(
          "flutter",
          "Text selection index was clamped ("
              + index
              + "->"
              + clamped
              + ") to remain in bounds. This may not be your fault, as some keyboards may select outside of bounds.");
    }
    return clamped;
  }

  @Override
  public boolean sendKeyEvent(KeyEvent event) {
    beginBatchEdit();
    final boolean result = doSendKeyEvent(event);
    endBatchEdit();
    return result;
  }

  private boolean doSendKeyEvent(KeyEvent event) {
    if (event.getAction() == KeyEvent.ACTION_DOWN) {
      if (event.getKeyCode() == KeyEvent.KEYCODE_DEL) {
        int selStart = clampIndexToEditable(Selection.getSelectionStart(mEditable), mEditable);
        int selEnd = clampIndexToEditable(Selection.getSelectionEnd(mEditable), mEditable);
        if (selStart == selEnd && selStart > 0) {
          // Extend selection to left of the last character
          selStart = flutterTextUtils.getOffsetBefore(mEditable, selStart);
        }
        if (selEnd > selStart) {
          // Delete the selection.
          Selection.setSelection(mEditable, selStart);
          mEditable.delete(selStart, selEnd);
          return true;
        }
        return false;
      } else if (event.getKeyCode() == KeyEvent.KEYCODE_DPAD_LEFT) {
        int selStart = Selection.getSelectionStart(mEditable);
        int selEnd = Selection.getSelectionEnd(mEditable);
        if (selStart == selEnd && !event.isShiftPressed()) {
          int newSel = Math.max(flutterTextUtils.getOffsetBefore(mEditable, selStart), 0);
          setSelection(newSel, newSel);
        } else {
          int newSelEnd = Math.max(flutterTextUtils.getOffsetBefore(mEditable, selEnd), 0);
          setSelection(selStart, newSelEnd);
        }
        return true;
      } else if (event.getKeyCode() == KeyEvent.KEYCODE_DPAD_RIGHT) {
        int selStart = Selection.getSelectionStart(mEditable);
        int selEnd = Selection.getSelectionEnd(mEditable);
        if (selStart == selEnd && !event.isShiftPressed()) {
          int newSel =
              Math.min(flutterTextUtils.getOffsetAfter(mEditable, selStart), mEditable.length());
          setSelection(newSel, newSel);
        } else {
          int newSelEnd =
              Math.min(flutterTextUtils.getOffsetAfter(mEditable, selEnd), mEditable.length());
          setSelection(selStart, newSelEnd);
        }
        return true;
      } else if (event.getKeyCode() == KeyEvent.KEYCODE_DPAD_UP) {
        int selStart = Selection.getSelectionStart(mEditable);
        int selEnd = Selection.getSelectionEnd(mEditable);
        if (selStart == selEnd && !event.isShiftPressed()) {
          Selection.moveUp(mEditable, mLayout);
          int newSelStart = Selection.getSelectionStart(mEditable);
          setSelection(newSelStart, newSelStart);
        } else {
          Selection.extendUp(mEditable, mLayout);
          int newSelStart = Selection.getSelectionStart(mEditable);
          int newSelEnd = Selection.getSelectionEnd(mEditable);
          setSelection(newSelStart, newSelEnd);
        }
        return true;
      } else if (event.getKeyCode() == KeyEvent.KEYCODE_DPAD_DOWN) {
        int selStart = Selection.getSelectionStart(mEditable);
        int selEnd = Selection.getSelectionEnd(mEditable);
        if (selStart == selEnd && !event.isShiftPressed()) {
          Selection.moveDown(mEditable, mLayout);
          int newSelStart = Selection.getSelectionStart(mEditable);
          setSelection(newSelStart, newSelStart);
        } else {
          Selection.extendDown(mEditable, mLayout);
          int newSelStart = Selection.getSelectionStart(mEditable);
          int newSelEnd = Selection.getSelectionEnd(mEditable);
          setSelection(newSelStart, newSelEnd);
        }
        return true;
        // When the enter key is pressed on a non-multiline field, consider it a
        // submit instead of a newline.
      } else if ((event.getKeyCode() == KeyEvent.KEYCODE_ENTER
              || event.getKeyCode() == KeyEvent.KEYCODE_NUMPAD_ENTER)
          && (InputType.TYPE_TEXT_FLAG_MULTI_LINE & mEditorInfo.inputType) == 0) {
        performEditorAction(mEditorInfo.imeOptions & EditorInfo.IME_MASK_ACTION);
        return true;
      } else {
        // Enter a character.
        int character = event.getUnicodeChar();
        if (character == 0) {
          return false;
        }
        int selStart = Math.max(0, Selection.getSelectionStart(mEditable));
        int selEnd = Math.max(0, Selection.getSelectionEnd(mEditable));
        int selMin = Math.min(selStart, selEnd);
        int selMax = Math.max(selStart, selEnd);
        if (selMin != selMax) mEditable.delete(selMin, selMax);
        mEditable.insert(selMin, String.valueOf((char) character));
        setSelection(selMin + 1, selMin + 1);
        return true;
      }
    }
    if (event.getAction() == KeyEvent.ACTION_UP
        && (event.getKeyCode() == KeyEvent.KEYCODE_SHIFT_LEFT
            || event.getKeyCode() == KeyEvent.KEYCODE_SHIFT_RIGHT)) {
      int selEnd = Selection.getSelectionEnd(mEditable);
      setSelection(selEnd, selEnd);
      return true;
    }
    return false;
  }

  @Override
  public boolean performContextMenuAction(int id) {
    beginBatchEdit();
    final boolean result = doPerformContextMenuAction(id);
    endBatchEdit();
    return result;
  }

  private boolean doPerformContextMenuAction(int id) {
    if (id == android.R.id.selectAll) {
      setSelection(0, mEditable.length());
      return true;
    } else if (id == android.R.id.cut) {
      int selStart = Selection.getSelectionStart(mEditable);
      int selEnd = Selection.getSelectionEnd(mEditable);
      if (selStart != selEnd) {
        int selMin = Math.min(selStart, selEnd);
        int selMax = Math.max(selStart, selEnd);
        CharSequence textToCut = mEditable.subSequence(selMin, selMax);
        ClipboardManager clipboard =
            (ClipboardManager)
                mFlutterView.getContext().getSystemService(Context.CLIPBOARD_SERVICE);
        ClipData clip = ClipData.newPlainText("text label?", textToCut);
        clipboard.setPrimaryClip(clip);
        mEditable.delete(selMin, selMax);
        setSelection(selMin, selMin);
      }
      return true;
    } else if (id == android.R.id.copy) {
      int selStart = Selection.getSelectionStart(mEditable);
      int selEnd = Selection.getSelectionEnd(mEditable);
      if (selStart != selEnd) {
        CharSequence textToCopy =
            mEditable.subSequence(Math.min(selStart, selEnd), Math.max(selStart, selEnd));
        ClipboardManager clipboard =
            (ClipboardManager)
                mFlutterView.getContext().getSystemService(Context.CLIPBOARD_SERVICE);
        clipboard.setPrimaryClip(ClipData.newPlainText("text label?", textToCopy));
      }
      return true;
    } else if (id == android.R.id.paste) {
      ClipboardManager clipboard =
          (ClipboardManager) mFlutterView.getContext().getSystemService(Context.CLIPBOARD_SERVICE);
      ClipData clip = clipboard.getPrimaryClip();
      if (clip != null) {
        CharSequence textToPaste = clip.getItemAt(0).coerceToText(mFlutterView.getContext());
        int selStart = Math.max(0, Selection.getSelectionStart(mEditable));
        int selEnd = Math.max(0, Selection.getSelectionEnd(mEditable));
        int selMin = Math.min(selStart, selEnd);
        int selMax = Math.max(selStart, selEnd);
        if (selMin != selMax) mEditable.delete(selMin, selMax);
        mEditable.insert(selMin, textToPaste);
        int newSelStart = selMin + textToPaste.length();
        setSelection(newSelStart, newSelStart);
      }
      return true;
    }
    return false;
  }

  @Override
  public boolean performPrivateCommand(String action, Bundle data) {
    textInputChannel.performPrivateCommand(mClient, action, data);
    return true;
  }

  @Override
  public boolean performEditorAction(int actionCode) {
    switch (actionCode) {
      case EditorInfo.IME_ACTION_NONE:
        textInputChannel.newline(mClient);
        break;
      case EditorInfo.IME_ACTION_UNSPECIFIED:
        textInputChannel.unspecifiedAction(mClient);
        break;
      case EditorInfo.IME_ACTION_GO:
        textInputChannel.go(mClient);
        break;
      case EditorInfo.IME_ACTION_SEARCH:
        textInputChannel.search(mClient);
        break;
      case EditorInfo.IME_ACTION_SEND:
        textInputChannel.send(mClient);
        break;
      case EditorInfo.IME_ACTION_NEXT:
        textInputChannel.next(mClient);
        break;
      case EditorInfo.IME_ACTION_PREVIOUS:
        textInputChannel.previous(mClient);
        break;
      default:
      case EditorInfo.IME_ACTION_DONE:
        textInputChannel.done(mClient);
        break;
    }
    return true;
  }
}
