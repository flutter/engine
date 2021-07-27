#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        // cos(0.0) = 1.0
        acos(1.0),
        // cos(1.0) = 0.54030230586
        acos(a * 0.54030230586),
        0.0,
        1.0
    );
}
