#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    float zero = tan(0.0);
    float one = tan(0.785398163);
    fragColor = vec4(zero, one, zero, one);
}
