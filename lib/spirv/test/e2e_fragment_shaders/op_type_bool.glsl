#version 310 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    bool f = false;
    bool t = true;
    float zero = float(f);
    float one = float(t);
    fragColor = vec4(zero, one, zero, one);
}
