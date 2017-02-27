package io.flutter.plugin.common;

import java.util.Objects;

/**
 * Command object representing a method call on a {@link FlutterMessageChannel}.
 */
public final class MethodCall {
    /**
     * The name of the called method.
     */
    public final String method;

    /**
     * Arguments for the call.
     */
    public final Object arguments;

    /**
     * Creates a {@link MethodCall} with the specified method name and arguments.
     *
     * @param method the method name String, not null.
     * @param arguments the arguments, a value supported by the channel's message codec.
     */
    public MethodCall(String method, Object arguments) {
        this.method = Objects.requireNonNull(method);
        this.arguments = arguments;
    }
}
