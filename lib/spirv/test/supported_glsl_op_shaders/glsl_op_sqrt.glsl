#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        sqrt(0.0),
        sqrt(1.0),
        sqrt(9.0) - 3.0,
        1.0
    );
}
