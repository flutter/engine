// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;


import android.view.KeyEvent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

public class HardwareKeyboard {
  private static final String TAG = "HardwareKeyboard";

  private final HashMap<Long, Long> mPressingRecords = new HashMap<Long, Long>();

  public HardwareKeyboard() {
  }

  static long logicalKeyFromEvent(KeyEvent event) {
    final int keyCode = event.getKeyCode();
    return keyCode;
  }

  static long physicalKeyFromEvent(KeyEvent event) {
    final int scanCode = event.getScanCode();
    return scanCode;
  }

  private long getLastLogicalRecord(long physicalKey) {
    final Long objValue = mPressingRecords.get(Long.valueOf(physicalKey));
    if (objValue == null) {
      return 0;
    }
    return objValue.longValue();
  }

  public List<KeyDatum> convertEvent(KeyEvent event) {
    final long logicalKey = logicalKeyFromEvent(event);
    final long physicalKey = physicalKeyFromEvent(event);
    final boolean isPhysicalDown = event.getAction() == KeyEvent.ACTION_DOWN;
    final long timeStamp = event.getEventTime() * 1000; // Convert from milliseconds to microseconds.

    final long lastLogicalRecord = getLastLogicalRecord(physicalKey);

    EventKind change;

    if (isPhysicalDown) {
      if (lastLogicalRecord != 0) {
        // This physical key is being pressed according to the record.
        if (event.getRepeatCount() == 0) {
          // A non-repeated key has been pressed that has the exact physical key as
          // a currently pressed one, usually indicating multiple keyboards are
          // pressing keys with the same physical key, or the up event was lost
          // during a loss of focus. The down event is ignored.
          return new ArrayList<KeyDatum>();
        } else {
          // A normal repeated key.
          change = EventKind.repeat;
        }
      } else {
        // This physical key is not being pressed according to the record. It's a
        // normal down event, whether the system event is a repeat or not.
        change = EventKind.down;
      }
    } else { // isPhysicalDown false
      if (lastLogicalRecord == 0) {
        // The physical key has been released before. It indicates multiple
        // keyboards pressed keys with the same physical key. Ignore the up event.
        return new ArrayList<KeyDatum>();
      }

      change = EventKind.up;
    }

    Long nextLogicalRecord = null;
    switch (change) {
      case down:
        nextLogicalRecord = logicalKey;
        break;
      case up:
        nextLogicalRecord = null;
        break;
      case repeat:
        nextLogicalRecord = lastLogicalRecord;
        break;
    }
    if (nextLogicalRecord == null) {
      mPressingRecords.remove(physicalKey);
    } else {
      mPressingRecords.put(physicalKey, nextLogicalRecord);
    }

    final KeyDatum keyDatum = new KeyDatum(
      change,
      timeStamp,
      physicalKey,
      logicalKey,
      event.getCharacters(),
      false);

    final List<KeyDatum> keyData = new ArrayList<KeyDatum>();
    keyData.add(keyDatum);
    return keyData;
  }

  public ByteBuffer packDatum(KeyDatum keyDatum) {
    ByteBuffer packet =
        ByteBuffer.allocateDirect(1);
    packet.order(ByteOrder.LITTLE_ENDIAN);
    return packet;
  }

  public enum EventKind {
    down,
    up,
    repeat,
  }

  public static class KeyDatum {
    @NonNull public final EventKind kind;
    // Time in microseconds from an arbitrary and consistent start.
    @NonNull public final long timeStamp;
    @NonNull public final long physical;
    @NonNull public final long logical;
    @Nullable public final String character;
    @NonNull public final boolean synthesized;

    public KeyDatum(
        @NonNull EventKind kind,
        @NonNull long timeStamp,
        @NonNull long physical,
        @NonNull long logical,
        @Nullable String character,
        @NonNull boolean synthesized) {
      this.kind = kind;
      this.timeStamp = timeStamp;
      this.physical = physical;
      this.logical = logical;
      this.character = character;
      this.synthesized = synthesized;
    }
  }
}
