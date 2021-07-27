#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        /* sin(0.0) = 0.0 */
        sin(0.0),
        // sin(1.57079632679) = sin(pi / 2.0) = 1.0
        sin(a * 1.57079632679),
        0.0,
        1.0
    );
}
