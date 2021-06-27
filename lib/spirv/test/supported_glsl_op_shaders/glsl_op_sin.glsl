#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    float zero = sin(0.0);
    float one = sin(1.57079632679);
    fragColor = vec4(zero, one, zero, one);
}
