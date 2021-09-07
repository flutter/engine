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
public final class TextEditingDelta {
  private CharSequence oldText;
  private CharSequence deltaText;
  private int deltaStart;
  private int deltaEnd;
  private int newSelectionStart;
  private int newSelectionEnd;
  private int newComposingStart;
  private int newComposingEnd;

  private static final String TAG = "TextEditingDelta";

  public TextEditingDelta(
      CharSequence oldEditable,
      int start,
      int end,
      CharSequence tb,
      int selectionStart,
      int selectionEnd,
      int composingStart,
      int composingEnd) {
    newSelectionStart = selectionStart;
    newSelectionEnd = selectionEnd;
    newComposingStart = composingStart;
    newComposingEnd = composingEnd;

    setDeltas(oldEditable, tb.toString(), start, end);
  }

  @VisibleForTesting
  public TextEditingDelta(
      CharSequence oldText,
      CharSequence deltaText,
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

    setDeltas(oldText, deltaText, deltaStart, deltaEnd);
  }

  public CharSequence getOldText() {
    return oldText;
  }

  public CharSequence getDeltaText() {
    return deltaText;
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

  private void setDeltas(CharSequence oldTxt, CharSequence newTxt, int newStart, int newExtent) {
    oldText = oldTxt;
    deltaText = newTxt;
    deltaStart = newStart;
    deltaEnd = newExtent;
  }

  public JSONObject toJSON() {
    JSONObject delta = new JSONObject();

    try {
      delta.put("oldText", oldText.toString());
      delta.put("deltaText", deltaText.toString());
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
