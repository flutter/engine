#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

// 0.0 is returned if x (second param) < edge (first param), and 1.0 is returned otherwise.
void main() {
    fragColor = vec4(
        0.0,
        step(0.5, a),
        0.0,
        1.0
    );
}
