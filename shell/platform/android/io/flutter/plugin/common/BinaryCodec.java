package io.flutter.plugin.common;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * A {@link MessageCodec} using unencoded binary messages, represented as {@link ByteBuffer}s.
 */
public final class BinaryCodec implements MessageCodec<ByteBuffer> {
    public static final BinaryCodec INSTANCE = new BinaryCodec();

    private BinaryCodec() {
    }

    @Override
    public ByteBuffer encodeMessage(ByteBuffer message) {
        return message;
    }

    @Override
    public ByteBuffer decodeMessage(ByteBuffer message) {
        return message;
    }
}
