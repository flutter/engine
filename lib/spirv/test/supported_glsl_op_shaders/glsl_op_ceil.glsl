#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        ceil(-0.25),
        ceil(0.25),
        ceil(-0.75),
        ceil(0.75)
    );
}
