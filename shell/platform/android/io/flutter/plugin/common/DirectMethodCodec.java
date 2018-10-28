// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.common;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * A {@link MethodCodec} using the Flutter standard binary encoding.
 *
 * <p>Compare with {@link StandardMethodCodec}, this class is more suitable for frequent passing on
 * large objects. This class avoid to use ByteArrayOutputStream which cause an extra replication of
 * byte array. Instead, it compute the estimated size first then allocate ByteBuffer directly.</p>
 *
 * <p>This codec is guaranteed to be compatible with the corresponding
 * <a href="https://docs.flutter.io/flutter/services/DirectMethodCodec-class.html">DirectMethodCodec</a>
 * on the Dart side. These parts of the Flutter SDK are evolved synchronously.</p>
 *
 * <p>Values supported as method arguments and result payloads are those supported by
 * {@link DirectMessageCodec}.</p>
 */
public final class DirectMethodCodec implements MethodCodec {
    public static final DirectMethodCodec INSTANCE = new DirectMethodCodec(DirectMessageCodec.INSTANCE);
    private final DirectMessageCodec messageCodec;

    /**
     * Creates a new method codec based on the specified message codec.
     */
    public DirectMethodCodec(DirectMessageCodec messageCodec) {
        this.messageCodec = messageCodec;
    }

    @Override
    public ByteBuffer encodeMethodCall(MethodCall methodCall) {
        int methodSize = messageCodec.computeValueSize(0, methodCall.method);
        int argsSize = messageCodec.computeValueSize(methodSize, methodCall.arguments);
        final ByteBuffer buffer = ByteBuffer.allocateDirect(methodSize + argsSize);
        messageCodec.writeValue(buffer, methodCall.method);
        messageCodec.writeValue(buffer, methodCall.arguments);
        return buffer;
    }

    @Override
    public MethodCall decodeMethodCall(ByteBuffer methodCall) {
        methodCall.order(ByteOrder.nativeOrder());
        final Object method = messageCodec.readValue(methodCall);
        final Object arguments = messageCodec.readValue(methodCall);
        if (method instanceof String && !methodCall.hasRemaining()) {
            return new MethodCall((String) method, arguments);
        }
        throw new IllegalArgumentException("Method call corrupted");
    }

    @Override
    public ByteBuffer encodeSuccessEnvelope(Object result) {
        final ByteBuffer buffer = ByteBuffer.allocateDirect(1 + messageCodec.computeValueSize(1, result));
        buffer.put((byte) 0);
        messageCodec.writeValue(buffer, result);
        return buffer;
    }

    @Override
    public ByteBuffer encodeErrorEnvelope(String errorCode, String errorMessage,
            Object errorDetails) {
        int errorCodeSize = messageCodec.computeValueSize(1, errorCode);
        int errorMessageSize = messageCodec.computeValueSize(1 + errorCodeSize, errorMessage);
        int errorDetailsSize = messageCodec.computeValueSize(1 + errorCodeSize + errorMessageSize, errorDetails);
        final ByteBuffer buffer = ByteBuffer.allocateDirect(1 + errorCodeSize + errorMessageSize + errorDetailsSize);
        buffer.put((byte) 1);
        messageCodec.writeValue(buffer, errorCode);
        messageCodec.writeValue(buffer, errorMessage);
        messageCodec.writeValue(buffer, errorDetails);
        return buffer;
    }

    @Override
    public Object decodeEnvelope(ByteBuffer envelope) {
        envelope.order(ByteOrder.nativeOrder());
        final byte flag = envelope.get();
        switch (flag) {
            case 0: {
                final Object result = messageCodec.readValue(envelope);
                if (!envelope.hasRemaining()) {
                    return result;
                }
            }
            // Falls through intentionally.
            case 1: {
                final Object code = messageCodec.readValue(envelope);
                final Object message = messageCodec.readValue(envelope);
                final Object details = messageCodec.readValue(envelope);
                if (code instanceof String
                        && (message == null || message instanceof String)
                        && !envelope.hasRemaining()) {
                    throw new FlutterException((String) code, (String) message, details);
                }
            }
        }
        throw new IllegalArgumentException("Envelope corrupted");
    }
}
