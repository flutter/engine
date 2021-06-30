#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        floor(0.25),
        floor(1.25),
        floor(0.75),
        floor(1.75)
    );
}
