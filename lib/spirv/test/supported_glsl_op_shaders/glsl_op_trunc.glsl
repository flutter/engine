#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        trunc(0.15),
        trunc(1.01),
        trunc(-0.15),
        trunc(1.99)
    );
}
