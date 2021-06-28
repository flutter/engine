#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(fract(1.0), fract(2.25) + fract(3.75), 0.0, 1.0);
}
