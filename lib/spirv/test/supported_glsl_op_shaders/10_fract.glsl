#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        fract(a),
        // 0.25 + 0.75 = 1.0
        fract(a + 1.25) + fract(3.75),
        0.0,
        1.0
    );
}
