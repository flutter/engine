package io.flutter.plugin.common;

import java.nio.ByteBuffer;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

/**
 * A {@link MessageCodec} using UTF-8 encoded JSON messages, and supporting the same Java values
 * as {@link JSONObject#wrap(Object)}.
 */
final class JSONCodec implements MessageCodec {
    @Override
    public ByteBuffer encodeMessage(Object message) {
        if (message == null) {
            return null;
        }
        return ByteBufferTools.extractBytes(JSONObject.wrap(message).toString());
    }

    @Override
    public Object decodeMessage(ByteBuffer message) {
        if (message == null) {
            return null;
        }
        try {
            final String json = ByteBufferTools.extractString(message);
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
            if (json instanceof JSONObject) {
                final JSONObject map = (JSONObject) json;
                if (map.has("method") && map.has("args")) {
                    return new MethodCall(map.getString("method"), map.get("args"));
                }
            }
            throw new IllegalArgumentException("Invalid method call: " + json);
        } catch (JSONException e) {
            throw new IllegalArgumentException("Invalid JSON", e);
        }
    }

    @Override
    public ByteBuffer encodeSuccessEnvelope(Object result) {
        try {
            return encodeMessage(new JSONObject()
                .put("status", "ok")
                .put("data", JSONObject.wrap(result))
                .toString());
        } catch (JSONException e) {
            throw new IllegalArgumentException("Invalid JSON result: " + result);
        }
    }

    @Override
    public ByteBuffer encodeErrorEnvelope(String errorCode, String errorMessage, Object errorDetails) {
        try {
            return encodeMessage(new JSONObject()
                .put("status", errorCode)
                .put("message", errorMessage)
                .put("data", JSONObject.wrap(errorDetails))
                .toString());
        } catch (JSONException e) {
            throw new IllegalArgumentException("Invalid JSON error details: " + errorDetails);
        }
    }
}
