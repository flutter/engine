// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.io.UnsupportedEncodingException;

/**
 * A {@link KeyboardManager.Responder} of {@link KeyboardManager} that handles events by sending processed
 * information in {@link KeyData}.
 *
 * <p>This class corresponds to the HardwareKeyboard API in the framework.
 */
public class KeyData {
  private static final String TAG = "KeyData";

  private static final int FIELD_COUNT = 5;
  private static final int BYTES_PER_FIELD = 8;

  public enum Type {
    kDown(0),
    kUp(1),
    kRepeat(2);

    private int value;
    private Type(int value) {
      this.value = value;
    }
    public int getValue() {
      return value;
    }
  }

  public KeyData() {}

  long timestamp;
  Type type;
  long physicalKey;
  long logicalKey;
  boolean synthesized;

  // Nullable
  Character character;

  ByteBuffer toBytes() {
    byte[] charUtf8 = null;
    if (character != null) {
      final String charStr = "" + character;
      try {
        charUtf8 = charStr.getBytes("UTF8");
      } catch (UnsupportedEncodingException e) {
        throw new AssertionError("Decoding error");
      }
    }
    final int charSize = charUtf8 == null ? 0 : charUtf8.length;
    final ByteBuffer packet =
        ByteBuffer.allocateDirect((1 + FIELD_COUNT) * BYTES_PER_FIELD + charSize);
    packet.order(ByteOrder.LITTLE_ENDIAN);

    packet.putLong(charSize);
    packet.putLong(timestamp);
    packet.putLong(type.getValue());
    packet.putLong(physicalKey);
    packet.putLong(logicalKey);
    packet.putLong(synthesized ? 1l : 0l);
    if (charUtf8 != null) {
      packet.put(charUtf8);
    }

    // Verify that the packet is the expected size.
    if (packet.position() != packet.capacity()) {
      throw new AssertionError("Packet is not filled");
    }

    packet.rewind();
    return packet;
  }
}
