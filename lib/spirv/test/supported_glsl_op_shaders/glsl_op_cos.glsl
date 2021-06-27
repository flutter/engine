#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    float zero = cos(1.57079632679);
    float one = cos(0.0);
    fragColor = vec4(zero, one, zero, one);
}
