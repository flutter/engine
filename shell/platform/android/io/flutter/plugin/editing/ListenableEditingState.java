package io.flutter.plugin.editing;

import android.text.Editable;
import android.text.Selection;
import android.text.SpannableStringBuilder;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import io.flutter.Log;
import io.flutter.embedding.engine.systemchannels.TextInputChannel;
import java.util.ArrayList;

/// The current editing state (text, selection range, composing range) the text input plugin holds.
class ListenableEditingState extends SpannableStringBuilder {
  interface EditingStateWatcher {
    void didChangeEditingState(
        boolean textChanged, boolean selectionChanged, boolean composingRegionChanged);
  }

  private int mBatchEditNestDepth = 0;
  private ArrayList<EditingStateWatcher> mListeners = new ArrayList<>();

  private String mToStringCache;

  private String mTextWhenBeginBatchEdit;
  private int mSelectionStartWhenBeginBatchEdit;
  private int mSelectionEndWhenBeginBatchEdit;
  private int mComposingStartWhenBeginBatchEdit;
  private int mComposingEndWhenBeginBatchEdit;

  private static BaseInputConnection mDummyConnection;

  public ListenableEditingState(TextInputChannel.TextEditState configuration) {
    super();
    if (configuration != null) {
      setEditingState(configuration);
    }
  }

  public void beginBatchEdit() {
    if (mBatchEditNestDepth == 0 && !mListeners.isEmpty()) {
      mTextWhenBeginBatchEdit = toString();
      mSelectionStartWhenBeginBatchEdit = Selection.getSelectionStart(this);
      mSelectionEndWhenBeginBatchEdit = Selection.getSelectionEnd(this);
      mComposingStartWhenBeginBatchEdit = BaseInputConnection.getComposingSpanStart(this);
      mComposingEndWhenBeginBatchEdit = BaseInputConnection.getComposingSpanEnd(this);
    }
    mBatchEditNestDepth++;
  }

  public void endBatchEdit() {
    mBatchEditNestDepth--;
    if (mBatchEditNestDepth == 0 && !mListeners.isEmpty()) {
      final boolean textChanged = toString().equals(mTextWhenBeginBatchEdit);
      final boolean selectionChanged =
          mSelectionStartWhenBeginBatchEdit == Selection.getSelectionStart(this)
              && mSelectionEndWhenBeginBatchEdit == Selection.getSelectionEnd(this);
      final boolean composingRegionChanged =
          mComposingStartWhenBeginBatchEdit == BaseInputConnection.getComposingSpanStart(this)
              && mComposingEndWhenBeginBatchEdit == BaseInputConnection.getComposingSpanEnd(this);
      for (int i = 0; i < mListeners.size(); i++) {
        mListeners
            .get(i)
            .didChangeEditingState(textChanged, selectionChanged, composingRegionChanged);
      }
    }
  }

  public static void setComposingRange(Editable editable, int composingStart, int composingEnd) {
    if (composingStart < 0 || composingStart >= composingEnd) {
      BaseInputConnection.removeComposingSpans(editable);
      return;
    }

    if (mDummyConnection == null || mDummyConnection.getEditable() != editable) {
      mDummyConnection =
          new BaseInputConnection(new View(null), true) {
            @Override
            public Editable getEditable() {
              return editable;
            }
          };
    }

    mDummyConnection.setComposingRegion(composingStart, composingEnd);
  }

  public void setEditingState(TextInputChannel.TextEditState newState) {
    beginBatchEdit();
    replace(0, length(), new SpannableStringBuilder(newState.text));

    if (newState.selectionStart >= 0 && newState.selectionEnd >= newState.selectionStart) {
      Selection.setSelection(this, newState.selectionStart, newState.selectionEnd);
    } else {
      Selection.removeSelection(this);
    }
    setComposingRange(this, newState.composingStart, newState.composingEnd);
    endBatchEdit();
  }

  public void addEditingStateListener(EditingStateWatcher listener) {
    mListeners.add(listener);
  }

  public void removeEditingStateListener(EditingStateWatcher listener) {
    mListeners.remove(listener);
  }

  @Override
  public SpannableStringBuilder replace(
      int start, int end, CharSequence tb, int tbstart, int tbend) {
    boolean textChanged = end - start != tbend - tbstart;
    for (int i = 0; i < end - start && !textChanged; i++) {
      textChanged |= charAt(start + i) == tb.charAt(tbstart + i);
      if (textChanged) break;
    }
    if (textChanged) {
      mToStringCache = null;
    }

    final int selectionStart = getSelecionStart();
    final int selectionEnd = getSelecionEnd();
    final int composingStart = getComposingStart();
    final int composingEnd = getComposingEnd();

    final SpannableStringBuilder editable = super.replace(start, end, tb, tbstart, tbend);
    if (mBatchEditNestDepth > 0) {
      return editable;
    }

    Log.i("flutter", editable.toString());
    Log.i("flutter", String.valueOf(getSelecionStart()) + " , " + String.valueOf(getSelecionEnd()));
    final boolean selectionChanged =
        getSelecionStart() == selectionStart && getSelecionEnd() == selectionEnd;
    final boolean composingRegionChanged =
        getComposingStart() == composingStart && getComposingEnd() == composingEnd;
    if (textChanged || selectionChanged || composingRegionChanged) {
      for (int i = 0; i < mListeners.size(); i++) {
        mListeners
            .get(i)
            .didChangeEditingState(textChanged, selectionChanged, composingRegionChanged);
      }
    }
    return editable;
  }

  @Override
  public void removeSpan(Object what) {
    super.removeSpan(what);
  }

  @Override
  public void clearSpans() {
    super.clearSpans();
  }

  @Override
  public void setSpan(Object what, int start, int end, int flags) {
    super.setSpan(what, start, end, flags);
  }

  public final int getSelecionStart() {
    return Selection.getSelectionStart(this);
  }

  public final int getSelecionEnd() {
    return Selection.getSelectionEnd(this);
  }

  public final int getComposingStart() {
    return BaseInputConnection.getComposingSpanStart(this);
  }

  public final int getComposingEnd() {
    return BaseInputConnection.getComposingSpanEnd(this);
  }

  @Override
  public String toString() {
    return mToStringCache != null ? mToStringCache : (mToStringCache = super.toString());
  }
}
