#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(trunc(0.15), trunc(1.89), trunc(0.77), trunc(1.05));
}
