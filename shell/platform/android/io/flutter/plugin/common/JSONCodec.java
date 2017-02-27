// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.common;

import java.nio.ByteBuffer;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

/**
 * A {@link MessageCodec} using UTF-8 encoded JSON messages, and supporting the same Java values
 * as {@link JSONObject#wrap(Object)}.
 */
public final class JSONCodec implements MethodCodec {
    // This codec must match the Dart codec of the same name in package flutter/services.
    public static final JSONCodec INSTANCE = new JSONCodec();

    private JSONCodec() {
    }

    @Override
    public ByteBuffer encodeMessage(Object message) {
        if (message == null) {
            return null;
        }
        return StringCodec.INSTANCE.encodeMessage(JSONObject.wrap(message).toString());
    }

    @Override
    public Object decodeMessage(ByteBuffer message) {
        if (message == null) {
            return null;
        }
        try {
            final String json = StringCodec.INSTANCE.decodeMessage(message);
            final JSONTokener tokener = new JSONTokener(json);
            final Object value = tokener.nextValue();
            if (tokener.more()) {
                throw new IllegalArgumentException("Invalid JSON");
            }
            return value;
        } catch (JSONException e) {
            throw new IllegalArgumentException("Invalid JSON", e);
        }
    }

    @Override
    public MethodCall decodeMethodCall(ByteBuffer message) {
        try {
            final Object json = decodeMessage(message);
            if (json instanceof JSONArray) {
                final JSONArray pair = (JSONArray) json;
                if (pair.length() == 2 && pair.get(0) instanceof String) {
                    return new MethodCall(pair.getString(0), pair.get(1));
                }
            }
            throw new IllegalArgumentException("Invalid method call: " + json);
        } catch (JSONException e) {
            throw new IllegalArgumentException("Invalid JSON", e);
        }
    }

    @Override
    public ByteBuffer encodeSuccessEnvelope(Object result) {
        return encodeMessage(new JSONArray().put(JSONObject.wrap(result)));
    }

    @Override
    public ByteBuffer encodeErrorEnvelope(String errorCode, String errorMessage, Object errorDetails) {
        return encodeMessage(new JSONArray()
            .put(errorCode)
            .put(errorMessage)
            .put(JSONObject.wrap(errorDetails)));
    }
}
