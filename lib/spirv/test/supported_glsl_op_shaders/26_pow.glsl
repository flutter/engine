#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        pow(2.0, a-1.0) - 1.0,
        pow(a * 3.14, 0.0),
        0.0,
        1.0
    );
}

