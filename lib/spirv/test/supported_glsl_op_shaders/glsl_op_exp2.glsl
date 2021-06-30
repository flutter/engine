#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        0.0,
        // 2.0^0.0 = 1.0
        exp2(0.0),
        0.0,
        // 2.0^3.0 - 7.0 = 8.0 - 7.0 = 1.0
        exp2(3.0) - 7.0
    );
}
