#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        0.0,
        // 57.2957795131 * pi / 180.0 = 1.0
        float(radians(57.2957795131)),
        0.0,
        1.0
    );
}
