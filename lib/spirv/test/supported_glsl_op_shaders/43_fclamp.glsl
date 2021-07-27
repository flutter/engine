#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

// clamp(x, a, b) is equivalent to min(max(x, a), b)
void main() {
    fragColor = vec4(
        0.0,
        clamp(10.0, 0.0, a),
        0.0,
        clamp(-1.0, a, 10.0)
    );
}

