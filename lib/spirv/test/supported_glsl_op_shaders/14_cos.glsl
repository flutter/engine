#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        /* cos(1.57079632679) = cos(pi / 2.0) = 0.0 */
        cos(a * 1.57079632679),
        // cos(0.0) = 0.0
        cos(0.0),
        0.0,
        1.0
    );
}
