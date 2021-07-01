#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    float zero = 0.0;
    float one = 1.0;
    fragColor = vec4(zero, one, zero, one);
}
