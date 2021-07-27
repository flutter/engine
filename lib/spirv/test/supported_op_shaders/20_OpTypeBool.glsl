#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    bool f = false;
    bool t = bool(a);
    float zero = float(f);
    float one = float(t);
    fragColor = vec4(zero, one, zero, one);
}

