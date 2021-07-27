#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        0.0,
        // 1.0 / sqrt(1.0) = 1.0
        inversesqrt(a),
        0.0,
        // 1.0 / sqrt(4.0) + 0.5 = 1.0 / 2.0 + 0.5 = 1.0
        inversesqrt(a * 4.0) + 0.5
    );
}
