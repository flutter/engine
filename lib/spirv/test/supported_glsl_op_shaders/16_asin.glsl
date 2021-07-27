#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        /* sin(0.0) = 0.0 */
        asin(0.0),
        // sin(1.0) = 0.8414709848
        asin(a * 0.8414709848),
        0.0,
        1.0
    );
}
