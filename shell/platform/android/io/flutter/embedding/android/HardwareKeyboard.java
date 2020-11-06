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

  public List<KeyDatum> convertEvent(KeyEvent event) {
    final long logicalKey = logicalKeyFromEvent(event);
    final long physicalKey = logicalKeyFromEvent(event);
    final boolean isPhysicalDown = event.getAction() == KeyEvent.ACTION_DOWN;

    final LogicalKeyDatum logicalDatum = new LogicalKeyDatum(
      isPhysicalDown ? EventKind.down : EventKind.up,
      logicalKey,
      null,
      false);

    final List<LogicalKeyDatum> logicalData = new ArrayList<LogicalKeyDatum>();
    logicalData.add(logicalDatum);

    final KeyDatum physicalDatum = new KeyDatum(
      isPhysicalDown ? EventKind.down : EventKind.up,
      logicalKey,
      false,
      0,
      logicalData);

    final List<KeyDatum> physicalData = new ArrayList<KeyDatum>();
    physicalData.add(physicalDatum);
    return physicalData;
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
    sync,
    cancel,
  }

  public static class LogicalKeyDatum {
    @NonNull public final EventKind kind;
    @NonNull public final long key;
    @Nullable public final String character;
    @NonNull public final boolean repeated;

    public LogicalKeyDatum(
        @NonNull EventKind kind,
        @NonNull long key,
        @Nullable String character,
        @NonNull boolean repeated) {
      this.kind = kind;
      this.key = key;
      this.character = character;
      this.repeated = repeated;
    }
  }

  public static class KeyDatum {
    @NonNull public final EventKind kind;
    @NonNull public final long key;
    @NonNull public final boolean repeated;
    @NonNull public final long activeLocks;
    @NonNull public final List<LogicalKeyDatum> logicalData;

    public KeyDatum(
        @NonNull EventKind kind,
        @NonNull long key,
        @NonNull boolean repeated,
        @NonNull long activeLocks,
        @Nullable List<LogicalKeyDatum> logicalData) {
      this.kind = kind;
      this.key = key;
      this.repeated = repeated;
      this.activeLocks = activeLocks;
      this.logicalData = logicalData;
    }
  }
}
