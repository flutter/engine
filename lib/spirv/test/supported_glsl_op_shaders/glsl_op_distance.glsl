#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        // distance between same value is 0.0
        distance(7.0, 7.0),
        // 7.0 - 6.0 = 1.0
        distance(7.0, 6.0),
        0.0,
        // sqrt(7.0 * 7.0 - 6.0 * 8.0) = sqrt(49.0 - 48.0) = sqrt(1.0) = 1.0
        distance(vec2(7.0, 6.0), vec2(7.0, 8.0))
    );
}
