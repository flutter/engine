#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        ceil(a * -0.25),
        ceil(a * 0.25),
        ceil(a * -0.75),
        ceil(a * 0.75)
    );
}

