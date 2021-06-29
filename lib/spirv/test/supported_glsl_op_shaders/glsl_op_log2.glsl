#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(log2(1.0), log2(2.0), 0.0, 1.0);
}
