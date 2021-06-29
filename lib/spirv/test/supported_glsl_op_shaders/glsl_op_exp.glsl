#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(0.0, exp(0.0), 0.0, exp(2.0) - 6.38905609893);
}
