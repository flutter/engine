// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.view.KeyEvent;
import androidx.annotation.IntDef;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

public class HardwareKeyboard {
  private static final String TAG = "HardwareKeyboard";

  // Must match the KeyChange enum in key.dart.
  @IntDef({
    KeyChange.DOWN,
    KeyChange.UP,
    KeyChange.REPEAT,
  })
  private @interface KeyChange {
    int DOWN = 0;
    int UP = 1;
    int REPEAT = 2;
  }

  // Must match _kKeyDataFieldCount in platform_dispatcher.dart.
  private static final int KEY_DATA_FIELD_COUNT = 6;
  private static final int BYTES_PER_FIELD = 8;

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

  // May return null
  public List<KeyDatum> convertEvent(KeyEvent event) {
    final long logicalKey = logicalKeyFromEvent(event);
    final long physicalKey = physicalKeyFromEvent(event);
    final boolean isPhysicalDown = event.getAction() == KeyEvent.ACTION_DOWN;
    final long timeStamp = event.getEventTime() * 1000; // Convert from milliseconds to microseconds.

    final long lastLogicalRecord = getLastLogicalRecord(physicalKey);

    int change;

    if (isPhysicalDown) {
      if (lastLogicalRecord != 0) {
        // This physical key is being pressed according to the record.
        if (event.getRepeatCount() == 0) {
          // A non-repeated key has been pressed that has the exact physical key as
          // a currently pressed one, usually indicating multiple keyboards are
          // pressing keys with the same physical key, or the up event was lost
          // during a loss of focus. The down event is ignored.
          return null;
        } else {
          // A normal repeated key.
          change = KeyChange.REPEAT;
        }
      } else {
        // This physical key is not being pressed according to the record. It's a
        // normal down event, whether the system event is a repeat or not.
        change = KeyChange.DOWN;
      }
    } else { // isPhysicalDown false
      if (lastLogicalRecord == 0) {
        // The physical key has been released before. It indicates multiple
        // keyboards pressed keys with the same physical key. Ignore the up event.
        return null;
      }

      change = KeyChange.UP;
    }

    Long nextLogicalRecord = null;
    switch (change) {
      case KeyChange.DOWN:
        nextLogicalRecord = logicalKey;
        break;
      case KeyChange.UP:
        nextLogicalRecord = null;
        break;
      case KeyChange.REPEAT:
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
    final ByteBuffer packet =
        ByteBuffer.allocateDirect(KEY_DATA_FIELD_COUNT * BYTES_PER_FIELD);
    packet.order(ByteOrder.LITTLE_ENDIAN);
    final byte[] bytes = keyDatum.character.getBytes(StandardCharsets.UTF_8);

    packet.putLong(bytes.length);

    packet.putLong(keyDatum.timeStamp);
    packet.putLong(keyDatum.change);
    packet.putLong(keyDatum.physical);
    packet.putLong(keyDatum.logical);
    final long synthesized = keyDatum.synthesized ? 1 : 0;
    packet.putLong(synthesized);
    return packet;
  }

  public static class KeyDatum {
    @NonNull public final int change;
    // Time in microseconds from an arbitrary and consistent start.
    @NonNull public final long timeStamp;
    @NonNull public final long physical;
    @NonNull public final long logical;
    @Nullable public final String character;
    @NonNull public final boolean synthesized;

    public KeyDatum(
        @NonNull int change,
        @NonNull long timeStamp,
        @NonNull long physical,
        @NonNull long logical,
        @Nullable String character,
        @NonNull boolean synthesized) {
      this.change = change;
      this.timeStamp = timeStamp;
      this.physical = physical;
      this.logical = logical;
      this.character = character;
      this.synthesized = synthesized;
    }
  }
}
