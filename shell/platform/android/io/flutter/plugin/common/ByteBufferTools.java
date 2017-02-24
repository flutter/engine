package io.flutter.plugin.common;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * Tools for moving data between ByteBuffers and Java objects.
 */
public class ByteBufferTools {
    private ByteBufferTools() {
        // To prevent instantiation.
    }

    public static String extractString(ByteBuffer buffer) {
        if (buffer == null) {
            return null;
        }
        final byte[] bytes;
        final int offset;
        final int length = buffer.remaining();
        if (buffer.hasArray()) {
            bytes = buffer.array();
            offset = buffer.arrayOffset();
        } else {
            bytes = new byte[length];
            buffer.get(bytes);
            offset = 0;
        }
        return new String(bytes, offset, length, StandardCharsets.UTF_8);
    }

    public static ByteBuffer extractBytes(String s) {
        if (s == null) {
            return null;
        }
        final byte[] bytes = s.getBytes(StandardCharsets.UTF_8);
        final ByteBuffer buffer = ByteBuffer.allocateDirect(bytes.length);
        buffer.put(bytes);
        return buffer;
    }

}
