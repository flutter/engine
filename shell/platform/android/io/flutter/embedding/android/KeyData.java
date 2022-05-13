// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import androidx.annotation.NonNull;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * A {@link KeyboardManager.Responder} of {@link KeyboardManager} that handles events by sending
 * processed information in {@link KeyData}.
 *
 * <p>This class corresponds to the HardwareKeyboard API in the framework.
 */
public class KeyData {
  private static final String TAG = "KeyData";

  // TODO(dkwingsmt): Doc
  public static final String CHANNEL = "flutter/keydata";

  // The number of fields except for `character`.
  private static final int FIELD_COUNT = 5;
  private static final int BYTES_PER_FIELD = 8;

  public enum Type {
    kDown(0),
    kUp(1),
    kRepeat(2);

    private long value;

    private Type(long value) {
      this.value = value;
    }

    public long getValue() {
      return value;
    }

    static Type fromLong(long value) {
      switch ((int) value) {
        case 0:
          return kDown;
        case 1:
          return kUp;
        case 2:
          return kRepeat;
        default:
          throw new AssertionError("Unexpected Type value");
      }
    }
  }

  public KeyData() {}

  public KeyData(@NonNull ByteBuffer buffer) {
    final long charSize = buffer.getLong();
    this.timestamp = buffer.getLong();
    this.type = Type.fromLong(buffer.getLong());
    this.physicalKey = buffer.getLong();
    this.logicalKey = buffer.getLong();
    this.synthesized = buffer.getLong() != 0;

    if (buffer.remaining() != charSize)
      throw new AssertionError(
          String.format(
              "Unexpected char length: charSize is %d while buffer has position %d, capacity %d, limit %d",
              charSize, buffer.position(), buffer.capacity(), buffer.limit()));
    this.character = null;
    if (charSize != 0) {
      final byte[] strBytes = new byte[(int) charSize];
      buffer.get(strBytes, 0, (int) charSize);
      try {
        this.character = new String(strBytes, "UTF-8");
      } catch (UnsupportedEncodingException e) {
        throw new AssertionError("UTF-8 unsupported");
      }
    }
  }

  long timestamp;
  Type type;
  long physicalKey;
  long logicalKey;
  boolean synthesized;

  // Nullable bytes of characters encoded in UTF8.
  String character;

  ByteBuffer toBytes() {
    byte[] charBytes;
    try {
      charBytes = character == null ? null : character.getBytes("UTF-8");
    } catch (UnsupportedEncodingException e) {
      throw new AssertionError("UTF-8 not supported");
    }
    final int charSize = charBytes == null ? 0 : charBytes.length;
    final ByteBuffer packet =
        ByteBuffer.allocateDirect((1 + FIELD_COUNT) * BYTES_PER_FIELD + charSize);
    packet.order(ByteOrder.LITTLE_ENDIAN);

    packet.putLong(charSize);
    packet.putLong(timestamp);
    packet.putLong(type.getValue());
    packet.putLong(physicalKey);
    packet.putLong(logicalKey);
    packet.putLong(synthesized ? 1l : 0l);
    if (charBytes != null) {
      packet.put(charBytes);
    }

    return packet;
  }
}
