#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        0.0,
        // 0.01745329251 * 180.0 / pi = 1.0
        degrees(a * 0.01745329251),
        0.0,
        1.0
    );
}
