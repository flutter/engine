#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        0.0,
        // 1.0 / sqrt(1.0) = 1.0
        inversesqrt(1.0),
        0.0,
        // 1.0 / sqrt(4.0) + 0.5 = 1.0 / 2.0 + 0.5 = 1.0
        inversesqrt(4.0) + 0.5
    );
}
