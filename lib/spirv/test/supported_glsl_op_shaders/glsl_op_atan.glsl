#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        // tan(0.0) = 0.0
        atan(0.0),
        // tan(1.0) = 1.55740772465
        atan(1.55740772465),
        0.0,
        1.0
    );
}
