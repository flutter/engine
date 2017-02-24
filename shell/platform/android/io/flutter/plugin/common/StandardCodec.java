package io.flutter.plugin.common;

import java.io.ByteArrayOutputStream;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

/**
 * A {@link MessageCodec} using binary messages in Flutter standard encoding.
 */
final class StandardCodec implements MessageCodec {
    @Override
    public ByteBuffer encodeMessage(Object message) {
        if (message == null) {
            return null;
        }
        final ExposedByteArrayOutputStream stream = new ExposedByteArrayOutputStream();
        writeValue(stream, message);
        final ByteBuffer buffer = ByteBuffer.allocateDirect(stream.size());
        buffer.put(stream.buffer(), 0, stream.size());
        return buffer;
    }

    @Override
    public Object decodeMessage(ByteBuffer message) {
        if (message == null || !message.hasRemaining()) {
            return null;
        }
        final Object value = readValue(message);
        if (message.hasRemaining()) {
            throw new IllegalArgumentException("Message corrupted");
        }
        return value;
    }

    @Override
    public MethodCall decodeMethodCall(ByteBuffer methodCall) {
        final Object method = readValue(methodCall);
        final Object arguments = readValue(methodCall);
        if (method instanceof String) {
            return new MethodCall((String) method, arguments);
        }
        throw new IllegalArgumentException("Method call corrupted");
    }

    @Override
    public ByteBuffer encodeSuccessEnvelope(Object result) {
        final ExposedByteArrayOutputStream stream = new ExposedByteArrayOutputStream();
        stream.write(0);
        writeValue(stream, result);
        final ByteBuffer buffer = ByteBuffer.allocateDirect(stream.size());
        buffer.put(stream.buffer(), 0, stream.size());
        return buffer;
    }

    @Override
    public ByteBuffer encodeErrorEnvelope(String errorCode, String errorMessage,
        Object errorDetails) {
        final ExposedByteArrayOutputStream stream = new ExposedByteArrayOutputStream();
        stream.write(1);
        writeValue(stream, errorCode);
        writeValue(stream, errorMessage);
        writeValue(stream, errorDetails);
        final ByteBuffer buffer = ByteBuffer.allocateDirect(stream.size());
        buffer.put(stream.buffer(), 0, stream.size());
        return buffer;
    }

    private static final byte NULL = 0;
    private static final byte TRUE = 1;
    private static final byte FALSE = 2;
    private static final byte INT32 = 3;
    private static final byte INT64 = 4;
    private static final byte BIGINT = 5;
    // static final byte FLOAT32 = 6; // Not currently used.
    private static final byte FLOAT64 = 7;
    private static final byte STRING = 8;
    // private static final byte REPEATED_STRING = 9; // Not currently used.
    private static final byte BYTES = 10;
    private static final byte LIST = 11;
    private static final byte MAP = 12;

    private static void writeSize(ByteArrayOutputStream stream, int value) {
        assert 0 <= value;
        if (value < 254) {
            stream.write(value);
        } else if (value < 0xffff) {
            stream.write(254);
            stream.write(value >>> 8);
            stream.write(value);
        } else {
            stream.write(255);
            writeInt(stream, value);
        }
    }

    private static void writeInt(ByteArrayOutputStream stream, int value) {
        stream.write(value >>> 24);
        stream.write(value >>> 16);
        stream.write(value >>> 8);
        stream.write(value);
    }

    private static void writeLong(ByteArrayOutputStream stream, long value) {
        stream.write((byte) (value >>> 56));
        stream.write((byte) (value >>> 48));
        stream.write((byte) (value >>> 40));
        stream.write((byte) (value >>> 32));
        stream.write((byte) (value >>> 24));
        stream.write((byte) (value >>> 16));
        stream.write((byte) (value >>> 8));
        stream.write((byte) value);
    }

    private static void writeBytes(ByteArrayOutputStream stream, byte[] bytes) {
        writeBytes(stream, bytes, 0, bytes.length);
    }

    private static void writeBytes(ByteArrayOutputStream stream, byte[] bytes, int offset, int length) {
        writeSize(stream, length);
        stream.write(bytes, offset, length);
    }

    private static void writeValue(ByteArrayOutputStream stream, Object value) {
        if (value == null) {
            stream.write(NULL);
        } else if (value == Boolean.TRUE) {
            stream.write(TRUE);
        } else if (value == Boolean.FALSE) {
            stream.write(FALSE);
        } else if (value instanceof Number) {
            if (value instanceof Float || value instanceof Double) {
                stream.write(FLOAT64);
                writeLong(stream, Double.doubleToLongBits(((Number) value).doubleValue()));
            } else if (value instanceof BigInteger) {
                stream.write(BIGINT);
                writeBytes(stream,
                    ((BigInteger) value).toString(16).getBytes(StandardCharsets.UTF_8));
            } else if (value instanceof Long) {
                stream.write(INT64);
                writeLong(stream, (long) value);
            } else {
                stream.write(INT32);
                writeInt(stream, ((Number) value).intValue());
            }
        } else if (value instanceof String) {
            stream.write(STRING);
            writeBytes(stream, ((String) value).getBytes(StandardCharsets.UTF_8));
        } else if (value instanceof byte[]) {
            stream.write(BYTES);
            writeBytes(stream, (byte[]) value);
        } else if (value instanceof ByteBuffer) {
            stream.write(BYTES);
            final ByteBuffer buffer = (ByteBuffer) value;
            final byte[] bytes;
            final int offset;
            final int length = buffer.remaining();
            if (buffer.hasArray()) {
                bytes = buffer.array();
                offset = buffer.arrayOffset();
            }
            else {
                bytes = new byte[length];
                buffer.get(bytes);
                offset = 0;
            }
            writeBytes(stream, bytes, offset, length);
        } else if (value instanceof List) {
            stream.write(LIST);
            final List<?> list = (List) value;
            writeSize(stream, list.size());
            for (final Object o : list) {
                writeValue(stream, o);
            }
        } else if (value instanceof Map) {
            stream.write(MAP);
            final Map<?, ?> map = (Map) value;
            writeSize(stream, map.size());
            for (final Entry entry: map.entrySet()) {
                writeValue(stream, entry.getKey());
                writeValue(stream, entry.getValue());
            }
        } else {
            throw new IllegalArgumentException("Unsupported value: " + value);
        }
    }

    private static int readSize(ByteBuffer buffer) {
        if (!buffer.hasRemaining()) {
            throw new IllegalArgumentException("Message corrupted");
        }
        final int value = buffer.get() & 0xff;
        if (value < 254) {
            return value;
        } else if (value == 254) {
            return ((buffer.get() & 0xff) << 8)
                 | ((buffer.get() & 0xff));
        } else {
            return ((buffer.get() & 0xff) << 24)
                 | ((buffer.get() & 0xff) << 16)
                 | ((buffer.get() & 0xff) << 8)
                 | ((buffer.get() & 0xff));
        }
    }

    private static Object readValue(ByteBuffer buffer) {
        if (!buffer.hasRemaining()) {
            throw new IllegalArgumentException("Message corrupted");
        }
        final Object result;
        switch (buffer.get()) {
            case NULL:
                result = null;
                break;
            case TRUE:
                result = true;
                break;
            case FALSE:
                result = false;
                break;
            case INT32:
                result = buffer.getInt();
                break;
            case INT64:
                result = buffer.getLong();
                break;
            case BIGINT: {
                final int length = readSize(buffer);
                final byte[] hex = new byte[length];
                buffer.get(hex);
                result = new BigInteger(new String(hex, StandardCharsets.UTF_8), 16);
                break;
            }
            case FLOAT64:
                result = buffer.getDouble();
                break;
            case STRING: {
                final int length = readSize(buffer);
                final byte[] bytes = new byte[length];
                buffer.get(bytes);
                result = new String(bytes, StandardCharsets.UTF_8);
                break;
            }
            case BYTES: {
                final int length = readSize(buffer);
                result = buffer.slice().limit(length);
                buffer.position(buffer.position() + length);
                break;
            }
            case LIST: {
                final int size = readSize(buffer);
                final List<Object> list = new ArrayList<Object>(size);
                for (int i = 0; i < size; i++) {
                    list.add(readValue(buffer));
                }
                result = list;
                break;
            }
            case MAP: {
                final int size = readSize(buffer);
                final Map<Object, Object> map = new HashMap<Object, Object>();
                for (int i = 0; i < size; i++) {
                    map.put(readValue(buffer), readValue(buffer));
                }
                result = map;
                break;
            }
            default:
                throw new IllegalArgumentException("Message corrupted");
        }
        return result;
    }

    private static final class ExposedByteArrayOutputStream extends ByteArrayOutputStream {
        public byte[] buffer() {
            return buf;
        }
    }
}
