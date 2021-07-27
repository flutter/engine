#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        floor(a * 0.25),
        floor(a * 1.25),
        floor(a * 0.75),
        floor(a * 1.75)
    );
}
