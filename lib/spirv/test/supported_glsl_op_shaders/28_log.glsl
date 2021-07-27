#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        // e^0.0 = 1.0
        log(a),
        // e^1.0 = e
        log(a * 2.718281828459),
        0.0,
        1.0
    );
}
