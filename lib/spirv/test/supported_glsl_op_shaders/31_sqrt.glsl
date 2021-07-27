#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        sqrt(a - 1.0),
        sqrt(a),
        sqrt(a * 9.0) - 3.0,
        1.0
    );
}

