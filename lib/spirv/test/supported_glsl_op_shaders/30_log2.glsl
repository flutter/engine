#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        // 2.0^0.0 = 1.0
        log2(a),
        // 2.0^1.0 = 2.0
        log2(a * 2.0),
        0.0,
        1.0
    );
}
