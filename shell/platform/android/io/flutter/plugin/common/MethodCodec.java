// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
public interface MethodCodec extends MessageCodec<Object> {
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
