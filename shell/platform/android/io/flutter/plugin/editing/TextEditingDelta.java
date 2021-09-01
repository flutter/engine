// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

import androidx.annotation.VisibleForTesting;
import io.flutter.Log;
import org.json.JSONException;
import org.json.JSONObject;

/// A representation of the change that occured to an editing state, along with the resulting
/// composing and selection regions.
public class TextEditingDelta {
  private CharSequence oldText;
  private CharSequence deltaText;
  private CharSequence deltaType;
  private int deltaStart;
  private int deltaEnd;
  private int newSelectionStart;
  private int newSelectionEnd;
  private int newComposingStart;
  private int newComposingEnd;

  private static final String TAG = "TextEditingDelta";

  public TextEditingDelta(
      CharSequence oldEditable,
      CharSequence newEditable,
      int start,
      int end,
      CharSequence tb,
      int tbstart,
      int tbend,
      int selectionStart,
      int selectionEnd,
      int composingStart,
      int composingEnd) {
    newSelectionStart = selectionStart;
    newSelectionEnd = selectionEnd;
    newComposingStart = composingStart;
    newComposingEnd = composingEnd;
    final boolean isDeletionGreaterThanOne = end - (start + tbend) > 1;
    final boolean isCalledFromDelete = tb == "" && tbstart == 0 && tbstart == tbend;

    final boolean isReplacedByShorter = isDeletionGreaterThanOne && (tbend - tbstart < end - start);
    final boolean isReplacedByLonger = tbend - tbstart > end - start;
    final boolean isReplacedBySame = tbend - tbstart == end - start;

    // Is deleting/inserting at the end of a composing region.
    final boolean isDeletingInsideComposingRegion = !isReplacedByShorter && start + tbend < end;
    final boolean isInsertingInsideComposingRegion = start + tbend > end;

    // To consider the cases when autocorrect increases the length of the text being composed by
    // one, but changes more than one character.
    final boolean isOriginalComposingRegionTextChanged =
        (isCalledFromDelete || isDeletingInsideComposingRegion || isReplacedByShorter)
            || !oldEditable
                .subSequence(start, end)
                .toString()
                .equals(tb.subSequence(tbstart, end - start).toString());

    final boolean isEqual = oldEditable.equals(newEditable);

    // A replacement means the original composing region has changed, anything else will be
    // considered an insertion.
    final boolean isReplaced =
        isOriginalComposingRegionTextChanged
            && (isReplacedByLonger || isReplacedBySame || isReplacedByShorter);

    if (isEqual) {
      Log.v(TAG, "A TextEditingDelta for an TextEditingDeltaType.equality has been created.");
      setDeltas(oldEditable, "", "TextEditingDeltaType.equality", -1, -1);
    } else if (isCalledFromDelete || isDeletingInsideComposingRegion) {
      Log.v(TAG, "A TextEditingDelta for a TextEditingDeltaType.deletion has been created.");
      final int startOfDelete;
      if (isDeletionGreaterThanOne) {
        startOfDelete = start;
      } else {
        startOfDelete = end - 1;
      }

      setDeltas(
          oldEditable,
          oldEditable.subSequence(start + tbend, end).toString(),
          "TextEditingDeltaType.deletion",
          startOfDelete,
          end);
    } else if ((start == end || isInsertingInsideComposingRegion)
        && !isOriginalComposingRegionTextChanged) {
      Log.v(TAG, "A TextEditingDelta for an TextEditingDeltaType.insertion has been created.");
      setDeltas(
          oldEditable,
          tb.subSequence(end - start, tbend).toString(),
          "TextEditingDeltaType.insertion",
          end,
          end);
    } else if (isReplaced) {
      Log.v(TAG, "A TextEditingDelta for a TextEditingDeltaType.replacement has been created.");
      setDeltas(
          oldEditable,
          tb.subSequence(tbstart, tbend).toString(),
          "TextEditingDeltaType.replacement",
          start,
          end);
    }
  }

  @VisibleForTesting
  public TextEditingDelta(
      CharSequence oldText,
      CharSequence deltaText,
      CharSequence deltaType,
      int deltaStart,
      int deltaEnd,
      int selectionStart,
      int selectionEnd,
      int composingStart,
      int composingEnd) {
    newSelectionStart = selectionStart;
    newSelectionEnd = selectionEnd;
    newComposingStart = composingStart;
    newComposingEnd = composingEnd;

    setDeltas(oldText, deltaText, deltaType, deltaStart, deltaEnd);
  }

  public void setNewComposingStart(int newStart) {
    newComposingStart = newStart;
  }

  public void setNewComposingEnd(int newEnd) {
    newComposingEnd = newEnd;
  }

  public CharSequence getOldText() {
    return oldText;
  }

  public CharSequence getDeltaText() {
    return deltaText;
  }

  public CharSequence getDeltaType() {
    return deltaType;
  }

  public int getDeltaStart() {
    return deltaStart;
  }

  public int getDeltaEnd() {
    return deltaEnd;
  }

  public int getNewSelectionStart() {
    return newSelectionStart;
  }

  public int getNewSelectionEnd() {
    return newSelectionEnd;
  }

  public int getNewComposingStart() {
    return newComposingStart;
  }

  public int getNewComposingEnd() {
    return newComposingEnd;
  }

  private void setDeltas(
      CharSequence oldTxt, CharSequence newTxt, CharSequence type, int newStart, int newExtent) {
    oldText = oldTxt;
    deltaText = newTxt;
    deltaStart = newStart;
    deltaEnd = newExtent;
    deltaType = type;
  }

  public JSONObject toJSON() {
    JSONObject delta = new JSONObject();

    try {
      delta.put("oldText", oldText.toString());
      delta.put("deltaText", deltaText.toString());
      delta.put("deltaType", deltaType.toString());
      delta.put("deltaStart", deltaStart);
      delta.put("deltaEnd", deltaEnd);
      delta.put("selectionBase", newSelectionStart);
      delta.put("selectionExtent", newSelectionEnd);
      delta.put("composingBase", newComposingStart);
      delta.put("composingExtent", newComposingEnd);
    } catch (JSONException e) {
      Log.e(TAG, "unable to create JSONObject: " + e);
    }

    return delta;
  }
}
