#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    float zero = 0.0;
    fragColor = vec4(zero, a, zero, a);
}

