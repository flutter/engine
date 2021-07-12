#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        max(-1.0, 0.0),
        max(1.0, 0.5),
        0.0,
        1.0
    );
}
