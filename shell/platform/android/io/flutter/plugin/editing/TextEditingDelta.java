// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

import io.flutter.Log;

public class TextEditingDelta {
  private CharSequence oldText;
  private CharSequence deltaText;
  private CharSequence deltaType;
  private int deltaStart;
  private int deltaEnd;

  public TextEditingDelta() {
    oldText = "";
    deltaText = "";
    deltaType = "EQUALITY";
    deltaStart = -1;
    deltaEnd = -1;
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

  private void setDeltas(
      CharSequence oldTxt, CharSequence newTxt, CharSequence type, int newStart, int newExtent) {
    oldText = oldTxt;
    deltaText = newTxt;
    deltaStart = newStart;
    deltaEnd = newExtent;
    deltaType = type;
  }

  public void reasonDeltas(
      CharSequence currentEditable, int start, int end, CharSequence tb, int tbstart, int tbend) {
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
            || !currentEditable
            .subSequence(start, end)
            .toString()
            .equals(tb.subSequence(tbstart, end - start).toString());

    final boolean isEqual =
        currentEditable.subSequence(start, end).equals(tb.toString().subSequence(tbstart, tbend));

    // A replacement means the original composing region has changed, anything else will be
    // considered an insertion.
    final boolean isReplaced =
        isOriginalComposingRegionTextChanged
            && (isReplacedByLonger || isReplacedBySame || isReplacedByShorter);

    if (isEqual) {
      Log.e("DELTAS", "EQUALITY");
      setDeltas(currentEditable, "", "EQUALITY", -1, -1);
    } else if (isCalledFromDelete || isDeletingInsideComposingRegion) {
      Log.e("DELTAS", "DELETION");
      setDeltas(
          currentEditable, currentEditable.subSequence(start + tbend, end).toString(), "DELETION", end, end);
    } else if ((start == end || isInsertingInsideComposingRegion)
        && !isOriginalComposingRegionTextChanged) {
      Log.e("DELTAS", "INSERTION");
      setDeltas(
          currentEditable, tb.subSequence(end - start, tbend).toString(), "INSERTION", end, end);
    } else if (isReplaced) {
      Log.e("DELTAS", "REPLACEMENT");
      setDeltas(
          currentEditable, tb.subSequence(tbstart, tbend).toString(), "REPLACEMENT", start, end);
    }
  }
}
