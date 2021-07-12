#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        min(0.0, 1.0),
        min(1.5, 1.5),
        0.0,
        1.0
    );
}
