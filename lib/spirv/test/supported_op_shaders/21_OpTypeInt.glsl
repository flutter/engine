#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    int zeroInt = 0;
    int oneInt = int(a);
    float zero = float(zeroInt);
    float one = float(oneInt);
    fragColor = vec4(zero, one, zero, one);
}

