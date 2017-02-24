package io.flutter.plugin.common;

import java.nio.ByteBuffer;

/**
 * A message encoding/decoding mechanism with support for method calls and enveloped replies.
 *
 * Messages are semi-structured values whose static Java type is Object. Concrete codecs support
 * different types of messages.
 *
 * Method calls are encoded as binary messages with enough structure that the codec can
 * extract a method name String and an arguments Object. These data items are used to populate a
 * {@link MethodCall}.
 *
 * All operations throw {@link IllegalArgumentException}, if conversion fails.
 */
public interface MessageCodec {
    /**
     * MessageCodec using the Flutter standard binary message encoding.
     *
     * The standard codec is guaranteed to be compatible with the corresponding codec available for
     * PlatformChannels on the Flutter side; these two parts of the Flutter SDK are evolved
     * synchronously.
     *
     * Supported messages are acyclic values of these forms:
     *
     * <ul>
     *     <li>null</li>
     *     <li>Booleans</li>
     *     <li>Bytes, Shorts, Integers, Longs, BigIntegers</li>
     *     <li>Floats, Doubles</li>
     *     <li>Strings</li>
     *     <li>byte arrays and ByteBuffers</li>
     *     <li>Lists of supported values</li>
     *     <li>Maps with supported keys and values</li>
     * </ul>
     */
    MessageCodec STANDARD = new StandardCodec();

    /**
     * MessageCodec with UTF-8 encoded JSON messages.
     *
     * Supported messages are the values supported by {@link org.json.JSONObject#wrap(Object)}.
     */
    MessageCodec JSON = new JSONCodec();

    /**
     * Encodes the specified message into binary.
     *
     * @param message the T message, possibly null.
     * @return a ByteBuffer containing the encoding between position 0 and
     * the current position, or null, if message is null.
     */
    ByteBuffer encodeMessage(Object message);

    /**
     * Decodes the specified message from binary.
     *
     * @param message the {@link ByteBuffer} message, possibly null.
     * @return a T value representation of the bytes between the given buffer's current
     * position and its limit, or null, if message is null.
     */
    Object decodeMessage(ByteBuffer message);

    /**
     * Decodes a message call from binary.
     *
     * @param methodCall the binary encoding of the method call as a {@link ByteBuffer}, not null.
     * @return a {@link MethodCall} representation of the bytes between the given buffer's current
     * position and its limit, not null.
     */
    MethodCall decodeMethodCall(ByteBuffer methodCall);

    /**
     * Encodes a successful result into a binary envelope message.
     *
     * @param result The result value, possibly null.
     * @return a ByteBuffer containing the encoding between position 0 and
     * the current position.
     */
    ByteBuffer encodeSuccessEnvelope(Object result);

    /**
     * Encodes an error reply into a binary envelope message.
     *
     * @param errorCode An error code String, not null.
     * @param errorMessage An error message String, not null.
     * @param errorDetails Error details, possibly null.
     * @return a ByteBuffer containing the encoding between position 0 and
     * the current position.
     */
    ByteBuffer encodeErrorEnvelope(String errorCode, String errorMessage, Object errorDetails);
}
