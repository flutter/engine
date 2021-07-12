#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    int zeroInt = 0;
    int oneInt = 1;
    float zero = float(zeroInt);
    float one = float(oneInt);
    fragColor = vec4(zero, one, zero, one);
}
