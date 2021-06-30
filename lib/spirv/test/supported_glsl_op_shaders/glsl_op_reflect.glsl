#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

// TODO
// I - 2.0 * dot(N, I) * N.
void main() {
    fragColor = vec4(
        0.0,
        1.0,
        0.0,
        1.0
    );
}
