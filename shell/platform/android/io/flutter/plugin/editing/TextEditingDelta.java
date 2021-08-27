// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

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
      Log.v(TAG, "A TextEditingDelta for an EQUALITY has been created.");
      setDeltas(oldEditable, "", "EQUALITY", -1, -1);
    } else if (isCalledFromDelete || isDeletingInsideComposingRegion) {
      Log.v(TAG, "A TextEditingDelta for a DELETION has been created.");
      final int startOfDelete;
      if (isDeletionGreaterThanOne) {
        startOfDelete = start;
      } else {
        startOfDelete = end;
      }

      setDeltas(
          oldEditable,
          oldEditable.subSequence(start + tbend, end).toString(),
          "DELETION",
          startOfDelete,
          end);
    } else if ((start == end || isInsertingInsideComposingRegion)
        && !isOriginalComposingRegionTextChanged) {
      Log.v(TAG, "A TextEditingDelta for an INSERTION has been created.");
      setDeltas(oldEditable, tb.subSequence(end - start, tbend).toString(), "INSERTION", end, end);
    } else if (isReplaced) {
      Log.v(TAG, "A TextEditingDelta for a REPLACEMENT has been created.");
      setDeltas(oldEditable, tb.subSequence(tbstart, tbend).toString(), "REPLACEMENT", start, end);
    }
  }

  public void setNewComposingStart(int newStart) {
    newComposingStart = newStart;
  }

  public void setNewComposingEnd(int newEnd) {
    newComposingEnd = newEnd;
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

  private void setDeltas(
      CharSequence oldTxt, CharSequence newTxt, CharSequence type, int newStart, int newExtent) {
    oldText = oldTxt;
    deltaText = newTxt;
    deltaStart = newStart;
    deltaEnd = newExtent;
    deltaType = type;
  }
}
