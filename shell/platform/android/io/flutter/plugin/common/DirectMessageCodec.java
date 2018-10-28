// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.common;

import android.util.Log;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

/**
 * MessageCodec using the Flutter standard binary encoding.
 *
 * <p>Compare with {@link StandardMessageCodec}, this class is more suitable for frequent passing on
 * large objects. This class avoid to use ByteArrayOutputStream which cause an extra replication of
 * byte array. Instead, it compute the estimated size first then allocate ByteBuffer directly.</p>
 *
 * <p>This codec is guaranteed to be compatible with the corresponding
 * <a href="https://docs.flutter.io/flutter/services/DirectMessageCodec-class.html">DirectMessageCodec</a>
 * on the Dart side. These parts of the Flutter SDK are evolved synchronously.</p>
 *
 * <p>Supported messages are acyclic values of these forms:</p>
 *
 * <ul>
 *     <li>null</li>
 *     <li>Booleans</li>
 *     <li>Bytes, Shorts, Integers, Longs</li>
 *     <li>Floats, Doubles</li>
 *     <li>Strings</li>
 *     <li>byte[], int[], long[], double[]</li>
 *     <li>Lists of supported values</li>
 *     <li>Maps with supported keys and values</li>
 * </ul>
 *
 * <p>On the Dart side, these values are represented as follows:</p>
 *
 * <ul>
 *     <li>null: null</li>
 *     <li>Boolean: bool</li>
 *     <li>Byte, Short, Integer, Long: int</li>
 *     <li>Float, Double: double</li>
 *     <li>String: String</li>
 *     <li>byte[]: Uint8List</li>
 *     <li>int[]: Int32List</li>
 *     <li>long[]: Int64List</li>
 *     <li>double[]: Float64List</li>
 *     <li>List: List</li>
 *     <li>Map: Map</li>
 * </ul>
 *
 * <p>Direct support for BigIntegers has been deprecated on 2018-01-09 to be made
 * unavailable four weeks after this change is available on the Flutter alpha
 * branch. BigIntegers were needed because the Dart 1.0 int type had no size
 * limit. With Dart 2.0, the int type is a fixed-size, 64-bit signed integer.
 * If you need to communicate larger integers, use String encoding instead.</p>
 *
 * <p>To extend the codec, overwrite the writeValue, computeValueSize and readValueOfType methods.</p>
 */
public class DirectMessageCodec implements MessageCodec<Object> {
    public static final DirectMessageCodec INSTANCE = new DirectMessageCodec();

    @Override
    public ByteBuffer encodeMessage(Object message) {
        if (message == null) {
            return null;
        }
        final ByteBuffer buffer = ByteBuffer.allocateDirect(computeValueSize(0, message));
        writeValue(buffer, message);
        return buffer;
    }

    @Override
    public Object decodeMessage(ByteBuffer message) {
        if (message == null) {
            return null;
        }
        message.order(ByteOrder.nativeOrder());
        final Object value = readValue(message);
        if (message.hasRemaining()) {
            throw new IllegalArgumentException("Message corrupted");
        }
        return value;
    }

    private static final boolean LITTLE_ENDIAN = ByteOrder.nativeOrder() == ByteOrder.LITTLE_ENDIAN;
    private static final Charset UTF8 = Charset.forName("UTF8");
    private static final byte NULL = 0;
    private static final byte TRUE = 1;
    private static final byte FALSE = 2;
    private static final byte INT = 3;
    private static final byte LONG = 4;
    @Deprecated
    private static final byte BIGINT = 5;
    private static final byte DOUBLE = 6;
    private static final byte STRING = 7;
    private static final byte BYTE_ARRAY = 8;
    private static final byte INT_ARRAY = 9;
    private static final byte LONG_ARRAY = 10;
    private static final byte DOUBLE_ARRAY = 11;
    private static final byte LIST = 12;
    private static final byte MAP = 13;

    /**
     * Writes an int representing a size to the specified stream.
     * Uses an expanding code of 1 to 5 bytes to optimize for small values.
     */
    protected static final void writeSize(ByteBuffer byteBuffer, int value) {
        assert 0 <= value;
        if (value < 254) {
            byteBuffer.put((byte) value);
        } else if (value <= 0xffff) {
            byteBuffer.put((byte) 254);
            writeChar(byteBuffer, value);
        } else {
            byteBuffer.put((byte) 255);
            writeInt(byteBuffer, value);
        }
    }

    /**
     * Writes the least significant two bytes of the specified int to the
     * specified stream.
     */
    protected static final void writeChar(ByteBuffer byteBuffer, int value) {
        if (LITTLE_ENDIAN) {
            byteBuffer.put((byte) value);
            byteBuffer.put((byte) (value >>> 8));
        } else {
            byteBuffer.put((byte) (value >>> 8));
            byteBuffer.put((byte) value);
        }
    }

    /**
     * Writes the specified int as 4 bytes to the specified stream.
     */
    protected static final void writeInt(ByteBuffer byteBuffer, int value) {
        if (LITTLE_ENDIAN) {
            byteBuffer.put((byte) value);
            byteBuffer.put((byte) (value >>> 8));
            byteBuffer.put((byte) (value >>> 16));
            byteBuffer.put((byte) (value >>> 24));
        } else {
            byteBuffer.put((byte) (value >>> 24));
            byteBuffer.put((byte) (value >>> 16));
            byteBuffer.put((byte) (value >>> 8));
            byteBuffer.put((byte) value);
        }
    }

    /**
     * Writes the specified long as 8 bytes to the specified stream.
     */
    protected static final void writeLong(ByteBuffer byteBuffer, long value) {
        if (LITTLE_ENDIAN) {
            byteBuffer.put((byte) value);
            byteBuffer.put((byte) (value >>> 8));
            byteBuffer.put((byte) (value >>> 16));
            byteBuffer.put((byte) (value >>> 24));
            byteBuffer.put((byte) (value >>> 32));
            byteBuffer.put((byte) (value >>> 40));
            byteBuffer.put((byte) (value >>> 48));
            byteBuffer.put((byte) (value >>> 56));
        } else {
            byteBuffer.put((byte) (value >>> 56));
            byteBuffer.put((byte) (value >>> 48));
            byteBuffer.put((byte) (value >>> 40));
            byteBuffer.put((byte) (value >>> 32));
            byteBuffer.put((byte) (value >>> 24));
            byteBuffer.put((byte) (value >>> 16));
            byteBuffer.put((byte) (value >>> 8));
            byteBuffer.put((byte) value);
        }
    }

    /**
     * Writes the specified double as 8 bytes to the specified stream.
     */
    protected static final void writeDouble(ByteBuffer byteBuffer, double value) {
        writeLong(byteBuffer, Double.doubleToLongBits(value));
    }

    /**
     * Writes the length and then the actual bytes of the specified array to
     * the specified stream.
     */
    protected static final void writeBytes(ByteBuffer byteBuffer, byte[] bytes) {
        writeSize(byteBuffer, bytes.length);
        byteBuffer.put(bytes, 0, bytes.length);
    }

    /**
     * Writes a number of padding bytes to the specified stream to ensure that
     * the next value is aligned to a whole multiple of the specified alignment.
     * An example usage with alignment = 8 is to ensure doubles are word-aligned
     * in the stream.
     */
    protected static final void writeAlignment(ByteBuffer byteBuffer, int alignment) {
        final int mod = byteBuffer.position() % alignment;
        if (mod != 0) {
            for (int i = 0; i < alignment - mod; i++) {
                byteBuffer.put((byte) 0);
            }
        }
    }

    /**
     * Writes a type discriminator byte and then a byte serialization of the
     * specified value to the specified stream.
     *
     * <p>Subclasses can extend the codec by overriding this method, calling
     * super for values that the extension does not handle.</p>
     */
    protected void writeValue(ByteBuffer byteBuffer, Object value) {
        if (value == null) {
            byteBuffer.put(NULL);
        } else if (value == Boolean.TRUE) {
            byteBuffer.put(TRUE);
        } else if (value == Boolean.FALSE) {
            byteBuffer.put(FALSE);
        } else if (value instanceof Number) {
            if (value instanceof Integer || value instanceof Short || value instanceof Byte) {
                byteBuffer.put(INT);
                writeInt(byteBuffer, ((Number) value).intValue());
            } else if (value instanceof Long) {
                byteBuffer.put(LONG);
                writeLong(byteBuffer, (long) value);
            } else if (value instanceof Float || value instanceof Double) {
                byteBuffer.put(DOUBLE);
                writeAlignment(byteBuffer, 8);
                writeDouble(byteBuffer, ((Number) value).doubleValue());
            } else if (value instanceof BigInteger) {
                Log.w("Flutter", "Support for BigIntegers has been deprecated. Use String encoding instead.");
                byteBuffer.put(BIGINT);
                writeBytes(byteBuffer,
                        ((BigInteger) value).toString(16).getBytes(UTF8));
            } else {
                throw new IllegalArgumentException("Unsupported Number type: " + value.getClass());
            }
        } else if (value instanceof String) {
            byteBuffer.put(STRING);
            writeBytes(byteBuffer, ((String) value).getBytes(UTF8));
        } else if (value instanceof byte[]) {
            byteBuffer.put(BYTE_ARRAY);
            writeBytes(byteBuffer, (byte[]) value);
        } else if (value instanceof int[]) {
            byteBuffer.put(INT_ARRAY);
            final int[] array = (int[]) value;
            writeSize(byteBuffer, array.length);
            writeAlignment(byteBuffer, 4);
            for (final int n : array) {
                writeInt(byteBuffer, n);
            }
        } else if (value instanceof long[]) {
            byteBuffer.put(LONG_ARRAY);
            final long[] array = (long[]) value;
            writeSize(byteBuffer, array.length);
            writeAlignment(byteBuffer, 8);
            for (final long n : array) {
                writeLong(byteBuffer, n);
            }
        } else if (value instanceof double[]) {
            byteBuffer.put(DOUBLE_ARRAY);
            final double[] array = (double[]) value;
            writeSize(byteBuffer, array.length);
            writeAlignment(byteBuffer, 8);
            for (final double d : array) {
                writeDouble(byteBuffer, d);
            }
        } else if (value instanceof List) {
            byteBuffer.put(LIST);
            final List<?> list = (List) value;
            writeSize(byteBuffer, list.size());
            for (final Object o : list) {
                writeValue(byteBuffer, o);
            }
        } else if (value instanceof Map) {
            byteBuffer.put(MAP);
            final Map<?, ?> map = (Map) value;
            writeSize(byteBuffer, map.size());
            for (final Entry entry: map.entrySet()) {
                writeValue(byteBuffer, entry.getKey());
                writeValue(byteBuffer, entry.getValue());
            }
        } else {
            throw new IllegalArgumentException("Unsupported value: " + value);
        }
    }

    /**
     * Reads an int representing a size as written by writeSize.
     */
    protected static final int readSize(ByteBuffer buffer) {
        if (!buffer.hasRemaining()) {
            throw new IllegalArgumentException("Message corrupted");
        }
        final int value = buffer.get() & 0xff;
        if (value < 254) {
            return value;
        } else if (value == 254) {
            return buffer.getChar();
        } else {
            return buffer.getInt();
        }
    }

    /**
     * Reads a byte array as written by writeBytes.
     */
    protected static final byte[] readBytes(ByteBuffer buffer) {
        final int length = readSize(buffer);
        final byte[] bytes = new byte[length];
        buffer.get(bytes);
        return bytes;
    }

    /**
     * Reads alignment padding bytes as written by writeAlignment.
     */
    protected static final void readAlignment(ByteBuffer buffer, int alignment) {
        final int mod = buffer.position() % alignment;
        if (mod != 0) {
            buffer.position(buffer.position() + alignment - mod);
        }
    }

    /**
     * Reads a value as written by writeValue.
     */
    protected final Object readValue(ByteBuffer buffer) {
        if (!buffer.hasRemaining()) {
            throw new IllegalArgumentException("Message corrupted");
        }
        final byte type = buffer.get();
        return readValueOfType(type, buffer);
    }

    /**
     * Reads a value of the specified type.
     *
     * <p>Subclasses may extend the codec by overriding this method, calling
     * super for types that the extension does not handle.</p>
     */
    protected Object readValueOfType(byte type, ByteBuffer buffer) {
        final Object result;
        switch (type) {
            case NULL:
                result = null;
                break;
            case TRUE:
                result = true;
                break;
            case FALSE:
                result = false;
                break;
            case INT:
                result = buffer.getInt();
                break;
            case LONG:
                result = buffer.getLong();
                break;
            case BIGINT: {
                Log.w("Flutter", "Support for BigIntegers has been deprecated. Use String encoding instead.");
                final byte[] hex = readBytes(buffer);
                result = new BigInteger(new String(hex, UTF8), 16);
                break;
            }
            case DOUBLE:
                readAlignment(buffer, 8);
                result = buffer.getDouble();
                break;
            case STRING: {
                final byte[] bytes = readBytes(buffer);
                result = new String(bytes, UTF8);
                break;
            }
            case BYTE_ARRAY: {
                result = readBytes(buffer);
                break;
            }
            case INT_ARRAY: {
                final int length = readSize(buffer);
                final int[] array = new int[length];
                readAlignment(buffer, 4);
                buffer.asIntBuffer().get(array);
                result = array;
                buffer.position(buffer.position() + 4 * length);
                break;
            }
            case LONG_ARRAY: {
                final int length = readSize(buffer);
                final long[] array = new long[length];
                readAlignment(buffer, 8);
                buffer.asLongBuffer().get(array);
                result = array;
                buffer.position(buffer.position() + 8 * length);
                break;
            }
            case DOUBLE_ARRAY: {
                final int length = readSize(buffer);
                final double[] array = new double[length];
                readAlignment(buffer, 8);
                buffer.asDoubleBuffer().get(array);
                result = array;
                buffer.position(buffer.position() + 8 * length);
                break;
            }
            case LIST: {
                final int size = readSize(buffer);
                final List<Object> list = new ArrayList<>(size);
                for (int i = 0; i < size; i++) {
                    list.add(readValue(buffer));
                }
                result = list;
                break;
            }
            case MAP: {
                final int size = readSize(buffer);
                final Map<Object, Object> map = new HashMap<>();
                for (int i = 0; i < size; i++) {
                    map.put(readValue(buffer), readValue(buffer));
                }
                result = map;
                break;
            }
            default: throw new IllegalArgumentException("Message corrupted");
        }
        return result;
    }

    /**
     * Compute byte size written by writeAlignment.
     */
    protected static final int computeAlignmentSize(int sizeUsed, int alignment) {
        final int mod = sizeUsed % alignment;
        return alignment - mod;
    }

    /**
     * Compute byte size written by writeSize.
     */
    protected static final int computeSize(int value) {
        if (value < 254) {
            return 1;
        } else if (value <= 0xffff) {
            return 1 + 2;
        } else {
            return 1 + 4;
        }
    }

    /**
     * Compute the size of writing a type discriminator byte and then a byte serialization of the
     * specified value.
     *
     * <p>Subclasses can extend the codec by overriding this method, calling
     * super for values that the extension does not handle.</p>
     */
    protected int computeValueSize(int sizeUsed, Object value) {
        if (value == null) {
            return 1;
        } else if (value == Boolean.TRUE) {
            return 1;
        } else if (value == Boolean.FALSE) {
            return 1;
        } else if (value instanceof Number) {
            if (value instanceof Integer || value instanceof Short || value instanceof Byte) {
                return 1 + 4;
            } else if (value instanceof Long) {
                return 1 + 8;
            } else if (value instanceof Float || value instanceof Double) {
                return 1 + computeAlignmentSize(sizeUsed + 1, 8) + 8;
            } else if (value instanceof BigInteger) {
                Log.w("Flutter", "Support for BigIntegers has been deprecated. Use String encoding instead.");
                int byteLength = ((BigInteger) value).toString(16).getBytes(UTF8).length;
                return 1 + computeSize(byteLength) + byteLength;
            } else {
                throw new IllegalArgumentException("Unsupported Number type: " + value.getClass());
            }
        } else if (value instanceof String) {
            int byteLength = ((String) value).getBytes(UTF8).length;
            return 1 + computeSize(byteLength) + byteLength;
        } else if (value instanceof byte[]) {
            int byteLength = ((byte[]) value).length;
            return 1 + computeSize(byteLength) + byteLength;
        } else if (value instanceof int[]) {
            final int[] array = (int[]) value;
            int addedSize = 1 + computeSize(array.length);
            return addedSize + computeAlignmentSize(sizeUsed + addedSize, 4) + array.length * 4;
        } else if (value instanceof long[]) {
            final long[] array = (long[]) value;
            int addedSize = 1 + computeSize(array.length);
            return addedSize + computeAlignmentSize(sizeUsed + addedSize, 8) + array.length * 8;
        } else if (value instanceof double[]) {
            final double[] array = (double[]) value;
            int addedSize = 1 + computeSize(array.length);
            return addedSize + computeAlignmentSize(sizeUsed + addedSize, 8) + array.length * 8;
        } else if (value instanceof List) {
            final List<?> list = (List) value;
            int addedSize = 1 + computeSize(list.size());
            for (final Object o : list) {
                addedSize += computeValueSize(sizeUsed + addedSize, o);
            }
            return addedSize;
        } else if (value instanceof Map) {
            final Map<?, ?> map = (Map) value;
            int addedSize = 1 + computeSize(map.size());
            for (final Entry entry: map.entrySet()) {
                addedSize += computeValueSize(sizeUsed + addedSize, entry.getKey());
                addedSize += computeValueSize(sizeUsed + addedSize, entry.getValue());
            }
            return addedSize;
        } else {
            throw new IllegalArgumentException("Unsupported value: " + value);
        }
    }

}
