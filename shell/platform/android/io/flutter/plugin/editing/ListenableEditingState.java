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

  private static final String TAG = "flutter";

  private int mBatchEditNestDepth = 0;
  private ArrayList<EditingStateWatcher> mListeners = new ArrayList<>();
  private ArrayList<EditingStateWatcher> mPendingListeners = new ArrayList<>();

  private String mToStringCache;

  private String mTextWhenBeginBatchEdit;
  private int mSelectionStartWhenBeginBatchEdit;
  private int mSelectionEndWhenBeginBatchEdit;
  private int mComposingStartWhenBeginBatchEdit;
  private int mComposingEndWhenBeginBatchEdit;

  private BaseInputConnection mDummyConnection;

  public ListenableEditingState(TextInputChannel.TextEditState configuration, View view) {
    super();
    if (configuration != null) {
      setEditingState(configuration);
    }

    Editable self = this;
    mDummyConnection =
        new BaseInputConnection(view, true) {
          @Override
          public Editable getEditable() {
            return self;
          }
        };
  }

  public void beginBatchEdit() {
    if (mBatchEditNestDepth == 0 && !mListeners.isEmpty()) {
      mTextWhenBeginBatchEdit = toString();
      mSelectionStartWhenBeginBatchEdit = getSelecionStart();
      mSelectionEndWhenBeginBatchEdit = getSelecionEnd();
      mComposingStartWhenBeginBatchEdit = getComposingStart();
      mComposingEndWhenBeginBatchEdit = getComposingEnd();
    }
    mBatchEditNestDepth++;
  }

  public void endBatchEdit() {
    mBatchEditNestDepth--;
    if (mBatchEditNestDepth != 0) {
      mBatchEditNestDepth = mBatchEditNestDepth < 0 ? 0 : mBatchEditNestDepth;
      return;
    }

    for (final EditingStateWatcher watcher : mPendingListeners) {
      watcher.didChangeEditingState(true, true, true);
    }

    if (!mListeners.isEmpty()) {
      Log.v(TAG, "didFinishBatchEdit with " + String.valueOf(mListeners.size()) + " listener(s)");
      final boolean textChanged = !toString().equals(mTextWhenBeginBatchEdit);
      final boolean selectionChanged =
          mSelectionStartWhenBeginBatchEdit != getSelecionStart()
              || mSelectionEndWhenBeginBatchEdit != getSelecionEnd();
      final boolean composingRegionChanged =
          mComposingStartWhenBeginBatchEdit != getComposingStart()
              || mComposingEndWhenBeginBatchEdit != getComposingEnd();
      if (textChanged || selectionChanged || composingRegionChanged) {
        for (int i = 0; i < mListeners.size(); i++) {
          mListeners
              .get(i)
              .didChangeEditingState(textChanged, selectionChanged, composingRegionChanged);
        }
      }
    }

    mListeners.addAll(mPendingListeners);
    mPendingListeners.clear();
  }

  public void setComposingRange(int composingStart, int composingEnd) {
    if (composingStart < 0 || composingStart >= composingEnd) {
      BaseInputConnection.removeComposingSpans(this);
    } else {
      mDummyConnection.setComposingRegion(composingStart, composingEnd);
    }
  }

  public void setEditingState(TextInputChannel.TextEditState newState) {
    beginBatchEdit();
    replace(0, length(), new SpannableStringBuilder(newState.text));

    if (newState.selectionStart >= 0 && newState.selectionEnd >= newState.selectionStart) {
      Selection.setSelection(this, newState.selectionStart, newState.selectionEnd);
    } else {
      Selection.removeSelection(this);
    }
    setComposingRange(newState.composingStart, newState.composingEnd);
    endBatchEdit();
  }

  public void addEditingStateListener(EditingStateWatcher listener) {
    // It is possible for a listener to get added during a batch edit. When that happens we always
    // notify the new listeners.
    // This does not check if the listener is already in the list of existing listeners.
    if (mBatchEditNestDepth > 0) {
      Log.w(TAG, "a listener was added to EditingState while a batch edit was in progress");
      mPendingListeners.add(listener);
    } else {
      mListeners.add(listener);
    }
  }

  public void removeEditingStateListener(EditingStateWatcher listener) {
    mListeners.remove(listener);
    if (mBatchEditNestDepth > 0) {
      mPendingListeners.remove(listener);
    }
  }

  @Override
  public SpannableStringBuilder replace(
      int start, int end, CharSequence tb, int tbstart, int tbend) {
    boolean textChanged = end - start != tbend - tbstart;
    for (int i = 0; i < end - start && !textChanged; i++) {
      textChanged |= charAt(start + i) != tb.charAt(tbstart + i);
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
        getSelecionStart() != selectionStart || getSelecionEnd() != selectionEnd;
    final boolean composingRegionChanged =
        getComposingStart() != composingStart || getComposingEnd() != composingEnd;
    if (textChanged || selectionChanged || composingRegionChanged) {
      for (int i = 0; i < mListeners.size(); i++) {
        mListeners
            .get(i)
            .didChangeEditingState(textChanged, selectionChanged, composingRegionChanged);
      }
    }
    return editable;
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
